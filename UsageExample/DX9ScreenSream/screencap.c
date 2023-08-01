

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <math.h>
#include <d3d9.h>
#include "../../libTeensyRawHid/libTeensyRawHid.h"



static int DWIDTH = 0;
static int DHEIGHT = 0;
static teensyRawHidcxt_t ctx;



static int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libTeensyRawHid_OpenDisplay(&ctx))
			return 1;
	}
	return 0;
}

int display_init ()
{
	if (!libTeensyRawHid_OpenDisplay(&ctx)){
		if (!openDisplayWait(500))
			return 0;
	}

	rawhid_header_t desc;
	libTeensyRawHid_GetConfig(&ctx, &desc);
		
	ctx.width = desc.u.cfg.width;
	ctx.height = desc.u.cfg.height;
	ctx.pitch = desc.u.cfg.pitch;
	ctx.rgbClamp = desc.u.cfg.rgbMax;
	
	DWIDTH = ctx.width;
	DHEIGHT = ctx.height;

	if (!DWIDTH || !DHEIGHT){
		libTeensyRawHid_Close(&ctx);
		return 0;
	}

	//printf("Display Width:%i Height:%i\n%s\n", DWIDTH, DHEIGHT, desc.u.cfg.string);
	printf("Device: %s\n", desc.u.cfg.string);
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

int Direct3D9Capture (const int adapter, const int bestFit, const int followCursor)
{

	IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d) return 0;
	
	D3DDISPLAYMODE mode;
	if (IDirect3D9_GetAdapterDisplayMode(d3d, adapter, &mode) != S_OK)
		goto cleanup;

	POINT pt;
	GetCursorPos(&pt);
	HWND hwnd = WindowFromPoint(pt);
	
	wchar_t title[256] = {0};
	GetWindowTextW(hwnd, title, sizeof(title));
	wprintf(L"Window: \"%ls\"\n", title);
	
	D3DPRESENT_PARAMETERS d3dpp; 
	memset(&d3dpp, 0, sizeof(d3dpp));
	
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.Windowed = TRUE;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferCount = 1;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	//d3dpp.EnableAutoDepthStencil = TRUE;
	//d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	IDirect3DDevice9 *device = NULL;
	if (IDirect3D9_CreateDevice(d3d, adapter, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device) != S_OK)
		goto cleanup;

	IDirect3DSurface9 *surface = NULL;
	if (IDirect3DDevice9_CreateOffscreenPlainSurface(device, mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL) != S_OK)
		goto cleanup;


	D3DLOCKED_RECT rc;
	RECT rect;
	int src_pitch = DWIDTH*2;

	if (IDirect3DSurface9_LockRect(surface, &rc, &rect, D3DLOCK_NOSYSLOCK|D3DLOCK_NOOVERWRITE|D3DLOCK_DONOTWAIT|D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY) == S_OK){
		src_pitch = rc.Pitch;
		IDirect3DSurface9_UnlockRect(surface);
	}

	uint8_t *out_buffer = malloc(src_pitch * DHEIGHT);
	if (!out_buffer) goto cleanup;

	RECT crect;
	int ct = 0;
	int32_t t0 = GetTickCount();
  	
	while (!kbhit()){
		if (!followCursor){
			GetClientRect(hwnd, &crect);
    		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&crect, 1);

			rect.left = abs(crect.left);
			rect.right = rect.left + crect.right;
			rect.top = abs(crect.top);
			rect.bottom = rect.top + crect.bottom;

			if (rect.left < 0) rect.left = 0;
			if (rect.top < 0) rect.top = 0;
			if (rect.right > mode.Width-1) rect.right = mode.Width-1;
			if (rect.bottom > mode.Height-1) rect.bottom = mode.Height-1;
#if 0
			if ((rect.bottom - rect.top)+1 > DHEIGHT) rect.bottom -= ((rect.bottom - rect.top) - DHEIGHT);
			if ((rect.right - rect.left)+1 > DWIDTH) rect.right -= ((rect.right - rect.left) - DWIDTH);
#endif	  
		}else if (bestFit){
			GetCursorPos(&pt);
			hwnd = WindowFromPoint(pt);
			GetClientRect(hwnd, &crect);
			MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&crect, 1);

			rect.left = abs(crect.left);
			rect.right = rect.left + crect.right;
			rect.top = abs(crect.top);
			rect.bottom = rect.top + crect.bottom;

			if (rect.left < 0) rect.left = 0;
			if (rect.top < 0) rect.top = 0;
			if (rect.right > mode.Width-1) rect.right = mode.Width-1;
			if (rect.bottom > mode.Height-1) rect.bottom = mode.Height-1;

		}else{		// follow cursor
			GetCursorPos(&pt);
			rect.left = pt.x - (DWIDTH/2);
			if (rect.left < 0) rect.left = 0;
			rect.right = rect.left + DWIDTH;
			if (rect.right > mode.Width-1){
				rect.right = mode.Width-1;
				rect.left = rect.right - DWIDTH;
			}
		
			rect.top = pt.y - (DHEIGHT/2);
			if (rect.top < 0) rect.top = 0;
			rect.bottom = rect.top + DHEIGHT;
			if (rect.bottom > mode.Height-1){
				rect.bottom = mode.Height-1;
				rect.top = rect.bottom - DHEIGHT;
			}
		}

		if (IDirect3DDevice9_GetFrontBufferData(device, 0, surface) == S_OK){
			if (IDirect3DSurface9_LockRect(surface, &rc, NULL, D3DLOCK_DONOTWAIT|D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY) == S_OK){
				if (!bestFit)
					frame32To16(&rect, rc.pBits, out_buffer, mode.Width, DWIDTH*2);
				else
					frame32To16Fit(&rect, rc.pBits, out_buffer, mode.Width, DWIDTH*2);
				IDirect3DSurface9_UnlockRect(surface);
			}
	  		libTeensyRawHid_WriteImage(&ctx, out_buffer);
		}else{
			Sleep(20);
		}

		//Sleep(120);

		ct++;
	}
	
	int32_t t1 = GetTickCount();
	printf("FPS: %.1f\n", (float)ct/(float)(t1-t0)*1000.0f);
	

cleanup:
	if (out_buffer) free(out_buffer);
	if (surface) IDirect3DSurface9_Release(surface);
	if (device) IDirect3DDevice9_Release(device);
	if (d3d) IDirect3D9_Release(d3d);
	
	return ct;
}

void printHelp ()
{
	printf("\n  DX9 screen and object capture for HidDisplay enabled devices\n");
	printf("\n");
	printf("Usage: -option1 -option2\n");
	printf("-fit      Scale scaptured object to display\n");
	printf("-follow   Capture object under cursor\n");
	printf("-help     This\n");
}


int main (int margc, char **cargv)
{
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	//if (argv == NULL || argc < 1) return 0;


	int bestFit = 0;
	int followCursor = 0;
	
	for (int i = 0; i < argc; i++){
		wchar_t *cmd = argv[i];
		
		if (!wcscmp(L"-fit", cmd)){				// fit image capture to display
			bestFit = 1;
			
		}else if (!wcscmp(L"-follow", cmd)){	// follow cursor
			followCursor = 1;
		
		}else if (!wcscmp(L"-help", cmd)){
			printHelp();
			return 0;
		}
	}

	printf("\n");
	if (!display_init()){
		printf("Device not found or in use\n");
		return 0;
	}


	Direct3D9Capture(0, bestFit, followCursor);

	return 0;
	
}

