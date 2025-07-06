#include "vec3.h"

#include <math.h>

Vec3 vec3_create(double x, double y, double z){
	Vec3 ret = {x, y, z};

	return ret;
}

Vec3 vec3_add(Vec3 a, Vec3 b){
	Vec3 ret = {
		a.x + b.x,
		a.y + b.y,
		a.z + b.z
	};
	
	return ret;
}

Vec3 vec3_sub(Vec3 a, Vec3 b){
	Vec3 ret = {
		a.x - b.x,
		a.y - b.y,
		a.z - b.z
	};
	
	return ret;
}

Vec3 vec3_mul(Vec3 a, double n){
	Vec3 ret = {
		a.x * n,
		a.y * n,
		a.z * n
	};
	
	return ret;
}

double vec3_sizeSqr(Vec3 a){
	return a.x*a.x + a.y*a.y + a.z*a.z;
}

double vec3_size(Vec3 a){
	return sqrt(vec3_sizeSqr(a));
}

Vec3 vec3_normalize(Vec3 a){
	Vec3 ret = {0, 0, 0};
	double size = vec3_size(a);

	if(size == 0) return ret;

	ret.x = a.x / size;
	ret.y = a.y / size;
	ret.z = a.z / size;

	return ret;
}

double vec3_dotProduct(Vec3 a, Vec3 b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 vec3_crossProduct(Vec3 a, Vec3 b){
	Vec3 normal = {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
	};

	return normal;
}

Vec3 vec3_rotateX(Vec3 a, double angle){
	Vec3 ret = {
		cos(angle) * a.x - sin(angle) * a.y,
		sin(angle) * a.x + cos(angle) * a.y,
		a.z
	};

	return ret;
}
