#ifndef WORLDSDF_H
#define WORLDSDF_H

#include "vec3.h"
#include "sdf.h"
inline float abs(float f) {
	return f > 0 ? f : -f;
}
float worldSDF1(vec3 v)
{
	float sdf_agg = 0.0;

	float plane = sdPlane(v, vec4(0.0, 1.0, 0.0, 2.0));
	vec3 q, r;

	// enemy
	float playerModel = sdEnemy(v, vec3(0, 20, 0));

	float box1 = sdBox(v, vec3(36.0, 22.0, 16.0));
	float box2 = sdBox(v, vec3(34.5, 35.0, 14.5));
	// -- Arches - lower floor
	float archesLowerFloor = _min(sdCappedCylinder(vec3(v.y - 6, v.x, mod(v.z + 5, 10) - 5), vec2(4.5, 36)), sdCappedCylinder(vec3(mod(v.x + 5, 10) - 5, v.z, v.y - 6), vec2(4.5, 15)));
	archesLowerFloor = opSubtraction(archesLowerFloor, sdBox(v - vec3(0, 8.5, 0), vec3(36.0, 2.5, 15.0)));
	float mainPillars = sdCappedCylinder(vec3(mod(v.x, 10) - 5, v.y, mod(v.z, 10) - 5), vec2(.4, 22));
	archesLowerFloor = opSmoothUnion(archesLowerFloor, mainPillars, .5);

	q = v - vec3(0, 11, 0);
	float archesUpperFloor = _min(sdCappedCylinder(vec3(q.y - 6, q.x, mod(q.z + 5, 10) - 5), vec2(4.5, 36)), sdCappedCylinder(vec3(mod(q.x + 5, 10) - 5, q.z, q.y - 6), vec2(4.5, 15)));
	archesUpperFloor = opSubtraction(archesUpperFloor, sdBox(q - vec3(0, 10.5, 0), vec3(36.0, 3.5, 15.0)));

	float arches = opSmoothUnion(archesUpperFloor, archesLowerFloor, .5);
	arches = _min(arches, sdBox(v + vec3(0, 0.75, 0), vec3(36.0, .5, 15.0)));
	arches = _min(arches, _min(sdBox(vec3(v.x - 0, v.y - 12, abs(v.z) - 5), vec3(25.0, 1., .25)), sdBox(vec3(abs(v.x) - 25, v.y - 12, v.z), vec3(.25, 1., 5))));
	sdf_agg = _min(opSubtraction(sdBox(v, vec3(24, 35, 4.5)), _min(arches, opSubtraction(box2, box1))), _min(plane,playerModel));
	// float blob = sin(4.0 * v.x) * sin(4.0 * v.y) * sin(4.0 * v.z) * 0.25 + sin(v.x) * sin(v.y) * sin(v.z)*0.1; // blobs on this sphere

	// vec3 v = v.xyz;

	// v.xz = mod(v.xz, vec2(20.0, 20.0)) - vec2(10.0, 10.0);

	// float sphere1 = sphereSDF(v, vec3(-0.0,10 - sin(v.x/10.0)*3.0 - sin(v.z/10.0),0), 4.0);
	//
	// float sphere2 = sphereSDF(v, vec3(-5.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 0.0), 2.0);
	// float sphere3 = sphereSDF(v, vec3(0.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 5.0), 2.0);
	// float sphere4 = sphereSDF(v, vec3(0.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, -5.0), 2.0);
	// float sphere5 = sphereSDF(v, vec3(5.0, 10.0 - sin(v.x/10.0)*3.0 - sin(v.z/10.0)*3.0, 0.0), 2.0);
	//
	// float c1 = opSmoothUnion(sphere1, sphere2, sin(TIME)*2.0+2.0); // unionize the spheres to make a... pinecone
	// float c2 = opSmoothUnion(sphere3, sphere4, sin(TIME)*2.0+2.0);
	// sdf_agg = opSmoothUnion(c1, c2, sin(TIME)*2.0+2.0);
	// sdf_agg = opSmoothUnion(sphere5, sdf_agg, sin(TIME)*2.0+2.0);

	// sdf_agg = opSmoothUnion(sdf_agg, plane, 3.0);

	//sdf_agg = _max(sdf_agg, -sdBox(v, vec3(4.0, 10.0, 4.0)));
	// sdf_agg = opSmoothUnion(sdf_agg, sdBox(v, vec3(1.0, 10.0, 1.0)), 3.5);
	// sdf_agg=_min(sdf_agg,worldSDF2(v));
	return sdf_agg;
}

// Tetrahedron technivue from http://ivuilezles.org/www/articles/normalsSDF/normalsSDF.htm
vec3 calcWorldNormal( vec3 p)
{
	const float h = 0.0001;
	const vec2 k = vec2(1.0, -1.0);
	return normalize(xyy(k)*worldSDF1(p + xyy(k)*h) +
		yyx(k)*worldSDF1(p + yyx(k)*h) +
		yxy(k)*worldSDF1(p + yxy(k)*h) +
		xxx(k)*worldSDF1(p + xxx(k)*h));
}

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
RaymarchResult worldMarch(vec3 ro, vec3 rd, int _max_STEPS, float radius, float _maxDist) {

	RaymarchResult marched;
	// default material
	marched.hit = true;
	float dist = 0;
	// vec3 dist=v;
	for (int i = 0; i < _max_STEPS; i++) {

		float samp = worldSDF1(ro)-radius; // find SDF at current march )
		// dist+=samp;
		// If SDF is low enough, handle the collision.
		if (abs(samp) < _min_HIT_DIST) {
			break;
		}
		if (length(ro) > _max_DIST) {
			marched.hit = false;
			break;
		}
		// Step ray forwards by SDF
		dist += samp;
		if (dist > _maxDist) {
			marched.hit = false;
			marched.position = ro;
			break;
		}
		ro += rd * samp*SPEED_MULTIPLIER;
	}
	marched.position = ro;
	marched.normal = calcWorldNormal(ro);
	marched.dist = dist;
	return marched;
}

#endif // !WORLDSDF_H
