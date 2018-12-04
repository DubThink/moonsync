#include <cstdlib>>
#include "enemy.h"

Enemy enemies[NR_ENEMIES];

void updateEnemies(float dt,vec3 playerPos, PhysBall* balls)
{
	Enemy* enemy;
	RaymarchResult result;
	for (int i = 0; i < NR_ENEMIES; i++) {
		enemy = enemies + i;
		enemies->fire_cooldown = _min(enemies->fire_cooldown - dt, 0);

		if (enemy->lifestate > 0 && enemy->lifestate < 1) {
			//spawning
			enemy->lifestate = _min(enemy->lifestate + 0.1, 1);
			if (enemy->lifestate == 1)
				enemy->health = 1;
			continue;
		}
		if (enemy->lifestate > 1 && enemy->lifestate < 2) {
			//dying
			enemy->lifestate = _min(enemy->lifestate + 0.1, 2);
			continue;
		}
		if (enemy->lifestate == 2) {
			//ded
			enemy->lifestate = 0;
		}
		if(enemy->lifestate == 0) {
			//not born
			continue;
		}
		
		//result = worldMarch(enemy->position+ normalize(enemy->position - playerPos)*2,normalize(enemy->position-playerPos)*-1, 800, 0.01, length(enemy->position - playerPos)+1);
		if (true||length(result.position - playerPos) < 1) {
			// fire

			// get unused projectile
			PhysBall* proj = nullptr;
			for (int j = PROJECTILE_OFFSET; j < NR_BALLS; j++) {
				if (balls[j].lifetime == 0) {
					proj = balls+j;
					break;
				}
			}
			if (proj == nullptr)
				continue; // can't shoot cause no projectile
			proj->friction = 0;
			proj->restitution = 1;
			proj->gravity = false;
			proj->velocity = normalize(enemy->position - playerPos) * -30;
			proj->position = enemy->position +normalize(enemy->position - playerPos) * -2;
			proj->radius = .3;
			proj->lifetime = 1;
			enemy->fire_cooldown = 3.f;
		}
	}
}

rRS updateEnemyProjectiles(float dt, vec3 playerPos, PhysBall* balls)
{
	rRS ret;
	PhysBall* ball;
	for (int i = 1; i < 4; i++) {
		ball = balls + i;
		if (ball->lifetime == 0)
			continue;
		/*ball->lifetime = _max(0, ball->lifetime - 0.01);*/

		// test if hit enemy
		for (int j = 0; j < NR_ENEMIES; j++) {
			if (length(ball->position - enemies[j].position) < 2.5) {
				ball->lifetime = 0;
				ball->radius = 0;
				if(enemies[j].lifestate==1)
					enemies[j].health -= .8;
				if (enemies[j].health < 0) {
					enemies[j].health = 0;
					enemies[j].lifestate = 1.01;
					ret.killCount++;
				}
			}
		}
	}
	for (int i = 4; i < NR_BALLS; i++) {
		ball = balls + i;
		if (ball->lifetime == 0)continue;
		ball->lifetime = _max(0, ball->lifetime - 0.02);
		if (length(ball->position - playerPos) < 1) {
			ball->radius = 0;
			ball->lifetime = 0;
			ret.hitCount++;
		}
	}
	return ret;
}

rRS laserUpdate(float dt, vec3 dir, vec3 playerPos)
{
	rRS ret;
	for (int j = 0; j < NR_ENEMIES; j++) {
		if (length(playerPos - enemies[j].position) < 20&&abs(dot(dir, normalize(playerPos - enemies[j].position)))>.99) {
			if (enemies[j].lifestate == 1)
				enemies[j].health -= .05;
			if (enemies[j].health < 0) {
				enemies[j].health = 0;
				enemies[j].lifestate = 1.01;
				ret.killCount++;
			}
		}
	}
	return ret;
}

void copyEnemyBalls(Ball * out)
{
	for (int i = 0; i < NR_ENEMIES; i++) {
		out->pos = vec4(enemies[i].position, enemies[i].lifestate).dataOnly();
		out->vel = vec4(enemies[i].target, enemies[i].health).dataOnly();
		out++;
	}
}

void respawn(float chance)
{
	for (int i = 0; i < NR_ENEMIES; i++) {
		if (enemies[i].lifestate == 0) {
			if (rand() / (float)RAND_MAX < chance) {
				enemies[i].position = vec3(
					(rand() % 5)*10.f - 2,
					12 * (rand() % 2) + 5,
					(rand() % 5)*10.f - 2);
				enemies[i].lifestate = 0.01;
			}
		}
	}
}
void killall(float chance)
{
	for (int i = 0; i < NR_ENEMIES; i++) {
		enemies[i].lifestate = 2;
	}
}