/*****************************************************************************
 * svmem.h: shared memory video driver for vlc
 *****************************************************************************
 * Copyright (C) 2008 the VideoLAN team
 * $Id:
 *
 * Authors: Michael McElligott
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

// maximum vsize
#define SVIDEOLENGTH	((1920*1080*4)+64)

// win32 system memory file magic
#define SYSMEMFILE		((HANDLE)0xFFFFFFFF)

// name of shared pipe
#define VLC_SMEMNAME	"VLC_SVIDEOPIPE"

// name of update event
#define VLC_SMEMEVENT	"VLC_SVIDEOEVENT"

// name of synchronization object protecting the shared memory
#define VLC_SMEMLOCK	"VLC_SVIDEOLOCK"



typedef struct {
	uint32_t ssize;			// header size, this structure size
	uint32_t vsize;			// video space size.
	uint32_t fsize;			// size of frame contained in video space
	uint16_t swidth;		// source width
	uint16_t sheight;		// source height
	uint16_t rwidth;		// width requested by client.
	uint16_t rheight;		// height requested by client.

	uint32_t time;			// TODO. source image time. Is currently assigned a GetTickCount()
	uint32_t count;			// image count index. incremented by 1 on each frame write
	uint16_t width;			// frame width
	uint16_t height; 		// frame height
	uint8_t  bpp;			// frame bits per pixel (15, 16, 24 or 32)
	uint8_t  version;		// header version. Currently at v0.2

	uint8_t  padding[28];	// pad struct to 64 bytes
}TSVMEMHDR;

typedef struct {
	TSVMEMHDR hdr;
	uint32_t  pixels;		// first pixel
}TSVMEM;


