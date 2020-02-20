#include "main.h"
/*
 * We are rewrting this
 */

#define _maxf32 0x7F7FFFFF
#define _minf32 0xFF7FFFFF

// This is our world
_global RMaterial materials[] = {{0, 0, 0, 1}, {1, 0, 0, 1}};
_global RPlane planes[] = {
    {{{0}, {0, 1, 0}}, 1},
};
//_global RSphere spheres[] = {};

Color CastRay(Ray3 ray) {
	u32 hit_material = 0;


	Color out_color = materials[hit_material].diffuse;
	return out_color;
}

void MainRayCast(WBackBufferContext buffer) {
	Vec3 world_y = {0, 1, 0};

	Vec3 camerapos = {0, 10.0f, -1.0f};
	Vec3 camera_z = Vec3MulConstR(NormalizeVec3(camerapos), -1.0f);
	Vec3 camera_x = NormalizeVec3(CrossVec3(world_y, camera_z));
	Vec3 camera_y = NormalizeVec3(CrossVec3(camera_z, camera_x));

	f32 grid_dist = 1.0f;
	f32 grid_w = 1.0f;
	f32 grid_h = 1.0f;
	f32 half_grid_w = grid_w * 0.5f;
	f32 half_grid_h = grid_h * 0.5f;

	Vec3 grid_origin = Vec3MulConstR(camera_z, grid_dist);

	for (u32 y = 0; y < buffer.height; y++) {
		for (u32 x = 0; x < buffer.width; x++) {
			f32 grid_y =
			    ((f32)y) / (f32)(buffer.height) * 2.0f - 1.0f;
			f32 grid_x =
			    ((f32)x) / (f32)(buffer.width) * 2.0f - 1.0f;

			Vec3 offset_x =
			    Vec3MulConstR(camera_x, grid_x * half_grid_w);
			Vec3 offset_y =
			    Vec3MulConstR(camera_y, grid_y * half_grid_h);

			Vec3 ray_dir =
			    Vec3Add(grid_origin, Vec3Add(offset_x, offset_y));

			Ray3 ray = {camerapos, ray_dir};

			Color color = CastRay(ray);

			u32* pixel = buffer.pixels + (y * buffer.width) + x;
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

	printf("casting %d rays...\n", backbuffer.width * backbuffer.height);

	MainRayCast(backbuffer);

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
