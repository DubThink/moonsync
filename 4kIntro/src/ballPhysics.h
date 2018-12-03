#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H
#include "vec3.h"

#define NR_BALLS 16
#define PHYS_MAX_ITER 16



// mirrors data in shader
struct Ball {
	Vec4 pos;
	Vec4 vel;
};

struct PhysBall {
	vec3 position;
	vec3 velocity;
	float radius=0;
	bool onground;
	float restitution = 0.5;
	float friction = 0.02;
	bool playerPhysics = false;
	bool gravity = true;
	float lifetime = 0;
};



extern PhysBall allMyBalls[NR_BALLS];

void updatePhysics(float dt);

void copyBalls(Ball* out);

#endif // !BALL_PHYSICS_H
