

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <libHidDisplay.h>




static int DWIDTH = 0;
static int DHEIGHT = 0;
static teensyRawHidcxt_t hid_ctx;



static int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libHidDisplay_OpenDisplay(&hid_ctx, 0))
			return 1;
	}
	return 0;
}

int display_init ()
{
	
	if (!libHidDisplay_Open(&hid_ctx, RAWHID_INTERFACE, 0)){
		if (!libHidDisplay_Open(&hid_ctx, RAWHID_INTERFACE, 1)){
			if (!libHidDisplay_OpenDisplay(&hid_ctx, 0)){
				if (!openDisplayWait(500))
					return 0;
			}
		}
	}

	rawhid_header_t desc;
	libHidDisplay_GetConfig(&hid_ctx, &desc);
		
	hid_ctx.width = desc.u.cfg.width;
	hid_ctx.height = desc.u.cfg.height;
	hid_ctx.pitch = desc.u.cfg.pitch;
	hid_ctx.rgbClamp = desc.u.cfg.rgbMax;
	
	DWIDTH = hid_ctx.width;
	DHEIGHT = hid_ctx.height;

	if (!DWIDTH || !DHEIGHT){
		libHidDisplay_Close(&hid_ctx);
		return 0;
	}

	printf("Found device: %s\nWxH:%ix%i\n", desc.u.cfg.string, DWIDTH, DHEIGHT);
	return 1;
}

int main (int argc, char **cargv)
{

	printf("\n");
	if (!display_init()){
		printf("Device not found or in use\n");
		return 0;
	}
	
	if (argc > 1)
		libHidDisplay_TouchReportEnable(&hid_ctx, 1, atoi(cargv[1]));		// 1 through 11 (TOUCH_DIR_)
	else
		libHidDisplay_TouchReportEnable(&hid_ctx, 1, TOUCH_DIR_DEFAULT);
	
	printf("waiting\n");
	
	while (!kbhit()){
		//if (libHidDisplay_GetReportWait(&hid_ctx, &touch)){
			
		int reportType = 0;
		uint8_t report[128];
		
		if (libHidDisplay_GetReportWaitEx(&hid_ctx, &reportType, report)){
			if (reportType == RAWHID_OP_TOUCH){
				touch_t touch;
				memcpy(&touch, report, sizeof(touch));
				
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
			}else if (reportType == RAWHID_OP_ENCODER){
				encodersrd_t encoders;
				memcpy(&encoders, report, sizeof(encoders));
				//printf("encoder %i %i %i\n", encoders.size, encoders.total, encoders.changed);

				for (int i = 0; i < encoders.total; i++){
					encoderrd_t *enc = &encoders.encoder[i];
					if (enc->buttonPress){
						printf("Button %i pressed\n", i+1);
					}

					if (enc->positionChange){
						printf("Rotary %i position changed by %i\n", i+1, enc->positionChange);
					}
				}
			}
		}else{
			Sleep(5);
		}
	}
	printf("done\n");

	libHidDisplay_TouchReportEnable(&hid_ctx, 0, 0);
	libHidDisplay_Close(&hid_ctx);
	return 0;

}
