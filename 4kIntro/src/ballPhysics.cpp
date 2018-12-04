#include "ballPhysics.h"
#include "worldsdf.h"

#include <windows.h>

#define EPSILON 0.01
PhysBall allMyBalls[NR_BALLS];

void updatePhysics(float dt){
	dt = _min(dt, 0.03);
	RaymarchResult result;
	PhysBall* ball;
	vec3 vnorm;
	bool hitground;
	for (int i = 0; i < NR_BALLS; i++) {
		ball = allMyBalls+i;
		result.hit = true;
		hitground = false;
		if(ball->gravity)ball->velocity += vec3(0.f, -60.f, 0.f)*dt;
		//if(length(ball->velocity)>20)
		//	ball
		float timeLeft = dt;
		//if (distanceToTravel < EPSILON)continue;
		// run for PHYS_MAX_ITER while we still need to travel
		for (int j = 0; j < PHYS_MAX_ITER&&timeLeft>0; j++) {
			vnorm = normalize(ball->velocity);
			result = worldMarch(ball->position+vnorm*EPSILON+calcWorldNormal(ball->position)*EPSILON, vnorm, 400, ball->radius, timeLeft*length(ball->velocity));
			if (!result.hit|| result.dist> timeLeft*length(ball->velocity)) {
				ball->position += ball->velocity*dt;
				break;
			}
			if (GetAsyncKeyState(VK_MBUTTON))
				int a = 0;
			if (worldSDF1(ball->position) < 0)
				ball->position += result.normal*-worldSDF1(ball->position);
			if (dot(result.normal, vec3{ 0,1,0 }) > 0.2)hitground = true;
			ball->position += normalize(ball->velocity)*result.dist;
			timeLeft -= result.dist/ length(ball->velocity);
			//distanceToTravel *= ball->restitution; // restitution
			ball->velocity = reflect(ball->velocity, result.normal);
			//ball->velocity = ball->velocity*ball->restitution;// += vnorm * dot(result.normal, ball->velocity)*0.3;
			ball->velocity += result.normal*-dot(result.normal,ball->velocity)*(1-ball->restitution);
		}
		if (ball->playerPhysics) {
			ball->onground = worldSDF1(ball->position+vec3(0,-ball->radius/2,0)) < ball->radius/2 + 0.05;
			if (ball->onground) ball->velocity = ball->velocity*0.5;
			// jump pad
			if (length(vec2(abs(ball->position.x) - 19, abs(ball->position.z) - 10)) < 3 && ball->position.y < 2) {
				ball->velocity.y = 40;
				ball->position.y +=1;

				ball->onground = false;
			}
			// speed limit
			if (length(ball->velocity.xz()) > 7) {
				ball->velocity.x = ball->velocity.x*(1.f / length(ball->velocity.xz()));
				ball->velocity.z = ball->velocity.z*(1.f / length(ball->velocity.xz()));
			}
		}
		else {
			ball->onground = hitground || worldSDF1(ball->position) < ball->radius + 0.05;
			if (ball->onground) ball->velocity = ball->velocity*0.5;
		}
	}
}

void copyBalls(Ball * out){
	for (int i = 0; i < NR_BALLS; i++) {
		out->pos = vec4(allMyBalls[i].position, allMyBalls[i].radius).dataOnly();
		out->vel = vec4(allMyBalls[i].velocity, allMyBalls[i].radius).dataOnly();
		out++;
	}
}