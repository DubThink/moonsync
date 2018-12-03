#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H


#define NR_BALLS 8
#define PHYS_MAX_ITER 16



// mirrors data in shader
struct Ball {
	Vec4 pos;
	Vec4 vel;
};

struct PhysBall {
	vec3 position;
	vec3 velocity;
	float radius;
};



extern PhysBall allMyBalls[NR_BALLS];

void updatePhysics(float dt);

void copyBalls(Ball* out);

#endif // !BALL_PHYSICS_H
