#include "worldsdf.h"
#include "enemy.h"

float worldSDF1( vec3 v)
{
	float sdf_agg = 0.0;

	float plane = sdPlane(v, vec4(0, 1.0, 0.0, 2.0));
	vec3 q, r;

	// enemy

	float box1 = sdBox(v, vec3(40.f, 18.0, 40.f));
	float box2 = sdBox(v, vec3(34.5, 40.0, 34.5));
	// -- Arches - lower floor
	float archesLowerFloor = _min(sdCappedCylinder(vec3(v.y - 6, v.x, mod(v.z + 5, 10) - 5), vec2(4.5, 36)), sdCappedCylinder(vec3(mod(v.x + 5, 10) - 5, v.z, v.y - 6), vec2(4.5, 36)));
	archesLowerFloor = opSubtraction(archesLowerFloor, sdBox(v - vec3(0, 8.5, 0), vec3(36.0, 2.5, 36.0)));
	float mainPillars = sdCappedCylinder(vec3(mod(v.x, 10) - 5, v.y, mod(v.z, 10) - 5), vec2(.4, 22));
	archesLowerFloor = opSmoothUnion(archesLowerFloor, mainPillars, .5);
	//q = v;
	//q.x = abs(q.x);
	//q = q + vec3(-19, abs(.45f), 1.8);
	//q=rotateX(q,.6);
	//float ramp = sdBox(q, vec3(3, .25, 19.9));

	q = v - vec3(0, 11, 0);
	float archesUpperFloor = _min(sdCappedCylinder(vec3(q.y - 6, q.x, mod(q.z + 5, 10) - 5), vec2(4.5, 36)), sdCappedCylinder(vec3(mod(q.x + 5, 10) - 5, q.z, q.y - 6), vec2(4.5, 36)));
	archesUpperFloor = opSubtraction(archesUpperFloor, sdBox(q - vec3(0, 10.5, 0), vec3(25.5, 3.5, 25.5)));

	float arches = opSmoothUnion(archesUpperFloor, archesLowerFloor, .5);
	arches = _min(arches, sdBox(v + vec3(0, 0.75, 0), vec3(40.0, .5, 40.0)));
	arches = _min(arches, _min(sdBox(vec3(v.x - 0, v.y - 12, abs(v.z) - 5), vec3(25.0, 1., .25)), sdBox(vec3(abs(v.x) - 25, v.y - 12, v.z), vec3(.25, 1., 5))));
	sdf_agg = _min(opSubtraction(
		_min(sdBox(v - vec3(0, 12, 0), vec3(24.5, 12.25, 14.5)), sdBox(v, vec3(15, 35, 4))) // courtyard
		,
		_min(arches, opSubtraction(box2, box1))),
		plane);

	return sdf_agg;
}

vec3 calcWorldNormal(vec3 p)
{
	const float h = 0.0001;
	const vec2 k = vec2(1.0, -1.0);
	return normalize(xyy(k)*worldSDF1(p + xyy(k)*h) +
		yyx(k)*worldSDF1(p + yyx(k)*h) +
		yxy(k)*worldSDF1(p + yxy(k)*h) +
		xxx(k)*worldSDF1(p + xxx(k)*h));
}

RaymarchResult worldMarch(vec3 ro, vec3 rd, int _max_STEPS, float radius, float _maxDist)
{

	RaymarchResult marched;
	// default material
	marched.hit = true;
	float dist = 0;
	// vec3 dist=v;
	for (int i = 0; i < _max_STEPS; i++) {

		float samp = worldSDF1(ro) - radius; // find SDF at current march )
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