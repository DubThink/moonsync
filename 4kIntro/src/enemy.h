#ifndef ENEMY_H
#define ENEMY_H
#include "ballPhysics.h"
#include "worldsdf.h"
#define NR_ENEMIES 8
#define PROJECTILE_OFFSET 4
struct Enemy {
	vec3 position;
	vec3 target;
	float fire_cooldown;
	float lifestate;
	float health;
};

extern Enemy enemies[NR_ENEMIES];

void updateEnemies(float dt, vec3 playerPos);

void updateEnemyProjectiles(float dt, vec3 playerPos);
void copyEnemyBalls(Ball* out);
#endif // !ENEMY_H
