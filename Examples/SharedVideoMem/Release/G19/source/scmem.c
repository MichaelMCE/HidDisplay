// Shared Memory Client
//
//  Copyright (c) 2005-2023  Michael McE
//  okio@users.sourceforge.net
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

Intended as a client for the VLC 3.0.x shared video out plugin libsvmem_plugin.dll

Start VLC before this:
vlc.exe --vout=svmem --svmem-width=854 --svmem-height=480 --svmem-chroma=RV16 "videofile.mp4"

*/

#include <stdio.h>
#include <stdlib.h>

#include "mylcd.h"
#include "demos.h"
#include "svmem.h"


typedef struct {
	int width;
	int height;
	int bpp;
	TFRAME *frame;
	
	HANDLE hMapFile;
	uint8_t *hMem;
}TSIMAGE;

const char bpplookup[] = {1, 8, 12, 15, 16, 24, 32, 32};
#define TARGET_BPP		16




void resetCurrentDirectory ()
{
	wchar_t drive[MAX_PATH+1];
	wchar_t dir[MAX_PATH+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	__mingw_swprintf(szPath, L"%ls%ls", drive, dir);
	
	SetCurrentDirectoryW(szPath);
}


static int getBpp (int bpp)
{
	for (int i = 0; i < 7; i++){
		if (bpplookup[i] == bpp)
			return i;
	}
	return LFRM_BPP_32;
}

// initiate VLC playback before calling this
static int openSharedMemory (TSIMAGE *img, const char *name)
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

static void closeSharedMemory (TSIMAGE *img)
{
	UnmapViewOfFile(img->hMem);
	CloseHandle(img->hMapFile);
}
			
int main (int argc, char* argv[])
{
	// enable datafiles to be found
	resetCurrentDirectory();

	if (!initDemoConfig("config.cfg"))
		return 0;

	HANDLE hUpdateEvent = NULL;
	HANDLE hDataLock = NULL;
	TSVMEM *svmem = NULL;
	TSIMAGE img;
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


	if (gotMapHandle){
		svmem = (TSVMEM*)img.hMem;
		
		// VLC will set a global event, VLC_SMEMEVENT, on each update
		hUpdateEvent = CreateEvent(NULL, 0, 0, VLC_SMEMEVENT);
		
		// data access is synchronized through a semaphore 
		hDataLock = CreateSemaphore(NULL, 0, 1, VLC_SMEMLOCK);

		img.width = -1;
		img.height = -1;
		img.bpp = -1;
		img.frame = lNewFrame(hw, DWIDTH, DHEIGHT, getBpp(TARGET_BPP));
		//uint32_t ftime = GetTickCount();
		

		// playback may need to be restarted for any change to these values to take effect
		svmem->hdr.rwidth = DWIDTH;
		svmem->hdr.rheight = DHEIGHT;
		
		while(!kbhit()){
			// wait for the frame ready signal from the plugin
			//printf("%ix%i %i %i: %ix%i, %ix%i, %ix%i\n", img.frame->width, img.frame->height, img.frame->bpp, LFRM_BPP_24, svmem->hdr.rwidth, svmem->hdr.rheight, svmem->hdr.swidth, svmem->hdr.sheight, svmem->hdr.width, svmem->hdr.height);
			
			if (WaitForSingleObject(hUpdateEvent, 500) == WAIT_OBJECT_0){
				// lock the IPC. 
				// because this also blocks VLC from updating the buffer, intensive operations
				// should be avoided inside the lock.
				// best to copy out the video frame, release the semaphore then perform your ops
				if (WaitForSingleObject(hDataLock, 1000) == WAIT_OBJECT_0){
					//printf("%i %u %i\n", svmem->hdr.count, svmem->hdr.time-ftime, svmem->hdr.fsize);
					
					// If playback has just begun then clear display of previous playback
					if (svmem->hdr.count < 2){
						lClearFrame(frame);
						lRefresh(frame);
					}
					
					// always verify bpp, width and height as they're likely to change between playbacks
					if (img.bpp != svmem->hdr.bpp){
						printf("bpp: %ix%i %i %i %i: %ix%i, %ix%i, %ix%i\n", img.frame->width, img.frame->height, img.bpp, img.frame->bpp, LFRM_BPP_16, svmem->hdr.rwidth, svmem->hdr.rheight, svmem->hdr.swidth, svmem->hdr.sheight, svmem->hdr.width, svmem->hdr.height);
						 
						img.bpp = svmem->hdr.bpp;
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;
						lDeleteFrame(img.frame);
						img.frame = lNewFrame(hw, img.width, img.height, getBpp(img.bpp));
						lClearFrame(frame);
						
					}else if (img.width != svmem->hdr.width || img.height != svmem->hdr.height){
						printf("Resizing image: %ix%i %i %i: %ix%i, %ix%i, %ix%i\n", img.frame->width, img.frame->height, img.frame->bpp, LFRM_BPP_24, svmem->hdr.rwidth, svmem->hdr.rheight, svmem->hdr.swidth, svmem->hdr.sheight, svmem->hdr.width, svmem->hdr.height);
						
						// clear front and back buffers
						lClearFrame(img.frame);
						lRefreshAsync(img.frame, 1);
						lClearFrame(img.frame);
						memset((void*)&svmem->pixels, 0, svmem->hdr.vsize);
						
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;
						lResizeFrame(img.frame, img.width, img.height, 0);
						lClearFrame(frame);
					}

					// copy image out of mapped file to our working buffer
					if (svmem->hdr.ssize && svmem->hdr.fsize){
#if 1
						memcpy(lGetPixelAddress(img.frame, 0, 0), (void*)&svmem->pixels, svmem->hdr.fsize);
#else
						int y = (abs(DHEIGHT - svmem->hdr.height)/2)-1;
						if (y < 0) y = 0;
						void *des = lGetPixelAddress(img.frame, 0, y);
						if (des)
							memcpy(des, (void*)&(svmem->pixels), svmem->hdr.fsize);
#endif
					}
					ReleaseSemaphore(hDataLock, 1, NULL);

					// Send frame to device via a separate thread
					lRefreshAsync(img.frame, 1);
				}
			}else{
				//printf("sleeping..\n");
				lSleep(30);
				//break;
			}
		}
		lDeleteFrame(img.frame);
		CloseHandle(hUpdateEvent);
		CloseHandle(hDataLock);
		closeSharedMemory(&img);	
	}

	demoCleanup();
	return 1;
}
