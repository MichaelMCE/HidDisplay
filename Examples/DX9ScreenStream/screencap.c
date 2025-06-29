

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <math.h>
#include <d3d9.h>
#include <libHidDisplay.h>



typedef struct _d3d {
	IDirect3D9 *d3d;
	D3DDISPLAYMODE mode;
	D3DPRESENT_PARAMETERS d3dpp;
	IDirect3DDevice9 *device;
	IDirect3DSurface9 *surface;
	D3DLOCKED_RECT rc;

	HWND hwnd;
	uint8_t *out_buffer;
}d3d9ctx_t;


typedef struct _gdi {
	HWND hwnd;
	HDC hDCSrc;
	HDC hDCMemory;
	HBITMAP hBmp;
	HBITMAP hBmpPrev;

	int desktopWidth;
	int desktopHeight; 

	POINT pt;
	RECT rc;
	int WidthSrc;
	int HeightSrc;
	void *image;
	uint8_t *out_buffer;
}gdictx_t;


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

	if (!DWIDTH || !DHEIGHT){
		libHidDisplay_Close(&hid_ctx);
		return 0;
	}

	printf("Found device: %s\nWxH:%ix%i\n", desc.u.cfg.string, DWIDTH, DHEIGHT);
	return 1;
}

void frame32To16 (RECT *rect, uint8_t *in_pixels, uint8_t *out_pixels, const int in_pitch, const int out_pitch)
{

	uint16_t *out = (uint16_t*)out_pixels;
	TRGBA *in = (TRGBA*)in_pixels;
	TRGBA *p;
		
	for (int y = rect->top; y < rect->bottom; y++){
		p = in+(y*in_pitch);
		int outx = 0;
		
		for (int x = rect->left; x < rect->right; x++, outx++)
			*(out+outx) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;

		out = (uint16_t*)((uint8_t*)out+out_pitch);
	}
}

void frame32To16Fit (RECT *rect, uint8_t *in_pixels, uint8_t *out_pixels, const int in_pitch, const int out_pitch)
{
	uint16_t *out = (uint16_t*)out_pixels;
	TRGBA *in = (TRGBA*)in_pixels;
	TRGBA *p;
	
	float src_x = rect->left;
	float src_y = rect->top;
	float src_width = (rect->right - rect->left);
	float src_height = (rect->bottom - rect->top);
	const float scalex = (float)DWIDTH/ src_width;
	const float scaley = (float)DHEIGHT / src_height;
	int x2, y2;

	for (int y = 0; y < DHEIGHT; y++){
		y2 = (src_y + ((float)y/scaley));
		p = in + (y2 * in_pitch);
		
		for (int x = 0; x < DWIDTH; x++){
			x2 = (src_x + ((float)x/scalex));
			*(out+x) = (p[x2].r&0xF8)>>3|(p[x2].g&0xFC)<<3|(p[x2].b&0xF8)<<8;
		}
		out = (uint16_t*)((uint8_t*)out+out_pitch);
	}
}


void d3d9_close (d3d9ctx_t *ctx)
{
	if (ctx->out_buffer) free(ctx->out_buffer);
	if (ctx->surface) IDirect3DSurface9_Release(ctx->surface);
	if (ctx->device) IDirect3DDevice9_Release(ctx->device);
	if (ctx->d3d) IDirect3D9_Release(ctx->d3d);
}

int d3d9_init (d3d9ctx_t *ctx, const int adapter, POINT *pt_cursor, HWND hwnd)
{
	memset(ctx, 0, sizeof(*ctx));
	
	ctx->hwnd = hwnd;
	ctx->d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!ctx->d3d) return 0;
	
	if (IDirect3D9_GetAdapterDisplayMode(ctx->d3d, adapter, &ctx->mode) != S_OK){
		d3d9_close(ctx);
		return 0;
	}
	
	D3DPRESENT_PARAMETERS *d3dpp = &ctx->d3dpp;
	memset(d3dpp, 0, sizeof(*d3dpp));
	
	d3dpp->Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp->Windowed = TRUE;
	d3dpp->MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp->SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp->BackBufferCount = 1;
	d3dpp->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	//d3dpp->EnableAutoDepthStencil = TRUE;
	//d3dpp->AutoDepthStencilFormat = D3DFMT_D16;

	if (IDirect3D9_CreateDevice(ctx->d3d, adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, d3dpp, &ctx->device) != S_OK){
		d3d9_close(ctx);
		return 0;
	}

	if (IDirect3DDevice9_CreateOffscreenPlainSurface(ctx->device, ctx->mode.Width, ctx->mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &ctx->surface, NULL) != S_OK){
		d3d9_close(ctx);
		return 0;
	}

	RECT rect;
	int src_pitch = DWIDTH*2;

	if (IDirect3DSurface9_LockRect(ctx->surface, &ctx->rc, &rect, D3DLOCK_NOSYSLOCK|D3DLOCK_NOOVERWRITE|D3DLOCK_DONOTWAIT|D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY) == S_OK){
		src_pitch = ctx->rc.Pitch;
		IDirect3DSurface9_UnlockRect(ctx->surface);
	}

	ctx->out_buffer = malloc(src_pitch * DHEIGHT);
	if (!ctx->out_buffer){
		d3d9_close(ctx);
		return 0;
	}
		
	return 1;
}

int d3d9_loop (d3d9ctx_t *ctx, const int bestFit, const int followCursor, const int maxFPS)
{

	POINT pt;
	RECT rect;
	RECT crect;
	int ct = 0;
	int32_t t0 = GetTickCount();
  	
	while (!kbhit()){
		if (!followCursor){
			GetClientRect(ctx->hwnd, &crect);
    		MapWindowPoints(HWND_DESKTOP, ctx->hwnd, (LPPOINT)&crect, 1);

			rect.left = abs(crect.left);
			rect.right = rect.left + crect.right;
			rect.top = abs(crect.top);
			rect.bottom = rect.top + crect.bottom;

			if (rect.left < 0) rect.left = 0;
			if (rect.top < 0) rect.top = 0;
			if (rect.right > ctx->mode.Width-1) rect.right = ctx->mode.Width-1;
			if (rect.bottom > ctx->mode.Height-1) rect.bottom = ctx->mode.Height-1;
#if 0
			if ((rect.bottom - rect.top)+1 > DHEIGHT) rect.bottom -= ((rect.bottom - rect.top) - DHEIGHT);
			if ((rect.right - rect.left)+1 > DWIDTH) rect.right -= ((rect.right - rect.left) - DWIDTH);
#endif	  
		}else if (bestFit){
			GetCursorPos(&pt);
			ctx->hwnd = WindowFromPoint(pt);

			GetClientRect(ctx->hwnd, &crect);
			MapWindowPoints(HWND_DESKTOP, ctx->hwnd, (LPPOINT)&crect, 1);

			rect.left = abs(crect.left);
			rect.right = rect.left + crect.right;
			rect.top = abs(crect.top);
			rect.bottom = rect.top + crect.bottom;

			if (rect.left < 0) rect.left = 0;
			if (rect.top < 0) rect.top = 0;
			if (rect.right > ctx->mode.Width-1) rect.right = ctx->mode.Width-1;
			if (rect.bottom > ctx->mode.Height-1) rect.bottom = ctx->mode.Height-1;

		}else{		// follow cursor
			GetCursorPos(&pt);
			rect.left = pt.x - (DWIDTH/2);
			if (rect.left < 0) rect.left = 0;
			rect.right = rect.left + DWIDTH;
			if (rect.right > ctx->mode.Width-1){
				rect.right = ctx->mode.Width-1;
				rect.left = rect.right - DWIDTH;
			}
		
			rect.top = pt.y - (DHEIGHT/2);
			if (rect.top < 0) rect.top = 0;
			rect.bottom = rect.top + DHEIGHT;
			if (rect.bottom > ctx->mode.Height-1){
				rect.bottom = ctx->mode.Height-1;
				rect.top = rect.bottom - DHEIGHT;
			}
		}

		if (IDirect3DDevice9_GetFrontBufferData(ctx->device, 0, ctx->surface) == S_OK){
			if (IDirect3DSurface9_LockRect(ctx->surface, &ctx->rc, NULL, D3DLOCK_DONOTWAIT|D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY) == S_OK){
				if (!bestFit)
					frame32To16(&rect, ctx->rc.pBits, ctx->out_buffer, ctx->mode.Width, DWIDTH*2);
				else
					frame32To16Fit(&rect, ctx->rc.pBits, ctx->out_buffer, ctx->mode.Width, DWIDTH*2);
				IDirect3DSurface9_UnlockRect(ctx->surface);
			}
	  		libHidDisplay_WriteImageAsync(&hid_ctx, ctx->out_buffer);
	  		
	  		Sleep((1.0f/(float)maxFPS)*1000);
		}else{
			Sleep(20);
		}
		ct++;
	}
	int32_t t1 = GetTickCount();
	printf("FPS: %.1f\n", (float)ct/(float)(t1-t0)*1000.0f);

	return ct;
}

int gdi_init (gdictx_t *ctx, const int desktop, POINT *pt_cursor, HWND hwnd)
{
	memset(ctx, 0, sizeof(*ctx));

	ctx->pt = *pt_cursor;
	ctx->hwnd = hwnd;

    //Get Window device context
	ctx->hDCSrc = GetWindowDC(ctx->hwnd);
	GetWindowRect(ctx->hwnd, &ctx->rc);
   	ctx->WidthSrc = ctx->rc.right - ctx->rc.left;
	ctx->HeightSrc = ctx->rc.bottom - ctx->rc.top;
	ctx->desktopWidth = GetDeviceCaps(ctx->hDCSrc, HORZRES);
	ctx->desktopHeight = GetDeviceCaps(ctx->hDCSrc, VERTRES);
	ctx->WidthSrc = max(ctx->WidthSrc, ctx->desktopWidth);
	ctx->HeightSrc = max(ctx->HeightSrc, ctx->desktopHeight);

	if (!ctx->out_buffer)
		ctx->out_buffer = calloc(sizeof(uint32_t), ctx->desktopHeight * ctx->desktopWidth);
	if (ctx->out_buffer == NULL) return 0;
	ctx->image = calloc(sizeof(uint32_t), ctx->HeightSrc * ctx->WidthSrc);
	if (ctx->image == NULL){
		free(ctx->out_buffer);
		return 0;
	}

    //create a memory device context
   	ctx->hDCMemory = CreateCompatibleDC(ctx->hDCSrc);

    //create a bitmap compatible with window hdc
   	ctx->hBmp = CreateCompatibleBitmap(ctx->hDCSrc, ctx->WidthSrc, ctx->HeightSrc);

	return 1;
}

int gdi_getBitmap (gdictx_t *ctx, const int bestFit, const int followCursor, const int usePrintWinAPI)
{

    //copy newly created bitmap into memory device context
    if (!ctx->hBmpPrev)
   		ctx->hBmpPrev = SelectObject(ctx->hDCMemory, ctx->hBmp);
   	else
   		SelectObject(ctx->hDCMemory, ctx->hBmp);
    
     // copy window hdc to memory hdc
	if (!usePrintWinAPI)
    	BitBlt(ctx->hDCMemory, 0, 0, ctx->WidthSrc, ctx->HeightSrc, ctx->hDCSrc, 0, 0, SRCCOPY|CAPTUREBLT);
    else
		PrintWindow(ctx->hwnd, ctx->hDCMemory, PW_CLIENTONLY);
	
    GetBitmapBits(ctx->hBmp, ctx->WidthSrc * ctx->HeightSrc * 4, ctx->image);

	int x = 0;
	int y = 0;
	int x2 = ctx->WidthSrc-1;
	int y2 = ctx->HeightSrc-1;


	if (followCursor){
		POINT *pt = &ctx->pt;

		pt->x -= ctx->rc.left;
		pt->y -= ctx->rc.top;

		if (x > ctx->WidthSrc - DWIDTH) x = ctx->WidthSrc - DWIDTH;
		if (x < 0) x = 0;
		x2 = x+DWIDTH-1;

		if (y > ctx->HeightSrc - DHEIGHT) y = ctx->HeightSrc - DHEIGHT;
		if (y < 0) y = 0;
		y2 = y+DHEIGHT-1;
		
		if (x2 > ctx->WidthSrc-1) x2 = ctx->WidthSrc-1;
		if (y2 > ctx->HeightSrc-1) y2 = ctx->HeightSrc-1;
	}

	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x2;
	rect.bottom = y2;

	RECT crect;
	GetClientRect(ctx->hwnd, &crect);
	MapWindowPoints(HWND_DESKTOP, ctx->hwnd, (LPPOINT)&crect, 1);

	rect.left += ((abs(crect.left) - ctx->rc.left));
	rect.top += ((abs(crect.top) - ctx->rc.top));
	rect.right = ((abs(crect.left) + crect.right) - ctx->rc.left) - 1;
	rect.bottom = ((abs(crect.top) + crect.bottom) - ctx->rc.top) - 1;

	//printf("f: %i %i %i %i, %i %i\n", rect.left, rect.right, rect.top, rect.bottom, ctx->WidthSrc, ctx->HeightSrc);
	
	if (!bestFit)
		frame32To16(&rect, ctx->image, ctx->out_buffer, ctx->WidthSrc, DWIDTH*2);
	else
		frame32To16Fit(&rect, ctx->image, ctx->out_buffer, ctx->WidthSrc, DWIDTH*2);

	ctx->hBmpPrev = NULL;
	return 1;
}

void gdi_close (gdictx_t *ctx)
{
    if (ctx->hBmp)
		DeleteObject(ctx->hBmp);
	if (ctx->hBmpPrev)
		DeleteObject(ctx->hBmpPrev);
	if (ctx->hDCMemory)
    	DeleteDC(ctx->hDCMemory);
    if (ctx->hwnd)
    	ReleaseDC(ctx->hwnd, ctx->hDCSrc);
    if (ctx->image)
    	free(ctx->image);
    if (ctx->out_buffer)
    	free(ctx->out_buffer);
}

int gdi_loop (gdictx_t *ctx, const int bestFit, const int followCursor, const int usePrintWinAPI, const int maxFPS)
{

	int ct = 0;
	int32_t t0 = GetTickCount();

	while (!kbhit()){
		if (followCursor){
			gdi_close(ctx);
			POINT pt;
			GetCursorPos(&pt);
			HWND hwnd = WindowFromPoint(pt);
			gdi_init(ctx, 0, &pt, hwnd);
		}

		if (gdi_getBitmap(ctx, bestFit, followCursor, usePrintWinAPI)){
			libHidDisplay_WriteImage(&hid_ctx, ctx->out_buffer);
			Sleep((1.0f/(float)maxFPS)*1000);
		}else{
			Sleep(20);
		}
		ct++;
	}
	
	int32_t t1 = GetTickCount();
	printf("FPS: %.1f\n", (float)ct/(float)(t1-t0)*1000.0f);

	return ct;
}

void printHelp ()
{
	printf("\n  DX9 screen and object capture for HidDisplay enabled devices\n");
	printf("\n");
	printf("Usage: -option1 -option2\n");
	printf("-fit      Scale scaptured object to display\n");
	printf("-follow   Capture object under cursor\n");
	printf("-gdi      Use GDI to capture\n");
	printf("-gdi-alt  Use GDI along with PrintWindow() for capturing\n");
	printf("-help     This\n");
	printf("Default behaviour is to capture object under cursor at startup with D3D9\n");
}

int main (int margc, char **cargv)
{
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	//if (argv == NULL || argc < 1) return 0;


	int bestFit = 0;
	int followCursor = 0;
	int gdiCapture = 0;
	int usePrintWinAPI = 0;
	wchar_t *objectName = NULL;

	for (int i = 0; i < argc; i++){
		wchar_t *cmd = argv[i];
		
		if (!wcscmp(L"-fit", cmd)){				// fit image capture to display
			bestFit = 1;
			
		}else if (!wcscmp(L"-follow", cmd)){	// follow cursor
			followCursor = 1;
		
		}else if (!wcscmp(L"-help", cmd)){
			printHelp();
			return 0;
			
		}else if (!wcscmp(L"-gdi", cmd)){
			gdiCapture = 1;
			usePrintWinAPI = 0;

		}else if (!wcscmp(L"-gdi-alt", cmd)){
			gdiCapture = 1;
			usePrintWinAPI = 1;
		
		}else if (!wcscmp(L"-name", cmd)){
			if (++i < argc)
				objectName = argv[i];
		}
	}

	POINT pt;
	GetCursorPos(&pt);
	HWND hwnd;
	
	if (!objectName)
		hwnd = WindowFromPoint(pt);
	else
		hwnd = FindWindowExW(NULL, NULL, objectName, NULL);

	if (!hwnd){
		printf("object not found\n");
		return 0;
	}

	wchar_t title[512] = {0};
	GetWindowTextW(hwnd, title, sizeof(title));
	wprintf(L"Window: \"%ls\"\n", title);

	printf("\n");
	if (!display_init()){
		printf("Device not found or in use\n");
		return 0;
	}

	if (!gdiCapture){
		d3d9ctx_t d3d_ctx;
		
		if (d3d9_init(&d3d_ctx, 0, &pt, hwnd)){
			d3d9_loop(&d3d_ctx, bestFit, followCursor, 50);
			d3d9_close(&d3d_ctx);
		}
	}else{
		gdictx_t gdi_ctx;

		if (gdi_init(&gdi_ctx, 0, &pt, hwnd)){
			gdi_loop(&gdi_ctx, bestFit, followCursor, usePrintWinAPI, 50);
			gdi_close(&gdi_ctx);
		}		
	}

	libHidDisplay_Close(&hid_ctx);
	return 0;

}
