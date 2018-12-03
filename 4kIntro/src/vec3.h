#ifndef VEC3_H
#define VEC3_H

#include <math.h>
/**
* A barebones header-only 3-vector for demos
* @author Benjamin Welsh
* broog.net
*/

// data only vec4
struct Vec4 { float x, y, z, w; };

struct vec2 {
	float x, y;
	vec2() {}
	vec2(float x) {
		this->x = x;
		this->y = x;
	}

	vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	vec2 operator+(const vec2& v) {
		return vec2{ x + v.x,y + v.y };
	}
	void operator+=(const vec2& v) {
		x += v.x;
		y += v.y;
	}
	vec2 operator-(const vec2& v) {
		return vec2{ x - v.x,y - v.y };
	}
	vec2 operator*(const float& v) {
		return vec2{ x*v, y*v };
	}
};


struct vec3 {
	float x, y, z;
	vec3() {}
	vec3(float x) {
		this->x = x;
		this->y = x;
		this->z = x;
	}

	vec3(Vec4 v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
	}

	vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	vec3 operator+(const vec3& v) {
		return vec3{ x + v.x,y + v.y,z + v.z };
	}
	void operator+=(const vec3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	vec3 operator-(const vec3& v) {
		return vec3{ x - v.x,y - v.y,z - v.z };
	}
	vec3 operator*(const float& v) {
		return vec3{ x*v, y*v, z*v };
	}
	vec2 xz() {
		return vec2{ x,z };
	}
	vec3 x0z() {
		return vec3{ x,0,z };
	}
};

inline float dot(vec3 a, vec3 b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}
inline float length(vec3 v) {
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}
inline float length(vec2 v) {
	return sqrt(v.x*v.x + v.y*v.y);
}

inline vec3 normalize(vec3 v) {
	float l = length(v);
	if (l == 0)return vec3{};
	return vec3{ v.x / l,v.y / l,v.z / l };
}
inline vec3 cross(vec3 a, vec3 b) {
	return vec3{
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	};
}

inline vec3 abs(vec3 v) {
	return vec3{
		v.x > 0 ? v.x : -v.x,
		v.y > 0 ? v.y : -v.y,
		v.z > 0 ? v.z : -v.z
	};
}

inline vec2 abs(vec2 v) {
	return vec2{
		v.x > 0 ? v.x : -v.x,
		v.y > 0 ? v.y : -v.y,
	};
}



/**
* A barebones header-only 4-vector for demos
* @author Benjamin Welsh
* broog.
*/
struct vec4 {
	float x, y, z, w;
	vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	vec4(vec3 v, float w) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = w;
	}
	vec4 operator+(const vec4& v) {
		return vec4{ x + v.x,y + v.y,z + v.z,w + v.w };
	}
	void operator+=(const vec4& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
	}
	vec4 operator-(const vec4& v) {
		return vec4{ x - v.x,y - v.y,z - v.z ,w-v.w};
	}
	vec4 operator*(const float& v) {
		return vec4{ x*v, y*v, z*v, w*v };
	}
	vec3 xyz() {
		return vec3{x, y, z};
	}
	Vec4 dataOnly() {
		return Vec4{x, y, z, w};
	}
};

inline vec3 xyy(vec2 v) {
	return vec3{ v.x,v.y,v.y };
}
inline vec3 yyx(vec2 v) {
	return vec3{ v.y,v.y,v.x };
}
inline vec3 yxy(vec2 v) {
	return vec3{ v.y,v.x,v.y };
}
inline vec3 xxx(vec2 v) {
	return vec3{ v.x,v.x,v.x };
}

/* Incedent, Normal */
inline vec3 reflect(vec3 I, vec3 N) {
	return I - N * 2.0f * dot(N, I);
}
//inline float length(vec4 v) {
//	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
//}

//inline vec4 normalize(vec4 v) {
//	float l = length(v);
//	if (l == 0)return vec4{};
//	return vec4{ v.x / l,v.y / l,v.z / l };
//}
//inline vec4 cross(vec4 a, vec4 b) {
//	return vec4{
//		a.y*b.z - a.z*b.y,
//		a.z*b.x - a.x*b.z,
//		a.x*b.y - a.y*b.x
//	};
//}
#endif // !VEC3_H
