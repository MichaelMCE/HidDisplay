

/*

	Remotely Reset Teensy 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <libHidDisplay.h>




static int DWIDTH = 0;
static int DHEIGHT = 0;
static int DPITCH = 0;
static teensyRawHidcxt_t hid_ctx;



static int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		// try another display if available
		if (libHidDisplay_Open(&hid_ctx, RAWHID_INTERFACE, i&0x07))
			return 1;
	}
	return 0;
}

int display_init ()
{
	if (!libHidDisplay_OpenDisplay(&hid_ctx, 0)){
		if (!openDisplayWait(500))
			return 0;
	}

	rawhid_header_t desc;
	libHidDisplay_GetConfig(&hid_ctx, &desc);
		
	hid_ctx.width = desc.u.cfg.width;
	hid_ctx.height = desc.u.cfg.height;
	hid_ctx.pitch = desc.u.cfg.pitch;
	hid_ctx.rgbClamp = desc.u.cfg.rgbMax;
	
	DWIDTH = hid_ctx.width;
	DHEIGHT = hid_ctx.height;
	DPITCH = hid_ctx.pitch;
	
	if (!DWIDTH || !DHEIGHT){
		libHidDisplay_Close(&hid_ctx);
		return 0;
	}

	void *dummyImage = calloc(DPITCH, DHEIGHT);
	if (dummyImage){
		libHidDisplay_WriteImage(&hid_ctx, dummyImage);		// ensure the pipe is clean
		libHidDisplay_WriteImage(&hid_ctx, dummyImage);
		free(dummyImage);
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
	

	if (libHidDisplay_Reset(&hid_ctx))
		printf("Reset signal sent\n");
	else
		printf("Reset signal not sent\n");
		
	libHidDisplay_Close(&hid_ctx);
	return 0;

}
