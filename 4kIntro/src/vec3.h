#pragma once
#include <math.h>
/**
* A barebones header-only 3-vector for demos
* @author Benjamin Welsh
* broog.
*/
struct Vec3 {
	double x, y, z;
	Vec3 operator+(const Vec3& v) {
		return Vec3{ x + v.x,y + v.y,z + v.z };
	}
	void operator+=(const Vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	Vec3 operator-(const Vec3& v) {
		return Vec3{ x - v.x,y - v.y,z - v.z };
	}
	Vec3 operator*(const float& v) {
		return Vec3{ x*v, y*v, z*v };
	}
};

inline float length(Vec3 v) {
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline Vec3 normalize(Vec3 v) {
	float l = length(v);
	if (l == 0)return Vec3{};
	return Vec3{ v.x / l,v.y / l,v.z / l };
}
inline Vec3 cross(Vec3 a, Vec3 b) {
	return Vec3{
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	};
}