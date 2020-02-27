#include "main.h"
/*
 * We are rewrting this
 * Make rays actual rays instead of lines
 * make plane use a norm + a t value instead of a norm + pos
 * TODO: so far so good but the green sphere isn't casting a shadow
 * pretty sure this is a bug
 */

#define _maxf32 0x7F7FFFFF
#define _minf32 0xFF7FFFFF

// This is our world
_global RMaterial materials[6] = {0};
_global RPlane planes[] = {
    {{{0}, {0, 1, 0}}, 1},
};

_global RSphere spheres[] = {
    {{{0}, 1.0f}, 2},
    {{{3, 0, 2}, 1.0f}, 3},
    {{{-2, 2, 1}, 1.0f}, 4},
    {{{1, 3, 1}, 1.0f}, 5},
};

Vec3 GetRayOffset(u32 x, u32 y, Vec3 camera_x, Vec3 camera_y,
		  Vec2 half_grid_dim, WBackBufferContext buffer) {
	f32 half_grid_w = half_grid_dim.x;
	f32 half_grid_h = half_grid_dim.y;

	f32 grid_y = ((f32)y) / (f32)(buffer.height) * -2.0f + 1.0f;
	f32 grid_x = ((f32)x) / (f32)(buffer.width) * 2.0f - 1.0f;
	Vec3 offset_x = Vec3MulConstR(camera_x, grid_x * half_grid_w);
	Vec3 offset_y = Vec3MulConstR(camera_y, grid_y * half_grid_h);

	return Vec3Add(offset_x, offset_y);
}

Vec3 GetJitterOffset(Vec3 camera_x, Vec3 camera_y, Vec2 half_grid_dim,
		     WBackBufferContext buffer) {
	Vec3 start =
	    GetRayOffset(0, 0, camera_x, camera_y, half_grid_dim, buffer);
	Vec3 end =
	    GetRayOffset(1, 1, camera_x, camera_y, half_grid_dim, buffer);

	Vec3 diff = Vec3MulConstR(Vec3Sub(end, start),0.5f);

	diff.x *= RandNegOneToOne();
	diff.y *= RandNegOneToOne();
	diff.z *= RandNegOneToOne();

	return diff;
}

#define _bounce_count 8
#define _rays_per_pixel 16

Color3 CastRay(Ray3 ray) {
	Color3 out_color = {0};
	Vec3 atten = {1, 1, 1};

#define _debug_plane 0

#if _debug_plane
	b32 plane_hit = false;
#endif

	f32 cur_t = _maxf32;

	for (u32 r = 0; r < _bounce_count; r++) {
		u32 hit_material = 0;
		Vec3 next_point = {0};
		Vec3 next_normal = {0};

		for (u32 i = 0; i < _arraycount(planes); i++) {
			RPlane rplane = planes[i];

			Vec3 hit_point = {0};

			if (IntersectOutLine3Plane(ray, rplane.plane,
						   &hit_point)) {
				Vec3 hit_to_ray = Vec3Sub(hit_point, ray.pos);
				f32 t = DotVec3(hit_to_ray, ray.dir);

				if (t > 0 && t < cur_t) {
					next_normal = rplane.plane.norm;
					next_point = hit_point;
					hit_material = rplane.material_index;
					cur_t = t;
				}
			}
		}

		for (u32 i = 0; i < _arraycount(spheres); i++) {
			RSphere rsphere = spheres[i];

			Vec3 hit_point = {0};

			if (IntersectOutLine3Sphere(ray, rsphere.sphere,
						    &hit_point)) {
				Vec3 hit_to_ray = Vec3Sub(hit_point, ray.pos);
				f32 t = DotVec3(hit_to_ray, ray.dir);

				if (t < cur_t) {
					next_normal = GetSphereNormalVe3(
					    rsphere.sphere, hit_point);
					next_point = hit_point;
					hit_material = rsphere.material_index;
					cur_t = t;
				}
			}
		}

		RMaterial material = materials[hit_material];
		// if we actually hit something
		if (hit_material) {
			out_color = Vec3Add(out_color,
					    CompMulVec3(atten, material.emit));
			f32 dot =
			    DotVec3(Vec3MulConstR(ray.dir, -1.0f), next_normal);

			if (dot < 0.0f) {
				dot = 0.0f;
			}

#if 1
			Color3 refl_color = Vec3MulConstR(material.refl, dot);
#else
			Color3 refl_color = material.refl;
#endif

			atten = CompMulVec3(atten, refl_color);

			// NOTE: this is the direct bounce ray
			Vec3 bounce_ray =
			    NormalizeVec3(ReflectVec3(ray.dir, next_normal));
			Vec3 scatter_ray = {RandNegOneToOne(),
					    RandNegOneToOne(),
					    RandNegOneToOne()};

			if (DotVec3(scatter_ray, next_normal) < 0.0f) {
				scatter_ray =
				    ReflectVec3(scatter_ray, next_normal);
			}

			Vec3 next_dir = InterpolateVec3(scatter_ray, bounce_ray,
							material.scatter);

			ray.dir = NormalizeVec3(next_dir);
			ray.pos = next_point;

#if _debug_plane

			if (r == 0 && hit_material == 1) {
				plane_hit = true;
			}

			if (plane_hit && r == 1) {
				printf("PLANE HIT %d\n", hit_material);

				if (hit_material == 4) {
					Color3 new_color = {1, 1, 0};
					return new_color;
				}
			}
#endif
		}

		else {
			out_color = Vec3Add(out_color,
					    CompMulVec3(atten, material.emit));
			break;
		}
	}

	return out_color;
}

void MainRayCast(WBackBufferContext buffer) {
	// this setups up the materials

	// Sky material
	materials[0].emit.x = 0.3f;
	materials[0].emit.y = 0.4f;
	materials[0].emit.z = 0.5f;
	materials[0].scatter = 0;

	// plane material
	materials[1].refl.x = 0.5;
	materials[1].refl.y = 0.5f;
	materials[1].refl.z = 0.5f;
	materials[1].scatter = 0;

	// sphere material
	materials[2].refl.x = 0.7;
	materials[2].refl.y = 0.5f;
	materials[2].refl.z = 0.3f;
	materials[2].scatter = 0;

	materials[3].emit.x = 4.0;
	materials[3].emit.y = 0.0f;
	materials[3].emit.z = 0.0f;
	materials[3].scatter = 0;

	materials[4].refl.x = 0.2;
	materials[4].refl.y = 0.8f;
	materials[4].refl.z = 0.2f;
	materials[4].scatter = 0.7f;

	materials[5].refl.x = 0.4;
	materials[5].refl.y = 0.8f;
	materials[5].refl.z = 0.9f;
	materials[5].scatter = 0.85f;

	Vec3 world_y = {0, 1, 0};

	Vec3 camerapos = {0, 1.0f, -10.0f};

	Vec3 camera_z = NormalizeVec3(Vec3MulConstR(camerapos, -1.0f));
	Vec3 camera_x = NormalizeVec3(CrossVec3(world_y, camera_z));
	Vec3 camera_y = NormalizeVec3(CrossVec3(camera_z, camera_x));

	f32 grid_dist = 1.0f;
	f32 grid_w = 1.0f;
	f32 grid_h = 1.0f;

	if (buffer.width > buffer.height) {
		grid_h = ((f32)buffer.height) / (f32)buffer.width;
	}

	else if (buffer.height > buffer.width) {
		grid_w = ((f32)buffer.width) / (f32)buffer.height;
	}

	f32 half_grid_w = grid_w * 0.5f;
	f32 half_grid_h = grid_h * 0.5f;

	Vec2 half_grid_dim = {half_grid_w, half_grid_h};

	Vec3 grid_center = Vec3MulConstR(camera_z, grid_dist);

	for (u32 y = 0; y < buffer.height; y++) {
		for (u32 x = 0; x < buffer.width; x++) {
			Vec3 main_offset = GetRayOffset(
			    x, y, camera_x, camera_y, half_grid_dim, buffer);

			Color3 color = {0};

			for (u32 i = 0; i < _rays_per_pixel; i++) {
				Vec3 jitter_offset = GetJitterOffset(
				    camera_x, camera_y, half_grid_dim, buffer);

				Vec3 final_offset =
				    Vec3Add(main_offset, jitter_offset);

				Vec3 ray_dir = NormalizeVec3(
				    Vec3Add(grid_center, final_offset));

				Ray3 ray = {camerapos, ray_dir};
				color = Vec3Add(color, CastRay(ray));
			}

			color =
			    Vec3MulConstR(color, 1.0f / (f32)_rays_per_pixel);

			u32* pixel = buffer.pixels + (y * buffer.width) + x;
			*pixel = ColorToPixelColor(color);
		}
#define _enable_status 1

		// status
#if _enable_status
		{
			f32 cur_count = (f32)((y * buffer.width));
			f32 total_count = (f32)(buffer.width * buffer.height);

			f32 progress = 100.0f * (cur_count / total_count);

			printf("\rCasting progress: %f %%", (f64)progress);
		}

#endif
	}

#if _enable_status

	printf("\n");

#endif
}

s32 main(s32 argc, const s8** argv) {
	WWindowContext window = WCreateWindow(
	    "Raytracer",
	    (WCreateFlags)(W_CREATE_NORESIZE | W_CREATE_BACKEND_XLIB), 0, 0,
	    1080, 720);

	WBackBufferContext backbuffer = WCreateBackBuffer(&window);
	WWindowEvent event = {0};

	b32 run = 1;

	Vec3 camerapos = {0};

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
