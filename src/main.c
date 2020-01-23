#include "mmath.h"
#include "mode.h"
#include "stdio.h"
#include "stdlib.h"
#include "ttype.h"
#include "wwindow.h"

void ClearBackBuffer(WBackBufferContext* buffer) {
	for (u32 y = 0; y < buffer->height; y++) {
		for (u32 x = 0; x < buffer->width; x++) {
			u32* pixel = buffer->pixels + (buffer->width * y) + x;
			// note: default format is argb
			*pixel = 0xFFFF0000;
		}
	}
}

s32 main(s32 argc, const s8** argv) {
	WWindowContext window = WCreateWindow(
	    "Raytracer",
	    (WCreateFlags)(W_CREATE_NORESIZE | W_CREATE_BACKEND_XLIB), 0, 0,
	    1280, 720);

	WBackBufferContext backbuffer = WCreateBackBuffer(&window);

	WWindowEvent event = {0};

	ClearBackBuffer(&backbuffer);

	b32 run = 1;

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
