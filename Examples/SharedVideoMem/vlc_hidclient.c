
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



/*


Intended as a client for the VLC 2.0.x shared video out plugin libsvmem_plugin.dll

Start VLC before this:
VLC_CaptureServer.bat

or manually via:
vlc.exe --vout=svmem --svmem-width=854 --svmem-height=480 --svmem-chroma=RV16 "videofile.mp4"

*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <inttypes.h>
#include <math.h>
#include <libHidDisplay.h>
#include "plugin/svmem.h"

// will store display dimensions 
static int DWIDTH = 0;
static int DHEIGHT = 0;

typedef struct {
	int width;
	int height;
	int bpp;
	int yOffset;
	void *frame;
	uint32_t frameAllocSize;
	HANDLE hMapFile;
	uint8_t *hMem;
}imagemap_t;


static teensyRawHidcxt_t ctx;
static rawhid_header_t desc;
static imagemap_t img;




int openSharedMemory (imagemap_t *img, const char *name)
{
	img->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, VLC_SMEMNAME);
	if (img->hMapFile != NULL){
		img->hMem = (uint8_t*)MapViewOfFile(img->hMapFile, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (img->hMem != NULL){
			//printf("MapViewOfFile(): Ok - 0x%p\n", (uintptr_t*)img->hMem);
			return 1;
		}else{
			//printf("MapViewOfFile() failed with error %d\n", (int)GetLastError());
			CloseHandle(img->hMapFile);
		}
	}else{
		//printf("OpenFileMapping() failed with error %d\n", (int)GetLastError());
	}
	return 0;
}

void closeSharedMemory (imagemap_t *img)
{
	UnmapViewOfFile(img->hMem);
	CloseHandle(img->hMapFile);
}

int openDisplayWait (const int timeMs)
{
	const int delay = 20;
	const int loops = timeMs/delay;
	
	for (int i = 0; i < loops; i++){
		Sleep(delay);
			
		if (libHidDisplay_OpenDisplay(&ctx, 0))
			return 1;
	}
	return 0;
}

int display_init ()
{
	if (!libHidDisplay_OpenDisplay(&ctx, 0)){
		if (!openDisplayWait(500))
			return 0;
	}

	libHidDisplay_GetConfig(&ctx, &desc);
		
	ctx.width = desc.u.cfg.width;
	ctx.height = desc.u.cfg.height;
	ctx.pitch = desc.u.cfg.pitch;
	ctx.rgbClamp = desc.u.cfg.rgbMax;
	if (!desc.u.cfg.stripHeight) desc.u.cfg.stripHeight = 32;
	
	DWIDTH = ctx.width;
	DHEIGHT = ctx.height;

	if (!DWIDTH || !DHEIGHT){
		libHidDisplay_Close(&ctx);
		return 0;
	}

	printf("Found device: %s\nWxH:%ix%i\n", desc.u.cfg.string, DWIDTH, DHEIGHT);
	return 1;
}

// when source and destination widths match
int updateDisplay_aligned (uint16_t *pixels, const int yOffset, const int stripHeight)
{
	//printf("updateDisplay_aligned\n");
	
	int ret = 0;
	const int twrites = img.height / stripHeight;
	

	for (int i = 0; i < twrites; i++){
		int y = i * stripHeight;
		ret += libHidDisplay_WriteArea(&ctx, &pixels[y * img.width], 0, y+yOffset, img.width-1, yOffset+y+stripHeight-1);
	}

	const int remaining = img.height % stripHeight;
	if (remaining && ret){
		for (int i = twrites; i < twrites+1; i++){
			int y = i * stripHeight;
			libHidDisplay_WriteArea(&ctx, &pixels[y * img.width], 0, y+yOffset, img.width-1, yOffset+y+remaining-1);
		}
	}
	return (ret != 0);
}

// when source destination width is smaller than device width it must be realigned
int updateDisplay_unaligned (uint16_t *pixels, const int yOffset, const int stripHeight)
{
	//printf("updateDisplay_unaligned\n");
	
	int ret = 0;
	const int twrites = img.height / stripHeight;
	
	uint16_t out[DWIDTH * stripHeight];
	memset(out, 0, sizeof(out));
	
	int xOffset = (DWIDTH - img.width) / 2;
	if (xOffset < 0) xOffset = 0;
	
	for (int i = 0; i < twrites; i++){
		int y = i * stripHeight;
		
		for (int c = 0; c < stripHeight; c++)
			memcpy(&out[(c*DWIDTH)+xOffset], &pixels[(y+c)*img.width], img.width*2);

		ret += libHidDisplay_WriteArea(&ctx, out, 0, y+yOffset, DWIDTH-1, yOffset+y+stripHeight-1);
	}

	const int remaining = img.height % stripHeight;
	if (remaining && ret){
		for (int i = twrites; i < twrites+1; i++){
			int y = i * stripHeight;
			
			for (int c = 0; c < remaining; c++)
				memcpy(&out[c*DWIDTH], &pixels[(y+c) * img.width], img.width*2);

			ret += libHidDisplay_WriteArea(&ctx, out, 0, y+yOffset, DWIDTH-1, yOffset+y+remaining-1);			
		}
	}
	return (ret != 0);
}

// when source destination width is smaller than device width it must be realigned
int updateDisplay_unaligned_op (uint16_t *pixels, const int yOffset, const int stripHeight)
{
	//printf("updateDisplay_unaligned_op\n");
	
	int ret = 0;
	const int twrites = img.height / stripHeight;

	int width = img.width;
	int xOffset = (DWIDTH - width) / 2;
	if (xOffset < 0) xOffset = 0;

	int xOffsetFudge = 0;
	if (width&0x01)		// if odd increase xoffset to offset smearing effect
		xOffsetFudge = 1;
	
	for (int i = 0; i < twrites; i++){
		int y = i * stripHeight;
		ret += libHidDisplay_WriteArea(&ctx, &pixels[y*width], xOffset+xOffsetFudge, y+yOffset, (DWIDTH-xOffset)-1, (yOffset+y+stripHeight)-1);
	}

	const int remaining = img.height % stripHeight;
	if (remaining && ret){
		for (int i = twrites; i < twrites+1; i++){
			int y = i * stripHeight;
			ret += libHidDisplay_WriteArea(&ctx,  &pixels[y*width], xOffset+xOffsetFudge, y+yOffset, (DWIDTH-xOffset)-1, (yOffset+y+remaining)-1);
		}
	}
	return (ret != 0);
}


// display, source, best fit out
void imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h)
{
	const int fg_sar_num = 1; const int fg_sar_den = 1;
	const int bg_sar_den = 1; const int bg_sar_num = 1;

	if (fg_w < 1 || fg_w > 8191) fg_w = bg_w;
	if (fg_h < 1 || fg_h > 8191) fg_h = bg_h;
	*w = bg_w;
	*h = (bg_w * fg_h * fg_sar_den * bg_sar_num) / (float)(fg_w * fg_sar_num * bg_sar_den);
	if (*h > bg_h){
		*w = (bg_h * fg_w * fg_sar_num * bg_sar_den) / (float)(fg_h * fg_sar_den * bg_sar_num);
		*h = bg_h;
	}
}

int main (int argc, char* argv[])
{
	if (!display_init()){
		printf("Display not found or connection in use\n");
		return 0;
	}

	HANDLE hUpdateEvent = NULL;
	HANDLE hDataLock = NULL;
	TSVMEM *svmem = NULL;
	int gotMapHandle = 0;

	printf("Waiting for frame server..\n");
	
	while(!kbhit()){
		gotMapHandle = openSharedMemory(&img, VLC_SMEMNAME);
		if (gotMapHandle){
			printf("Connected\n");
			break;
		}
		Sleep(50);
	};


	//MEMORY_BASIC_INFORMATION mbi = {0};
	//VirtualQueryEx(GetCurrentProcess(), img.hMem, &mbi, sizeof(mbi));


	if (gotMapHandle){
		svmem = (TSVMEM*)img.hMem;
		
		// VLC will set a global event, VLC_SMEMEVENT, on each update
		hUpdateEvent = CreateEvent(NULL, 0, 0, VLC_SMEMEVENT);
		
		// data access is synchronized through a semaphore 
		hDataLock = CreateSemaphore(NULL, 0, 1, VLC_SMEMLOCK);

		// playback may need to be restarted for any change to these values to take effect
		// tell VLC the resolution we need
		svmem->hdr.rwidth = DWIDTH;
		svmem->hdr.rheight = DHEIGHT;

		img.yOffset = 0;
		img.width = -1;
		img.height = -1;
		img.bpp = -1;
		img.frameAllocSize = sizeof(uint32_t) * DWIDTH * DHEIGHT;
		img.frame = calloc(1, img.frameAllocSize);
		if (!img.frame) abort();


		while(!kbhit()){
			// wait for the frame ready signal from the plugin
			if (WaitForSingleObject(hUpdateEvent, 500) == WAIT_OBJECT_0){
				// lock the IPC. 
				// this also blocks VLC from updating the buffer
				if (WaitForSingleObject(hDataLock, 1000) == WAIT_OBJECT_0){
					if (svmem->hdr.count < 2){
						memset(img.frame, 0, sizeof(uint16_t)*DWIDTH*DHEIGHT);
						libHidDisplay_WriteImage(&ctx, img.frame);
					}
					
					if (img.bpp != svmem->hdr.bpp){
						img.bpp = svmem->hdr.bpp;
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;
						img.yOffset = abs(DHEIGHT - svmem->hdr.height) / 2;
						
						memset((void*)&svmem->pixels, 0, svmem->hdr.vsize);
						memset(img.frame, 0, img.frameAllocSize);
						libHidDisplay_WriteImage(&ctx, img.frame);

					}else if (img.width != svmem->hdr.width || img.height != svmem->hdr.height){
						img.yOffset = abs(DHEIGHT - svmem->hdr.height) / 2;
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;

						memset((void*)&svmem->pixels, 0, svmem->hdr.vsize);
						memset(img.frame, 0, img.frameAllocSize);
						libHidDisplay_WriteImage(&ctx, img.frame);
					}

					if (svmem->hdr.ssize && svmem->hdr.fsize)
						memcpy(img.frame, (void*)&svmem->pixels, svmem->hdr.fsize);

					ReleaseSemaphore(hDataLock, 1, NULL);

					int w, h;
					imageBestFit(DWIDTH, DHEIGHT, svmem->hdr.swidth, svmem->hdr.sheight, &w, &h);
					//printf("%i %i, %i %i, %i %i, %i %i\n", img.width, img.height, DWIDTH, DHEIGHT, svmem->hdr.swidth, svmem->hdr.sheight, w, h);
					int devStatus = 0;

					/*if (w < DWIDTH){
						int x = ((DWIDTH - w) / 2);
						devStatus = libHidDisplay_WriteArea(&ctx, img.frame, x, 0, x+w-1, h-1);
					}else */if (img.width == DWIDTH)			// stripHeight should match that of the display
						devStatus = updateDisplay_aligned((uint16_t*)img.frame, img.yOffset, desc.u.cfg.stripHeight);
					else if ((abs(DWIDTH-img.width) > 32))	// intended for vertical videos
						devStatus = updateDisplay_unaligned_op((uint16_t*)img.frame, img.yOffset, desc.u.cfg.stripHeight);
					else
						devStatus = updateDisplay_unaligned((uint16_t*)img.frame, img.yOffset, desc.u.cfg.stripHeight);
					if (!devStatus){
						printf("no converter found\ngoing down\n");
						break;
					}
				}
			}else{
				Sleep(25);
			}
		}
		
		if (img.frame)
			free(img.frame);
		if (hUpdateEvent)
			CloseHandle(hUpdateEvent);
		if (hDataLock)
			CloseHandle(hDataLock);
		closeSharedMemory(&img);	
	}

	libHidDisplay_Close(&ctx);
	return 1;
}
