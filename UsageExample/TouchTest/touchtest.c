

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>

#include "../../libTeensyRawHid/libTeensyRawHid.h"




static int DWIDTH = 0;
static int DHEIGHT = 0;
static teensyRawHidcxt_t hid_ctx;



static int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libTeensyRawHid_OpenDisplay(&hid_ctx))
			return 1;
	}
	return 0;
}

int display_init ()
{
	if (!libTeensyRawHid_OpenDisplay(&hid_ctx)){
		if (!openDisplayWait(500))
			return 0;
	}

	rawhid_header_t desc;
	libTeensyRawHid_GetConfig(&hid_ctx, &desc);
		
	hid_ctx.width = desc.u.cfg.width;
	hid_ctx.height = desc.u.cfg.height;
	hid_ctx.pitch = desc.u.cfg.pitch;
	hid_ctx.rgbClamp = desc.u.cfg.rgbMax;
	
	DWIDTH = hid_ctx.width;
	DHEIGHT = hid_ctx.height;

	if (!DWIDTH || !DHEIGHT){
		libTeensyRawHid_Close(&hid_ctx);
		return 0;
	}

	//printf("Display Width:%i Height:%i\n%s\n", DWIDTH, DHEIGHT, desc.u.cfg.string);
	printf("Device: %s\n", desc.u.cfg.string);
	return 1;
}

int main (int margc, char **cargv)
{

	printf("\n");
	if (!display_init()){
		printf("Device not found or in use\n");
		return 0;
	}
	
	touch_t touch;
	libTeensyRawHid_TouchReportEnable(&hid_ctx, 1, TOUCH_DIR_RLBT);

	printf("waiting\n");
	while (!kbhit()){
		if (libTeensyRawHid_GetReportWait(&hid_ctx, &touch)){
			if (touch.flags == RAWHID_OP_TOUCH_POINTS){
#if 0
				// report multi touch
				for (int i = 0; i < touch.tPoints; i++)
					printf("%i, %i, %i %i\n", touch.tPoints, touch.time, touch.points[i].x, touch.points[i].y);
#else
				// report single touch
				printf("%i, %i %i\n", touch.time, touch.x, touch.y);
#endif
			}else if (touch.flags == RAWHID_OP_TOUCH_RELEASE){
				printf("Released\n");
			}
		}else{
			Sleep(5);
		}
	}
	printf("done\n");

	libTeensyRawHid_TouchReportEnable(&hid_ctx, 0, 0);
	libTeensyRawHid_Close(&hid_ctx);
	return 0;

}
