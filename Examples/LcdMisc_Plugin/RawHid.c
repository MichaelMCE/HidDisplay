
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <inttypes.h>
#include <process.h>
#include <libHidDisplay.h>

#include "plugin.h"
#include "scriptval.h"



#define EXPORT			__declspec(dllexport)
#define TILES_WIDTH		(24)
#define TILES_HEIGHT	(20)


typedef struct _rawhid_t{
	teensyRawHidcxt_t ctx;
	LcdCallbacks *LMC;
	LcdInfo windows;
	config_t config;
	int isShuttingdown;
	int initialized;
	
	HANDLE hTouchThread;
	uint32_t frameSize;
}rawHidDisplay_t;

static rawHidDisplay_t rawhid;




static void frame32ToBuffer16 (uint8_t *pixels, uint8_t *buffer, const int width, const int height, const int dpitch)
{
	uint16_t *out = (uint16_t*)buffer;
	TRGBA *in = (TRGBA*)pixels;
	TRGBA *p;
	const int cpitch = dpitch;
	const int fpitch = width;
	
	for (int y = 0; y < height; y++){
		p = in+(y*fpitch);
		for (int x = 0; x < width; x++)
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;
		out = (uint16_t*)((uint8_t*)out+cpitch);
	}
}

static void frame32ToBuffer16Clamp (uint8_t *pixels, uint8_t *buffer, const int width, const int height, const int dpitch)
{
	uint16_t *out = (uint16_t*)buffer;
	TRGBA *in = (TRGBA*)pixels;
	TRGBA *p;
	const int cpitch = dpitch;
	const int fpitch = width;
	const uint8_t max = rawhid.ctx.rgbClamp;
	
	for (int y = 0; y < height; y++){
		p = in+(y*fpitch);
		for (int x = 0; x < width; x++){
		
			if (p[x].r > max) p[x].r = max;
			if (p[x].g > max) p[x].g = max;
			if (p[x].b > max) p[x].b = max;
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;
		}
		out = (uint16_t*)((uint8_t*)out+cpitch);
	}
}

static inline int tileToX (const int tileX)
{
	return tileX * (rawhid.ctx.width/rawhid.config.tiles.across);
}

static inline int tileToY (const int tileY)
{
	return tileY * (rawhid.ctx.height/rawhid.config.tiles.down);
}

void teensyRawHid_Update_AsTiles (rawHidDisplay_t *rawhid, uint8_t *pixels, const int width, const int height)
{
	int dst_x = 0;
	int dst_y = 0;
	int x = dst_x;
	int y = dst_y;
	const int tTiles = rawhid->config.tiles.height*rawhid->config.tiles.width;
	int total = tTiles;
	const int srcStride = width * sizeof(uint16_t);
	
	int pitch = rawhid->config.tiles.width * sizeof(uint16_t);		// 2 = 16bpp
	uint32_t size = total * pitch * rawhid->config.tiles.height;
	
	uint8_t *mem_tile = malloc(size);
	uint8_t *tile = mem_tile;
	if (!tile) return;

	while (total-- > 0){
		int x1 = tileToX(x);
		int y1 = tileToY(y);

		uint8_t *addr = (uint8_t*)(pixels + (y1*width*sizeof(uint16_t))) + (x1*sizeof(uint16_t));

		for (int r = y1; r < y1+rawhid->config.tiles.height; r++){
			//uint8_t *addr = (uint8_t*)(pixels + (r*width*2));
			//addr += (x1*sizeof(uint16_t));

			memcpy(tile, addr, pitch);
			tile += pitch;
			addr += srcStride;
		}
		
		x++;
		if (x > rawhid->config.tiles.window.x2){
			x = rawhid->config.tiles.window.x1;
			y++;
			if (y > rawhid->config.tiles.window.y2){
				y = rawhid->config.tiles.window.y1;
				x = rawhid->config.tiles.window.x1;
			}
		}
	}
	
	libHidDisplay_WriteTiles(&rawhid->ctx, mem_tile, size, dst_x, dst_y, tTiles);
	free(mem_tile);
}

int teensyRawHid_Update (teensyRawHidcxt_t *ctx, uint8_t *pixels, const int width, const int height)
{
	// in: pixels - 32bpp. alpha is unused
	// out: ctx->frame - 16bpp
	frame32ToBuffer16(pixels, ctx->frame, width, height, ctx->pitch);

	// update as a single image
	//return libHidDisplay_WriteImageAsync(ctx, ctx->frame);
	return libHidDisplay_WriteImage(ctx, ctx->frame);
	
	// update as two halves
	//libHidDisplay_WriteArea(ctx, ctx->frame, 0, 0, ctx->width-1, (ctx->height>>1)-1);
	//uint8_t *halfpixels = ctx->frame + ((ctx->height>>1) * ctx->width * 2);
	//libHidDisplay_WriteArea(ctx, halfpixels, 0, (ctx->height>>1), ctx->width-1, ctx->height-1);
	
	// update as multiple tiles
	//teensyRawHid_Update_AsTiles(&rawhid, ctx->frame, ctx->width, ctx->height);
	
	
	/*const int strips = 2;
	const int stripHeight = ctx->height/strips;
	
	for (int i = 0; i < strips; i++){
		uint8_t *pix = (uint8_t*)ctx->frame + ((i*stripHeight) * ctx->width * 2);
		libHidDisplay_WriteArea(ctx, pix, 0, i*stripHeight, ctx->width-1, (i*stripHeight)+stripHeight-1);
	}*/
	return 1;
}

CALLBACK void lcdmisc_destroy (LcdInfo *info)
{
	rawhid.isShuttingdown = 1;
	rawhid.initialized = 0;
	rawhid.LMC = NULL;
}

static int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libHidDisplay_OpenDisplay(&rawhid.ctx, 0))
			return 1;
	}
	return 0;
}

void setTilesWindow (config_t *config, const int tileWidth, const int tileHeight, const int dwidth, const int dheight)
{
	config->tiles.width = tileWidth;
	config->tiles.height = tileHeight;
	config->tiles.across = dwidth/config->tiles.width;
	config->tiles.down = dheight/config->tiles.height;
	config->tiles.window.x1 = 0;
	config->tiles.window.x2 = config->tiles.across-1;
	config->tiles.window.y1 = 0;
	config->tiles.window.y2 = config->tiles.down-1;
}

void setDisplayMetrics (rawHidDisplay_t *rawhid, const int width, const int height, const int stride, const uint8_t rgbClamp)
{
	rawhid->ctx.width = width;
	rawhid->ctx.height = height;
	rawhid->ctx.pitch = stride;
	rawhid->ctx.rgbClamp = rgbClamp;
}

unsigned int __stdcall touchListenerThread (void *ptr)
{
	teensyRawHidcxt_t *ctx = &rawhid.ctx;
	
	char eventArg[128];
	touch_t touch;
	int touchState = 2;
		
	while (!rawhid.isShuttingdown && rawhid.initialized){
		if (libHidDisplay_GetReportWait(ctx, &touch)){
			if (!rawhid.initialized) break;

			if (touch.flags == RAWHID_OP_TOUCH_POINTS){
				__mingw_snprintf(eventArg, 64, "%i,%i,%i,%i", touch.x, touch.y, (int)touch.time, (int)GetTickCount());
				if (touchState == 2)									// previous  event was release, so this is a down
					rawhid.LMC->TriggerEvent(rawhid.LMC->id, "touchDown", eventArg);
				else if (touchState == 1)								// previous event was down, so this is a move
					rawhid.LMC->TriggerEvent(rawhid.LMC->id, "touchMove", eventArg);
					
				touchState = 1;
				
			}else if (touch.flags == RAWHID_OP_TOUCH_RELEASE){
				snprintf(eventArg, sizeof(eventArg), "%i,%i,%i,%i", touch.x, touch.y, (int)touch.time, (int)GetTickCount());
				rawhid.LMC->TriggerEvent(rawhid.LMC->id, "touchUp", eventArg);
				touchState = 2;
			}
		}else{
			Sleep(5);
		}
	};

	_endthreadex(1);
	return 1;
}

unsigned int touchStartListenerThread (rawHidDisplay_t *ctx, const int threadFlags)
{
	unsigned int tid = 0;
	ctx->hTouchThread = (HANDLE)_beginthreadex(NULL, 0, touchListenerThread, ctx, threadFlags, &tid);
	return tid;
}

void touchStopistenerThreadWait (rawHidDisplay_t *ctx)
{
	libHidDisplay_TouchReportEnable(&ctx->ctx, 0, 0);
	ResumeThread(ctx->hTouchThread);
	WaitForSingleObject(ctx->hTouchThread, INFINITE);
}

void touch_init (rawHidDisplay_t *ctx)
{
	libHidDisplay_TouchReportEnable(&ctx->ctx, 1, TOUCH_DIR_DEFAULT);
	touchStartListenerThread(ctx, HIGH_PRIORITY_CLASS);
}

void configTiles (rawHidDisplay_t *ctx)
{
	// using tile rendering is not necessary but an option, configure it anyway
	setTilesWindow(&ctx->config, TILES_WIDTH, TILES_HEIGHT, ctx->ctx.width, ctx->ctx.height);
	
	rawhid_header_t desc;
	memset(&desc, 0, sizeof(desc));
	memcpy(&desc.u.config, &ctx->config, sizeof(ctx->config));

	libHidDisplay_SetTileConfig(&rawhid.ctx, &desc);
}

int teensyRawHid_init ()
{
	if (!libHidDisplay_OpenDisplay(&rawhid.ctx, 0)){
		if (!openDisplayWait(750))
			return 0;
	}

	rawhid_header_t desc;
	if (libHidDisplay_GetConfig(&rawhid.ctx, &desc)){
		setDisplayMetrics(&rawhid, desc.u.cfg.width, desc.u.cfg.height, desc.u.cfg.pitch, desc.u.cfg.rgbMax);
		configTiles(&rawhid);

		if (!rawhid.ctx.frame){
			rawhid.frameSize = rawhid.ctx.height * rawhid.ctx.pitch;
			rawhid.ctx.frame = calloc(1, rawhid.frameSize);
		}
			
		touch_init(&rawhid);
	}

	return (rawhid.ctx.frame != NULL);
}

int initDisplay ()
{
	rawhid.initialized = teensyRawHid_init();
	return rawhid.initialized;
}

CALLBACK void lcdmisc_update (LcdInfo *info, BitmapInfo *bmp);

void initWindow (LcdInfo *win)
{	
	if (win->bpp != 32){
		memset(win, 0, sizeof(LcdInfo));
		win->bpp = 32;
		win->refreshRate = 30;		// doesn't do anything, but still
		win->name = "hiddisplay";	// name must be const (bug in lcdmisc)
	}

	win->width = rawhid.ctx.width;
	win->height = rawhid.ctx.height;
	win->Update = lcdmisc_update;
	win->Destroy = lcdmisc_destroy;
}

CALLBACK void lcdmisc_update (LcdInfo *info, BitmapInfo *bmp)
{
	static int sendFirstTwice = 0;

	if (!rawhid.isShuttingdown && rawhid.initialized){
		int success = 0;
		if (rawhid.ctx.usb_handle)
			success = teensyRawHid_Update(&rawhid.ctx, bmp->bitmap, bmp->width, bmp->height);
#if 1
		(void)success;
	}
#else
		if (!success){
			if (rawhid.ctx.usb_handle){
				if (sendFirstTwice == 0xFF){
					sendFirstTwice = 0;
					usb_release_interface(rawhid.ctx.usb_handle, rawhid.ctx.interface);
    				usb_reset(rawhid.ctx.usb_handle);
    				Sleep(10);

					rawhid.initialized = 0;
					touchStopistenerThreadWait(&rawhid);
				}
			}
		}
	}
	if (!rawhid.initialized){
		int ret = libHidDisplay_Open(&rawhid.ctx, rawhid.ctx.interface, 0);
		if (ret){
			usb_resetep(rawhid.ctx.usb_handle, LIBUSB_ENDPOINT_READ);
			usb_resetep(rawhid.ctx.usb_handle, LIBUSB_ENDPOINT_WRITE);
			Sleep(10);
					
			rawhid_header_t desc;
			int ret = libHidDisplay_GetConfig(&rawhid.ctx, &desc);
			if (ret){
				setDisplayMetrics(&rawhid, desc.u.cfg.width, desc.u.cfg.height, desc.u.cfg.pitch, desc.u.cfg.rgbMax);
				configTiles(&rawhid);

				if (rawhid.ctx.frame){
					if (rawhid.frameSize < rawhid.ctx.height * rawhid.ctx.pitch){
						rawhid.frameSize = rawhid.ctx.height * rawhid.ctx.pitch;
						free(rawhid.ctx.frame);
						rawhid.ctx.frame = calloc(1, rawhid.frameSize);
					}
				}
					
				initWindow(&rawhid.windows);
				rawhid.initialized = 2;
				touch_init(&rawhid);
			}
			sendFirstTwice = 0;
		}
	}
#endif

	// first frame is always corrupted. send again to fix
	if (!sendFirstTwice){
		sendFirstTwice = 0xFF;
		teensyRawHid_Update(&rawhid.ctx, bmp->bitmap, bmp->width, bmp->height);
		teensyRawHid_Update(&rawhid.ctx, bmp->bitmap, bmp->width, bmp->height);
	}
}

EXPORT CALLBACK int lcdInit (LcdCallbacks *LCDCallbacks)
{
	rawhid.LMC = LCDCallbacks;
	
	if (initDisplay()){
		initWindow(&rawhid.windows);
		return 1;
	}
	return 0;
}

static void teensyRawHid_shutdown ()
{
	rawhid.isShuttingdown = 1;
	rawhid.initialized = 0;

	touchStopistenerThreadWait(&rawhid);
	libHidDisplay_CloseDisplay(&rawhid.ctx);

	if (rawhid.ctx.frame){
		free(rawhid.ctx.frame);
		rawhid.ctx.frame = NULL;
	}
}

EXPORT CALLBACK void lcdUninit ()
{
	rawhid.initialized = 0;
	teensyRawHid_shutdown();
}

EXPORT CALLBACK LcdInfo *lcdEnum ()
{
	if (rawhid.initialized == 1){
		rawhid.initialized = 2;
		return &rawhid.windows;
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


#if 0		// no longer required
// functions razersb_xxxx are for backwards compatibility

static int disop = 2;

EXPORT CALLBACK void razersb_SelectDisplayPad ()
{
	disop = 2;
}

EXPORT CALLBACK void razersb_SelectDisplayKeys ()
{
	disop = 1;
}

EXPORT CALLBACK intptr_t razersb_GetSelectedDisplay ()
{
	return disop;
}

EXPORT CALLBACK void razersb_ClearDisplayPad ()
{
}

EXPORT CALLBACK void razersb_ClearDisplayKeys ()
{
}
#endif
