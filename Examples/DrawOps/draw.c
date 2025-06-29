
#include <stdio.h>
#include <conio.h>
#include <wchar.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libHidDisplay.h>
#include "drawop.h"
#include "image_256x142_16.h"


#define DRAWOPS_REFID	1234
#define DRAW_PAL_REFID	5432
#define DRAW_PAL_SIZE	32



enum colIdx {
	PAL_BLACK = 0,
	PAL_WHITE = 1,
	PAL_RED,
	PAL_GREEN,
	PAL_BLUE,
	PAL_CYAN,
	PAL_YELLOW,
	PAL_BROWN,
	PAL_MAGENTA
	// etc..
};




static int DWIDTH = 960;
static int DHEIGHT = 540;
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

ops_clut_t *createPalette (const uint16_t tableLength, const uint16_t refId)
{
	ops_clut_t *clut = pal_create(tableLength, refId);
	if (clut){
		pal_add(clut, PAL_BLACK,  COLOUR_BLACK);
		pal_add(clut, PAL_WHITE,  COLOUR_WHITE);
		pal_add(clut, PAL_RED,    COLOUR_RED);
		pal_add(clut, PAL_GREEN,  COLOUR_GREEN);
		pal_add(clut, PAL_BLUE,   COLOUR_BLUE);
		pal_add(clut, PAL_CYAN,   COLOUR_CYAN);
		pal_add(clut, PAL_YELLOW, COLOUR_YELLOW);
		pal_add(clut, PAL_BROWN,  COLOUR_BROWN);
		pal_add(clut, PAL_MAGENTA,COLOUR_MAGENTA);
	}
	return clut;
}

int createDrawOps (draw_ops_t *ops)
{
	void *pixels = (void*)imageMarbles;
	int img_size = sizeof(imageMarbles);
	uint16_t img_w = imageMarbles_WIDTH;
	uint16_t img_h = imageMarbles_HEIGHT;
	
	drawop_Image(ops, pixels, img_size, img_w, img_h, DRAW_IMG_CENTERED, 0.0f, 0.0f);
	drawop_Image(ops, pixels, img_size, img_w, img_h, 0, 0.5f, 0.7f);

	drawop_Line(ops, 0.001f, 0.002f, 0.513f, 0.54f, 0, PAL_RED);
	drawop_Line(ops, 0.351f, 0.052f, 0.953f, 0.84f, DRAW_DOTTED, PAL_MAGENTA);
	drawop_Point(ops, 0.309f, 0.410f, PAL_RED);
	drawop_Circle(ops, 0.809f, 0.500f, 0.1f, DRAW_FILLED, PAL_GREEN);
	drawop_Circle(ops, 0.809f, 0.550f, 0.1f, DRAW_FILLED, PAL_WHITE);
	drawop_Ellipse(ops, 0.105f, 0.206f, 0.07f, 0.08f, PAL_BLUE);
	drawop_Rectangle(ops, 0.105f, 0.116f, 0.807f, 0.308f, DRAW_FILLED, PAL_MAGENTA);
	drawop_Rectangle(ops, 0.005f, 0.016f, 0.857f, 0.358f, 0, PAL_BROWN);

	pointFlt_t pts[] = {{0.0f, 0.0f}, {0.1f, 0.0f}, {0.1f, 0.25f}, {0.5f, 0.5f}, {0.5f, 0.4f}, {0.9f, 0.9f}};
	const int ptotal = sizeof(pts)/sizeof(pointFlt_t);
	// DRAW_PLY_CLOSE - connect last with first
	drawop_Poly(ops, pts, ptotal, DRAW_PLY_CLOSE, PAL_BLUE);
	

	const char *str1 = "Hello, World";
	int slen = strlen(str1);
	uint8_t flags = 0;
	uint32_t fontId = 34;		// fontId is relative to whichever primative library is compiled in
	drawop_String(ops, (void*)str1, slen, flags, fontId, PAL_MAGENTA, 0.35, 0.35);

	drawop_Arc(ops, 0.105f, 0.106f, 0.07f, 0.08f, 0.09f, 0.01f, PAL_BLACK);
	drawop_Triangle(ops, 0.05f, 0.06f, 0.07f, 0.08f, 0.09f, 0.01f, 0, PAL_CYAN);
	drawop_Copy(ops, 0.00f, 0.00f, 0.7f, 0.2f, 0.101f, 0.301f);
	drawop_Triangle(ops, 0.05f, 0.96f, 0.17f, 0.58f, 0.09f, 0.31f, DRAW_FILLED, PAL_YELLOW);

	float x = 0.5f;
	float y = 0.5f;
	float w = 0.02f;
	float h = 0.1f;
	drawop_Rectangle(ops, x, y, x+w, y+h, DRAW_FILLED, PAL_BROWN);

	const char *str2 = "This is worlds apart";
	slen = strlen(str2);
	flags = 0;
	fontId = 7;
	drawop_String(ops, (void*)str2, slen, flags, fontId, PAL_MAGENTA, 0.01, 0.11);
	
	return drawops_total(ops);
}

int main (int argc, char **cargv)
{

	printf("\n");
	if (!display_init()){
		if (libHidDisplay_GetDisplayTotal())
			printf("Display in use\n");
		else
			printf("Display not found\n");
		return 0;
	}

	// create and build a Palette
	ops_clut_t *clut = createPalette(DRAW_PAL_SIZE, DRAW_PAL_REFID);
	if (clut){
		// activate this palette
		pal_setActive(clut);

		// send palette to device
		pal_commit(clut, 0, &hid_ctx);

		// create a draw handle with initial space for 32 instructions, will auto grow
		draw_ops_t *ops = drawops_create(32, DRAWOPS_REFID);
		if (ops){

			// generate a few primative instructions
			createDrawOps(ops);
			
			// send to device
			// store for later execution - HIDD_DRAW_STORE
			// don't auto execute upon store - HIDD_DRAW_EXECUTE
			// overwrite an existing drawOps instrtuction set upon a refId clash - HIDD_DRAW_OVERWRITE
			drawops_commit(ops, HIDD_DRAW_STORE|HIDD_DRAW_OVERWRITE, &hid_ctx);
			
			// execute 3 times, because
			drawops_execute(&hid_ctx, DRAWOPS_REFID);
			drawops_execute(&hid_ctx, DRAWOPS_REFID);
			drawops_execute(&hid_ctx, DRAWOPS_REFID);
		
			drawops_free(ops);
		}
		pal_free(clut);
	}

	libHidDisplay_Close(&hid_ctx);
	return 0;

}
