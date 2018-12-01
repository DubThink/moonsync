#include <math.h>
#include "debug_camera.h"

Vec3 DebugCamera::getPosition()
{
	return pos;
}

Vec3 DebugCamera::getLookDirection()
{
	return Vec3{ cos(yRot)*cos(xRot),sin(xRot),sin(yRot)*cos(xRot) };
}

Vec3 DebugCamera::getLookAt()
{
	return getPosition() + getLookDirection();
}



void DebugCamera::moveForward(float amt)
{
	pos += getLookDirection()*speed*amt;
}

void DebugCamera::moveRight(float amt)
{
	pos += cross(getLookDirection(), Vec3{ 0,1 })*speed*amt;
}

void DebugCamera::moveUp(float amt)
{
	pos.y += speed * amt;
}

void DebugCamera::lookRight(float amt)
{
	yRot += 0.01*amt;
}

void DebugCamera::lookUp(float amt)
{
	xRot += 0.01*amt;
}