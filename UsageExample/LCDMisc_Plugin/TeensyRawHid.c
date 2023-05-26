
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <inttypes.h>
#include <process.h>
#include "plugin.h"
#include "scriptval.h"
#include "../../desktop/libTeensyRawHid.h"
#include "../../desktop/mmx_rgb.h"




#define DWIDTH			TEENSYRAWHID_WIDTH
#define DHEIGHT			TEENSYRAWHID_HEIGHT
#define DTITLE			"TeensyRawHid"
#define EXPORT			__declspec(dllexport)

static LcdCallbacks *LMC = NULL;
static int isShuttingdown = 0;
static int initialized = 0;
static LcdInfo windows;
static teensyRawHid_t ctx;




// functions razersb_xxxx are for backwards compatibility

EXPORT CALLBACK void razersb_SelectDisplayPad ()
{
}

EXPORT CALLBACK intptr_t razersb_GetSelectedDisplay ()
{
	return 2;
}

EXPORT CALLBACK void razersb_SelectDisplayKeys ()
{
}

EXPORT CALLBACK void razersb_ClearDisplayPad ()
{
}

EXPORT CALLBACK void razersb_ClearDisplayKeys ()
{

}

static void frame32ToBuffer16 (uint8_t *pixels, uint8_t *buffer, const int width, const int height)
{
#if 1
	rgb_32_to_16_mmx(pixels, width, height, buffer, width*4, DWIDTH*2);

#else
	
	uint16_t * restrict out = (uint16_t*)buffer;
	TRGBA * restrict in = (TRGBA*)pixels;
	TRGBA * restrict p;
	const int cpitch = DWIDTH*2;
	const int fpitch = width;
	
	for (int y = 0; y < height; y++){
		p = in+(y*fpitch);
		for (int x = 0; x < width; x++)
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;

		out = (uint16_t*)((uint8_t*)out+cpitch);
	}

#endif
}

int openDisplayWait (const int timems)
{
	const int delay = 20;
	const int loops = timems/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libTeensyRawHid_OpenDisplay(&ctx.teensyRawHid, 0)){
			return 1;
		}
	}
	return 0;
}

int teensyRawHid_init ()
{
	if (!libTeensyRawHid_OpenDisplay(&ctx.teensyRawHid, 0)){
		if (!openDisplayWait(500))
			return 0;
	}

	if (!ctx.teensyRawHid.frame){
		ctx.teensyRawHid.frame = calloc(2, DWIDTH*DHEIGHT);
		ctx.teensyRawHid.frameAlt = calloc(2, DWIDTH*DHEIGHT);
	}
		
	if (ctx.teensyRawHid.frame){
		return 1;
	}

	return 0;
}

void teensyRawHid_shutdown ()
{
	isShuttingdown = 1;
	initialized = 0;

	libTeensyRawHid_CloseDisplay(&ctx.teensyRawHid);

	if (ctx.teensyRawHid.frame){
		free(ctx.teensyRawHid.frame);
		free(ctx.teensyRawHid.frameAlt);
		ctx.teensyRawHid.frame = NULL;
	}
}

void teensyRawHid_Update (teensyRawHidcxt_t *teensyhid, uint8_t *pixels, const int width, const int height)
{
	frame32ToBuffer16(pixels, teensyhid->frame, width, height);

	header_t idesc;
	idesc.x1 = DWIDTH - width;
	idesc.y1 = DHEIGHT - height;
	idesc.x2 = DWIDTH-1;
	idesc.y2 = DHEIGHT-1;

	libTeensyRawHid_WriteImage(teensyhid, &idesc, ctx.teensyRawHid.frame);
}

CALLBACK void Update (LcdInfo *info, BitmapInfo *bmp)
{
	if (!isShuttingdown && initialized){
		if (ctx.teensyRawHid.frame == NULL)
			ctx.teensyRawHid.frame = calloc(2, DWIDTH*DHEIGHT);

		teensyRawHid_Update(&ctx.teensyRawHid, bmp->bitmap, bmp->width, bmp->height);
	}
}

CALLBACK void Destroy (LcdInfo *info)
{
	isShuttingdown = 1;
	initialized = 0;
	LMC = NULL;
}

void initWindow (LcdInfo *win, const char *name)
{	
	memset(win, 0, sizeof(LcdInfo));
	win->bpp = 32;
	win->refreshRate = 30;
	win->id = name;
	win->width = DWIDTH;
	win->height = DHEIGHT;
	win->Update = Update;
	win->Destroy = Destroy;
	
	initialized = teensyRawHid_init();
}

EXPORT CALLBACK int lcdInit (LcdCallbacks *LCDCallbacks)
{
	LMC = LCDCallbacks;
	initWindow(&windows, DTITLE);
	return 1;
}

EXPORT CALLBACK void lcdUninit ()
{
	initialized = 0;
	teensyRawHid_shutdown();
}

EXPORT CALLBACK LcdInfo * lcdEnum ()
{
	if (initialized == 1){
		initialized = 2;
		return &windows;
	}
	return NULL;
}

EXPORT CALLBACK void Init (AppInfo *in, DllInfo *out)
{
	if (in->size < sizeof(AppInfo) || in->maxDllVersion < 1 || out->size < sizeof(DllInfo))
		return;
	
	out->free = free;
	out->dllVersion = 1;
}

EXPORT BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, void *lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInstance);
	}else if (fdwReason == DLL_PROCESS_DETACH) {
		//lcdUninit();
		(void)lpvReserved;
	}
    return 1;
}
