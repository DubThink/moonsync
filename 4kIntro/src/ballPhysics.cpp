#include "worldsdf.h"
#include "ballPhysics.h"
#include <windows.h>

#define EPSILON 0.001
PhysBall allMyBalls[NR_BALLS];

void updatePhysics(float dt){
	dt = _min(dt, 0.03);
	RaymarchResult result;
	PhysBall* ball;
	float distanceToTravel;
	vec3 vnorm;
	for (int i = 3; i < 4; i++) {
		ball = allMyBalls+i;
		result.hit = true;
		ball->velocity += vec3(0.f, -0.5f, 0.f);
		float distanceToTravel = dt*length(ball->velocity);
		//if (distanceToTravel < EPSILON)continue;
		// run for PHYS_MAX_ITER while we still need to travel
		for (int j = 0; j < PHYS_MAX_ITER&&distanceToTravel>0; j++) {
			vnorm = normalize(ball->velocity);
			result = worldMarch(ball->position+vnorm*EPSILON, vnorm, 800, ball->radius, distanceToTravel);
			if (!result.hit|| result.dist>distanceToTravel) {
				ball->position += ball->velocity*dt;
				break;
			}
			if (GetAsyncKeyState(VK_RBUTTON))
				int a = 0;
			ball->position += normalize(ball->velocity)*result.dist;
			distanceToTravel -= result.dist;
			distanceToTravel *= 0.5; // restitution
			ball->velocity = reflect(ball->velocity, result.normal);
			ball->velocity = ball->velocity*0.5;// += vnorm * dot(result.normal, ball->velocity)*0.3;
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