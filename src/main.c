#include "mmath.h"
#include "mode.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ttype.h"
#include "wwindow.h"
//#include "random.h"

/*
NOTE: We won't bother with AA for now. It's basically shooting more rays into a
pixel and blending.
 */

#define _rays_per_pixel 4
#define _bounce_depth 32

typedef struct Sphere {
	Vec3 pos;
	f32 radius;
} Sphere;

typedef struct RMaterial {
	Color diffuse;
	f32 scatter;  // 0 reflective, 1 is rough
} RMaterial;

typedef struct RSphere {
	Sphere sphere;
	u32 material_index;
} RSphere;

typedef struct RPlane {
	Plane plane;
	u32 material_index;
} RPlane;

_global RMaterial materials[] = {
    {0, 0, 1, 1},      {1, 0, 0, 1, 1}, {0, 1, 0, 1, 0.8},
    {1, 1, 0, 1, 0.2}, {0, 1, 1, 1, 1},
};
_global RSphere spheres[] = {
    {.sphere = {{0, 0, 10}, 3}, .material_index = 2},
    //{.sphere = {{-15, 5, 20}, 5}, .material_index = 3},
    //{.sphere = {{20, 8, 30}, 7}, .material_index = 4},
};
_global RPlane planes[] = {
    {.plane = {{0, -1, 0}, {0, 1, 0}}, .material_index = 1},
};

f32 RandZeroToOne() { return (f32)rand() / (f32)RAND_MAX; }
f32 RandNegOneToOne() {
	f32 value = (f32)rand() / (f32)(RAND_MAX >> 2);
	return value;
}
Vec3 ReflectVec3(Vec3 vec, Vec3 normal) {
	return Vec3Sub(vec, Vec3MulConstR(ProjectOntoVec3(vec, normal), 2.0f));
}

Vec3 GetSphereNormalVe3(Sphere sphere, Point3 point_on_sphere) {
	Vec3 normal = Vec3Sub(point_on_sphere, sphere.pos);

#ifdef DEBUG
//_kill("point is not on sphere\n",MagnitudeVec3(normal) !=
// sphere.radius);
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

	if (t0 <= 0.0f && t1 <= 0.0f) {
		return 0;
	}

	if (t0 < t1) {
		*point = Vec3Add((Vec3MulConstR(line.dir, t0)), line.pos);
	} else {
		*point = Vec3Add((Vec3MulConstR(line.dir, t1)), line.pos);
	}

	return 1;
}

u32 ColorToPixelColor(Color color) {
	color.r = _clampf(color.r, 0.0f, 1.0f);
	color.g = _clampf(color.g, 0.0f, 1.0f);
	color.b = _clampf(color.b, 0.0f, 1.0f);
	color.a = _clampf(color.a, 0.0f, 1.0f);

	return _encode_argb((u32)(color.a * 255.0f), (u32)(color.r * 255.0f),
			    (u32)(color.g * 255.0f), (u32)(color.b * 255.0f));
}

Vec3 ClampVec3(Vec3 vec) {
	vec.x = _clampf(vec.x, 0.0f, 1.0f);
	vec.y = _clampf(vec.y, 0.0f, 1.0f);
	vec.z = _clampf(vec.z, 0.0f, 1.0f);

	return vec;
}

Vec4 ClampVec4(Vec4 vec) {
	vec.x = _clampf(vec.x, 0.0f, 1.0f);
	vec.y = _clampf(vec.y, 0.0f, 1.0f);
	vec.z = _clampf(vec.z, 0.0f, 1.0f);
	vec.w = _clampf(vec.w, 0.0f, 1.0f);

	return vec;
}

void ComputeBounceRays(Ray3 ray, Vec3* attenuation, u32 depth) {
	if (depth > _bounce_depth) {
		return;
	}

	depth++;

	u32 is_hit = 0;
	Vec3 next_point = {0, 0, 2147483647.0f};
	Vec3 next_normal = {0};

	for (u32 i = 0; i < _arraycount(planes); i++) {
		Point3 hitpoint = {0};
		RPlane plane = planes[i];

		if (IntersectOutLine3Plane(ray, plane.plane, &hitpoint)) {
			Vec3 ray_to_hitpoint =
			    NormalizeVec3(Vec3Sub(hitpoint, ray.pos));
			f32 dot = DotVec3(ray_to_hitpoint, ray.dir);

			if (dot > 0.0f && hitpoint.z < next_point.z) {
				is_hit = 1;
				next_point = hitpoint;
				next_normal = plane.plane.norm;
			}
		}
	}

	for (u32 i = 0; i < _arraycount(spheres); i++) {
		RSphere sphere = spheres[i];
		Point3 hitpoint = {0};

		if (IntersectOutLine3Sphere(ray, sphere.sphere, &hitpoint)) {
			if (hitpoint.z < next_point.z) {
				is_hit = 1;
				next_point = hitpoint;
				next_normal =
				    GetSphereNormalVe3(sphere.sphere, hitpoint);
			}
		}
	}

	if (is_hit) {
		// This is the direct reflected light
		Vec3 reflected =
		    NormalizeVec3(ReflectVec3(ray.dir, next_normal));
		Ray3 next_ray = {next_point, reflected};

		ComputeBounceRays(next_ray, attenuation, depth);

		f32 dot = DotVec3(ray.dir, next_normal);
		*attenuation = Vec3MulConstR(*attenuation, dot);
	}
}

// we need to produce the bounces as well
Color CastRay(Ray3 ray) {
	u32 mat_index = 0;
	Vec3 next_point = {0, 0, 2147483647.0f};
	Vec3 next_normal = {0};

	for (u32 i = 0; i < _arraycount(planes); i++) {
		Point3 hitpoint = {0};
		RPlane plane = planes[i];

		if (IntersectOutLine3Plane(ray, plane.plane, &hitpoint)) {
			Vec3 ray_to_hitpoint =
			    NormalizeVec3(Vec3Sub(hitpoint, ray.pos));
			f32 dot = DotVec3(ray_to_hitpoint, ray.dir);

			if (dot > 0.0f && hitpoint.z < next_point.z) {
				mat_index = plane.material_index;

				next_point = hitpoint;
				next_normal = plane.plane.norm;
			}
		}
	}

	for (u32 i = 0; i < _arraycount(spheres); i++) {
		RSphere sphere = spheres[i];
		Point3 hitpoint = {0};

		if (IntersectOutLine3Sphere(ray, sphere.sphere, &hitpoint)) {
			if (hitpoint.z < next_point.z) {
				mat_index = sphere.material_index;

				next_point = hitpoint;
				next_normal =
				    GetSphereNormalVe3(sphere.sphere, hitpoint);

// output the sphere normal as the color
#if 0
				{
					Vec3 normal = GetSphereNormalVe3(
					    sphere.sphere, hitpoint);
					return ColorToPixelColor(
					    Vec3ToVec4(normal));
				}
#endif
			}
		}
	}

	Color color = materials[mat_index].diffuse;
	if (mat_index) {

		Vec3 attenuation = {0};

#if 1
		// This is the direct reflected light
		{
			Vec3 refl_attenuation = {1, 1, 1};

			Vec3 reflected =
			    NormalizeVec3(ReflectVec3(ray.dir, next_normal));
			Ray3 next_ray = {next_point,
					 reflected};  // directly reflected ray

			ComputeBounceRays(next_ray, &refl_attenuation, 0);

			f32 reflective =
			    1.0f - materials[mat_index].scatter;

			refl_attenuation =
			    Vec3MulConstR(refl_attenuation, reflective);

			attenuation = Vec3Add(attenuation, refl_attenuation);
		}

#endif

#if 1

		for (u32 i = 0; i < _rays_per_pixel; i++) {
			// this is the scattered light

			Vec3 scattered_attenuation = {1, 1, 1};

			Vec3 scattered = {RandNegOneToOne(), RandNegOneToOne(),
					  RandNegOneToOne()};

			if (DotVec3(scattered, next_normal) < 0.0f) {
				scattered = NormalizeVec3(
				    ReflectVec3(scattered, next_normal));
			}

			Ray3 next_ray = {next_point, scattered};
			ComputeBounceRays(next_ray, &scattered_attenuation, 0);
			f32 scatter = materials[mat_index].scatter;
			scattered_attenuation =
			    Vec3MulConstR(scattered_attenuation, scatter);
			attenuation =
			    Vec3Add(attenuation, scattered_attenuation);
		}

#endif

		attenuation = ClampVec3(attenuation);

		color = CompMulVec4(color, Vec3ToVec4(attenuation));
	}

	return color;
}

// maps a pixel to a 3d grid. the vector is at the top left of the pixel, not
// the center
Vec3 PixelPosToGridPos(u32 x, u32 y, u32 width, u32 height, f32 z) {
	/*
	 *grid coords go from -1 <= x <= 1 and -1 <= y <= 1
	 * */
	Vec3 gridpos = {0};

	f32 h_w = (f32)(width >> 1);
	f32 h_h = (f32)(height >> 1);

	gridpos.x = ((f32)(x)-h_w) / h_w;
	gridpos.y = (((f32)(y)-h_h) / h_h) * -1.0f;
	gridpos.z = z;

	if (width > height) {
		gridpos.y *= (f32)height / (f32)width;
	} else if (height > width) {
		gridpos.x *= (f32)width / (f32)height;
	}

	return gridpos;
}

void CastRays(WBackBufferContext* buffer, Vec3 camerapos, f32 z) {
	Vec3 grid_width;
	Vec3 grid_height;

	{
		Vec3 start =
		    PixelPosToGridPos(0, 0, buffer->width, buffer->height, 1);

		Vec3 x =
		    PixelPosToGridPos(1, 0, buffer->width, buffer->height, 1);
		Vec3 y =
		    PixelPosToGridPos(0, 1, buffer->width, buffer->height, 1);

		grid_width = Vec3Sub(x, start);
		grid_height = Vec3Sub(y, start);
	}

	for (u32 y = 0; y < buffer->height; y++) {
		for (u32 x = 0; x < buffer->width; x++) {
			Vec3 gridpos = PixelPosToGridPos(x, y, buffer->width,
							 buffer->height, 1.0f);

			Color color = {0};

			for (u32 i = 0; i < _rays_per_pixel; i++) {
				Vec3 rand_offset =
				    Vec3Add(grid_width, grid_height);
				rand_offset =
				    Vec3MulConstR(rand_offset, RandZeroToOne());
				gridpos = Vec3Add(gridpos, rand_offset);
				Vec3 dir =
				    NormalizeVec3(Vec3Sub(gridpos, camerapos));
				Ray3 ray = {camerapos, dir};
				color = Vec4Add(color, CastRay(ray));
			}

			color = Vec4DivConstR(color, (f32)_rays_per_pixel);

			u32* pixel = buffer->pixels + (buffer->width * y) + x;
			*pixel = ColorToPixelColor(color);
		}
	}
}

s32 main(s32 argc, const s8** argv) {
	WWindowContext window = WCreateWindow(
	    "Raytracer",
	    (WCreateFlags)(W_CREATE_NORESIZE | W_CREATE_BACKEND_XLIB), 0, 0,
	    720, 480);

	WBackBufferContext backbuffer = WCreateBackBuffer(&window);
	WWindowEvent event = {0};

	b32 run = 1;

	Vec3 camerapos = {0};

	printf("casting %d rays...\n",
	       backbuffer.width * backbuffer.height * _rays_per_pixel);

	CastRays(&backbuffer, camerapos, 1.0f);

	while (run) {
		WPresentBackBuffer(&window, &backbuffer);

		while (WWaitForWindowEvent(&window, &event)) {
			if (event.type == W_EVENT_KBEVENT_KEYDOWN ||
			    event.type == W_EVENT_CLOSE) {
				run = 0;
			}
		}
	}

	return 0;
}
