#include "ccolor.h"
#include "mmath.h"
#include "mode.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ttype.h"
#include "wwindow.h"

typedef struct Sphere {
	Vec3 pos;
	f32 radius;
} Sphere;

typedef struct RMaterial {
	Color3 emit;
	Color3 refl;
	f32 scatter;  // 0 diffuse and 1 is shiny
} RMaterial;

typedef struct RSphere {
	Sphere sphere;
	u32 material_index;
} RSphere;

typedef struct RPlane {
	Plane plane;
	u32 material_index;
} RPlane;

f32 RandZeroToOne() { return (f32)rand() / (f32)RAND_MAX; }
f32 RandNegOneToOne() { return (2.0f * RandZeroToOne()) - 1.0f; }
Vec3 ReflectVec3(Vec3 vec, Vec3 normal) {
	return Vec3Sub(vec, Vec3MulConstR(ProjectOntoVec3(vec, normal), 2.0f));
}

Vec3 GetSphereNormalVe3(Sphere sphere, Point3 point_on_sphere) {
	Vec3 normal = Vec3Sub(point_on_sphere, sphere.pos);

#if 0
	{
		f32 magnitude = MagnitudeVec3(normal);
		b32 condition =
		    (sphere.radius > magnitude || sphere.radius < magnitude);

		if(condition){
			printf("ERROR: %f\n",(f64)magnitude);
		}
		//_kill("point is not on sphere\n", condition);
	}
#endif

	return NormalizeVec3(normal);
}

b32 IntersectOutLine3Sphere(Line3 line, Sphere sphere, Point3* point) {
	RSphere s = {.sphere = {.pos = {0}, 3}, .material_index = 2};

	Vec3 ray_to_sphere = Vec3Sub(sphere.pos, line.pos);

	f32 a = DotVec3(line.dir, line.dir);
	f32 b = -2.0f * DotVec3(line.dir, ray_to_sphere);
	f32 c = DotVec3(ray_to_sphere, ray_to_sphere) -
		(sphere.radius * sphere.radius);
	f32 discriminant = (b * b) - (4.0f * a * c);

	if (discriminant < 0.0f) {
		return 0;
	}

	f32 root = sqrtf(discriminant);
	f32 t0 = ((-1.0f * b) + root) / (2.0f * a);
	f32 t1 = ((-1.0f * b) - root) / (2.0f * a);

	// This is ray specific. TODO: don't treat rays as lines
	if (t0 <= _f32_error_offset && t1 <= _f32_error_offset) {
		return 0;
	}

#if 0
	//NOTE: this is line specific
	//TODO: think about this. I don't think we're there yet
	f32 error_margin = _f32_error_offset;
	f32 neg_error_margin = _f32_error_offset * -1.0f;
	b32 line_condition = 
		(t0 > neg_error_margin && t0 < error_margin) &&
		(t1 > neg_error_margin && t1 < error_margin);
	if (line_condition) {
		return 0;
	}
#endif

	// printf("%f %f\n", (f64)t0, (f64)t1);

	if (t0 < t1) {
		*point = Vec3Add((Vec3MulConstR(line.dir, t0)), line.pos);
	} else {
		*point = Vec3Add((Vec3MulConstR(line.dir, t1)), line.pos);
	}

	return 1;
}

Vec3 ClampVec3(Vec3 vec, f32 lower, f32 upper) {
	vec.x = _clampf(vec.x, lower, upper);
	vec.y = _clampf(vec.y, lower, upper);
	vec.z = _clampf(vec.z, lower, upper);

	return vec;
}

Vec4 ClampVec4(Vec4 vec, f32 lower, f32 upper) {
	vec.x = _clampf(vec.x, lower, upper);
	vec.y = _clampf(vec.y, lower, upper);
	vec.z = _clampf(vec.z, lower, upper);
	vec.w = _clampf(vec.w, lower, upper);

	return vec;
}

f32 LinearToSRGB(f32 linear) {
	f32 ret = 0.0f;

	if (linear < 0.0031308f) {
		ret = linear * 12.92f;
	}

	else {
		ret = 1.055f * powf(linear,1.0f/2.4f) - 0.055f;
	};

	return ret;
}
u32 ColorToPixelColor(Color3 color) {
	ClampVec3(color, 0.0f, 1.0f);

	color.R = LinearToSRGB(color.R);
	color.G = LinearToSRGB(color.G);
	color.B = LinearToSRGB(color.B);

	return _encode_argb((u32)(255.0f), (u32)(color.r * 255.0f),
			    (u32)(color.g * 255.0f), (u32)(color.b * 255.0f));
}

