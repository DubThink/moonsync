#include <math.h>
#include "debug_camera.h"

constexpr float HALF_PI = 1.570795f;
Vec3 DebugCamera::getPosition()
{
	return pos;
}

Vec3 DebugCamera::getLookDirection()
{
	xRot = (xRot>HALF_PI) ? HALF_PI : (xRot < -HALF_PI ? -HALF_PI : xRot);
	return Vec3{ cos(yRot)*cos(xRot),sin(xRot),sin(yRot)*cos(xRot) };
}

Vec3 DebugCamera::getLookAt()
{
	return getPosition() + getLookDirection();
}



void DebugCamera::moveForward(float amt)
{
	pos += getLookDirection()*speed*amt*frameTime;
}

void DebugCamera::moveRight(float amt)
{
	pos += cross(getLookDirection(), Vec3{ 0,1 })*speed*amt*frameTime;
}

void DebugCamera::moveUp(float amt)
{
	pos.y += speed * amt*frameTime;
}

void DebugCamera::lookRight(float amt)
{
	yRot += 0.01*amt*frameTime;
}

void DebugCamera::lookUp(float amt)
{
	xRot += 0.01*amt*frameTime;
}