#include "stdio.h"
#include "stdlib.h"

#include "mode.h"
#include "ttype.h"

#include "wwindow.h"

s32 main(s32 argc, const s8** argv){

	WWindowContext window = WCreateWindow("Raytracer",
			(WCreateFlags)(W_CREATE_NORESIZE | W_CREATE_BACKEND_XLIB),
			0,0,
			1280,720);

	WBackBuffer backbuffer = WCreateBackBuffer(&window);

	WWindowEvent event = {};

	b32 run = true;

	while(true){

		while(WWaitForWindowEvent(&window,&event){

				if(event.type == W_EVENT_KBEVENT_KEYDOWN){
				run = false;	
				}	

				}


				}

				return 0;
				}
