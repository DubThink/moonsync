#ifndef _DEBUGCAMERA_H_
#define _DEBUGCAMERA_H_
#ifdef DEBUG
#include "vec3.h"
class DebugCamera {
private:
	float xRot = 0.0;
	float yRot = 0.0;
	Vec3 pos{};
public:
	float speed = 0.1;
	Vec3 getPosition();
	Vec3 getLookDirection();
	Vec3 getLookAt();
	void moveForward(float amt);
	void moveRight(float amt);
	void moveUp(float amt);
	void lookRight(float amt);
	void lookUp(float amt);
};
#endif // DEBUG
#endif // header guard