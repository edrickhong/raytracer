#include "ccolor.h"
#include "mmath.h"
#include "mode.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ttype.h"
#include "wwindow.h"
#include "ttimer.h"


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

