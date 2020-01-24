#include "mmath.h"
#include "mode.h"
#include "stdio.h"
#include "stdlib.h"
#include "ttype.h"
#include "wwindow.h"

typedef struct Sphere {
	Vec3 pos;
	f32 radius;
} Sphere;

b32 IntersectOutLine3Sphere(Line3 line, Sphere sphere, Point3* point) {

	Vec3 sphere_to_ray = Vec3Sub(sphere.pos,line.pos);

	f32 a = DotVec3(line.dir,line.dir);
	f32 b = -2.0f * DotVec3(line.dir,sphere_to_ray);
	f32 c = DotVec3(sphere_to_ray,sphere_to_ray) - (sphere.radius * sphere.radius);
	f32 discriminant = (b * b) - (4.0f * a * c);

	if(discriminant < 0.0f){
		return 0;
	}

	f32 root = sqrtf(discriminant);
	f32 t0 = ((-1.0f * b) + root)/(2.0f * a);
	f32 t1 = ((-1.0f * b) - root)/(2.0f * a);

	if(t0 < t1){
		*point = Vec3Add((Vec3MulConstR(line.dir,t0)),line.pos);
	}
	else{
		*point = Vec3Add((Vec3MulConstR(line.dir,t1)),line.pos);
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

u32 CastRay(Vec3 camerapos, Vec3 gridpos) {
	Color color = {0, 0, 1, 1};

	Vec3 dir = NormalizeVec3(Vec3Sub(gridpos, camerapos));
	Ray3 ray = {camerapos, dir};
	Vec3 last_hitpoint = {1000,1000,1000};

	{
		// hardcoded plane
		Plane plane = {{0, -1.0f, 0}, {0, 1, 0}};
		Point3 hitpoint = {0};

		if (IntersectOutLine3Plane(ray, plane, &hitpoint)) {
			Vec3 camera_to_hitpoint =
			    NormalizeVec3(Vec3Sub(hitpoint, camerapos));
			f32 dot = DotVec3(camera_to_hitpoint, dir);

			if (dot > 0.0f) {
				color.r = 1.0f;
				color.g = 0.0f;
				color.b = 0.0f;

				last_hitpoint = hitpoint; 
			}
		}
	}

	{
		// hardcoded sphere
		Sphere sphere = {{0, 0.0f, 10.0f}, 3.0f};
		Point3 hitpoint = {0};

		if(IntersectOutLine3Sphere(ray,sphere,&hitpoint)){

			Vec3 lasthit_to_camera = Vec3Sub(last_hitpoint,camerapos);
			Vec3 hitpoint_to_camera = Vec3Sub(hitpoint,camerapos);

			if(MagnitudeVec3(hitpoint_to_camera) < MagnitudeVec3(lasthit_to_camera)){
				color.r = 0.0f;
				color.g = 1.0f;
				color.b = 0.0f;
			}

		}
	}

	return ColorToPixelColor(color);
}

Vec3 PixelPosToGridPos(Vec3 camerapos, u32 x, u32 y, u32 width, u32 height,
		       f32 z) {
	/*
	 *grid coords go from -1 <= x <= 1 and -1 <= y <= 1
	 * */
	Vec3 gridpos = {0};

	f32 h_w = (f32)(width >> 1);
	f32 h_h = (f32)(height >> 1);

	gridpos.x = ((f32)(x)-h_w) / h_w;
	gridpos.y = (((f32)(y)-h_h) / h_h) * -1.0f;
	gridpos.z = z;

	return gridpos;
}

void CastRays(WBackBufferContext* buffer, Vec3 camerapos, f32 z) {
	for (u32 y = 0; y < buffer->height; y++) {
		for (u32 x = 0; x < buffer->width; x++) {
			Vec3 gridpos =
			    PixelPosToGridPos(camerapos, x, y, buffer->width,
					      buffer->height, 1.0f);

			u32* pixel = buffer->pixels + (buffer->width * y) + x;
			*pixel = CastRay(camerapos, gridpos);
		}
	}
}

s32 main(s32 argc, const s8** argv) {
	WWindowContext window = WCreateWindow(
	    "Raytracer",
	    (WCreateFlags)(W_CREATE_NORESIZE | W_CREATE_BACKEND_XLIB), 0, 0,
	    1280, 1280);

	WBackBufferContext backbuffer = WCreateBackBuffer(&window);

	WWindowEvent event = {0};

	b32 run = 1;

	Vec3 camerapos = {0};

	while (run) {
		CastRays(&backbuffer, camerapos, 1.0f);
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
