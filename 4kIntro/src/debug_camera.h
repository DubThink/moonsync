#ifndef _DEBUGCAMERA_H_
#define _DEBUGCAMERA_H_
#include "vec3.h"
class DebugCamera {
public:
	float xRot = 0.0;
	float yRot = 0.0;
	vec3 pos{-5.f,7.f,0.f};
	float frameTime = 1 / 60.f;
	float speed = 0.1;
	vec3 getPosition();
	vec3 getLookDirection();
	vec3 getLookAt();
	void moveForward(float amt);
	void moveRight(float amt);
	void moveUp(float amt);
	void lookRight(float amt);
	void lookUp(float amt);
};
#endif // header guard