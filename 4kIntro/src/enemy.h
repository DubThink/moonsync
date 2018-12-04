#ifndef ENEMY_H
#define ENEMY_H
#include "ballPhysics.h"
#include "worldsdf.h"
#define NR_ENEMIES 8
#define PROJECTILE_OFFSET 4
struct Enemy {
	vec3 position;
	vec3 target;
	float fire_cooldown=40.f;
	float lifestate;
	float health = 1;
};

struct rRS {
	int hitCount=0;
	int killCount=0;
};
extern Enemy enemies[NR_ENEMIES];

void updateEnemies(float dt, vec3 playerPos, PhysBall* balls);

rRS updateEnemyProjectiles(float dt, vec3 playerPos, PhysBall* balls);
rRS laserUpdate(float dt, vec3 dir, vec3 playerPos);
void copyEnemyBalls(Ball* out);
void respawn(float chance);
void killall(float chance);
#endif // !ENEMY_H
