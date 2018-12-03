#ifndef SDF_H
#define SDF_H

#include "vec3.h"

inline float _min(float a, float b) {
	return a < b ? a : b;
}

inline float _max(float v, float f) {
	return v > f ? v : f;
}

inline vec3 _max(vec3 v, float f) {
	return vec3{
		v.x > f ? v.x : f,
		v.y > f ? v.y : f,
		v.z > f ? v.z : f
	};
}

inline vec2 _max(vec2 v, float f) {
	return vec2{
		v.x > f ? v.x : f,
		v.y > f ? v.y : f
	};
}

inline vec3 _min(vec3 v, float f) {
	return vec3{
		v.x < f ? v.x : f,
		v.y < f ? v.y : f,
		v.z < f ? v.z : f
	};
}

inline vec3 _max(vec3 v, vec3 f) {
	return vec3{
		v.x > f.x ? v.x : f.x,
		v.y > f.y ? v.y : f.y,
		v.z > f.z ? v.z : f.z
	};
}

inline vec3 clamp(vec3 v, float _minv, float _maxv) {
	return _max(_minv, _min(v, _maxv));
}

inline vec3 mix(vec3 x, vec3 y, float a) {
	return vec3{
		(1 - a)*x.x + y.x*a,
		(1 - a)*x.y + y.y*a,
		(1 - a)*x.z + y.z*a
	};
}

inline float clamp(float v, float _minv, float _maxv) {
	return _max(_minv, _min(v, _maxv));
}

inline float mix(float x, float y, float a) {
	return (1 - a)*x + y * a;
}

inline float mod(float a, float b) {
	return a - b * floor(a / b);
}

inline vec3 mod(vec3 v, float f) {
	return vec3{
		(float)mod(v.x,f),
		(float)mod(v.y,f),
		(float)mod(v.z,f),
	};
}


float sdPlane(vec3 p, vec4 n) {
	// n must be normalized
	return dot(p, n.xyz()) + n.w;// + sin(p.x/10.0)*3.0 + sin(p.z/10.0)*3.0;
}

float sphereSDF( vec3 v, vec3 center, float radius)
{
	return length(v - center) - radius;
}

float sdBox(vec3 p, vec3 b)
{
	vec3 d = abs(p) - b;
	return length(_max(d, 0.0))
		+ _min(_max(d.x, _max(d.y, d.z)), 0.0); // remove this line for an only partially signed sdf
}

float sdEnemy(vec3 v, vec3 p) {
	//float playerModel = //sdVerticalCapsule(v,2,1);
	return  sphereSDF(v, vec3(p), 1);
}
float sdCappedCylinder(vec3 p, vec2 h)
{
	vec2 d = abs(vec2(length(p.xz()), p.y)) - h;
	return _min(_max(d.x, d.y), 0.0) + length(_max(d, 0.0));
}

float opSmoothUnion(float d1, float d2, float k)
{
	float h = clamp(0.5 + 0.5*(d2 - d1) / k, 0.0, 1.0);
	return mix(d2, d1, h) - k * h*(1.0 - h);
}

float opSubtraction(float d1, float d2) { return _max(-d1, d2); }
#endif // !SDF_H
