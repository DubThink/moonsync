#include "enemy.h"

Enemy enemies[NR_ENEMIES];

void updateEnemies(float dt,vec3 playerPos)
{
	Enemy* enemy;
	RaymarchResult result;
	for (int i = 0; i < NR_ENEMIES; i++) {
		enemy = enemies + i;
		enemies->fire_cooldown = _max(enemies->fire_cooldown - dt, 0);
		// trace to player
		result = worldMarch(enemy->position, normalize(enemy->position-playerPos), 800, 0.01, length(enemy->position - playerPos)+1);
		if (length(result.position - playerPos) < 1) {
			// fire

			// get unused projectile
			PhysBall* proj = nullptr;
			for (int j = PROJECTILE_OFFSET; j < NR_BALLS; i++) {
				if (allMyBalls[i].lifetime == 0) {
					proj = allMyBalls+i;
				}
			}
			if (proj == nullptr)
				continue; // can't shoot cause no projectile
			proj->friction = 0;
			proj->restitution = 1;
			proj->gravity = false;
			proj->velocity = normalize(enemy->position - playerPos) * 10;
			proj->position = enemy->position + normalize(enemy->position - playerPos) * 2;
			proj->radius = .3;
		}
	}
}

void updateEnemyProjectiles(float dt, vec3 playerPos)
{
}

void copyEnemyBalls(Ball * out)
{
	for (int i = 0; i < NR_ENEMIES; i++) {
		out->pos = vec4(enemies[i].position, enemies[i].lifestate).dataOnly();
		out->vel = vec4(enemies[i].target, enemies[i].health).dataOnly();
		out++;
	}
}
