#ifndef VEC3_H
#define VEC3_H

typedef struct{
	double x, y, z;
} Vec3;

Vec3 vec3_create(double x, double y, double z);

Vec3 vec3_add(Vec3 a, Vec3 b);

Vec3 vec3_sub(Vec3 a, Vec3 b);

Vec3 vec3_mul(Vec3 a, double n);

double vec3_sizeSqr(Vec3 a);

double vec3_size(Vec3 a);

Vec3 vec3_normalize(Vec3 a);

double vec3_dotProduct(Vec3 a, Vec3 b);

Vec3 vec3_crossProduct(Vec3 a, Vec3 b);

Vec3 vec3_rotateX(Vec3 a, double angle);

#endif
