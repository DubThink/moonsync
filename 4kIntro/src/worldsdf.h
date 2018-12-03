#ifndef WORLDSDF_H
#define WORLDSDF_H

#include "vec3.h"
#include "sdf.h"
inline float abs(float f) {
	return f > 0 ? f : -f;
}
float worldSDF1(vec3 v);

// Tetrahedron technivue from http://ivuilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcWorldNormal(vec3 p);

const float _min_HIT_DIST = 0.0005;
const float _max_DIST = 100.0;
const float SPEED_MULTIPLIER = 1.0;

struct Material {
	vec3 diffuseColor;
	float specular;
	float shininess;
};

struct RaymarchResult
{
	vec3 position;
	vec3 normal;
	bool hit;
	float dist;
};

// @param ray origin and ray direction
RaymarchResult worldMarch(vec3 ro, vec3 rd, int _max_STEPS, float radius, float _maxDist);

#endif // !WORLDSDF_H
