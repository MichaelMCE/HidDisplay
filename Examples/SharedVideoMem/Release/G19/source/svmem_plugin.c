/*****************************************************************************
 * svmem.c: shared memory video driver for vlc
 *****************************************************************************
 * Copyright (C) 2008 the VideoLAN team
 * Copyrgiht (C) 2010 Rémi Denis-Courmont
 * Copyrgiht (C) 2023 MichaelMcE
 *
 * Authors: Sam Hocevar <sam@zoy.org>
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include <vlc_picture_pool.h>
#include "svmem.h"
#include <windows.h>
#include <fcntl.h>




/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
#define T_WIDTH  N_("Width")
#define LT_WIDTH N_("Video memory buffer width.")

#define T_HEIGHT  N_("Height")
#define LT_HEIGHT N_("Video memory buffer height.")

#define T_CHROMA  N_("Chroma")
#define LT_CHROMA N_("Output chroma for the memory image as a 4-character " \
                     "string, eg. \"RV32\".")

#define T_PIPE  N_("Shared pipe name")
#define LT_PIPE N_("Shared global memory pipe named. Must be 6 or more characters.")

#define T_EVENT  N_("Updated event name")
#define LT_EVENT N_("Name given to event which is triggered on each update. "\
					"Must be 6 or more characters.")

#define T_LOCK  N_("Shared semaphore lock name")
#define LT_LOCK N_("Shared global memory semaphore lock. Synchronizes data access."\
				   "Must be 6 or more characters.")

static int  Open (vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_description(N_("Shared video memory output"))
    set_shortname(N_("Shared video memory"))

    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_capability("vout display", 200)

    add_integer("svmem-width", 854, T_WIDTH, LT_WIDTH, false)
    add_integer("svmem-height", 480, T_HEIGHT, LT_HEIGHT, false)
    add_string("svmem-chroma", "RV16", T_CHROMA, LT_CHROMA, true)
    add_string("svmem-pipe", VLC_SMEMNAME, T_PIPE, LT_PIPE, true)
    add_string("svmem-event", VLC_SMEMEVENT, T_EVENT, LT_EVENT, true)
    add_string("svmem-lock", VLC_SMEMLOCK, T_LOCK, LT_LOCK, true)
    	
	add_shortcut("svmem")

    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
struct picture_sys_t {
    vout_display_sys_t *sys;
    vout_display_t *vd;
    void *id;
};

/* NOTE: the callback prototypes must match those of LibVLC */
struct vout_display_sys_t {
    picture_pool_t *pool;
    picture_sys_t *p_sys;
    unsigned        count;

    void *hDataLock;
	void *hUpdateEvent;
	void *hMapFile;
	uint8_t *hMem;
	TSVMEM *svmem;

    unsigned pitches[PICTURE_PLANE_MAX];
    unsigned lines[PICTURE_PLANE_MAX];
};

picture_pool_t *Pool (vout_display_t *, unsigned);
void Prepare (vout_display_t *, picture_t *, subpicture_t *);
void Display (vout_display_t *, picture_t *, subpicture_t *);
int  Control (vout_display_t *, int, va_list);




static void closeSharedMemory (struct vout_display_sys_t *p_sys)
{
	if (p_sys->hUpdateEvent != NULL)
		CloseHandle(p_sys->hUpdateEvent);

	if (p_sys->hDataLock)
		CloseHandle(p_sys->hDataLock);
	
	if (p_sys->hMem != NULL)
		UnmapViewOfFile(p_sys->hMem);
	
	if (p_sys->hMapFile != NULL)
		CloseHandle(p_sys->hMapFile);
		
	p_sys->hDataLock = NULL;
	p_sys->hUpdateEvent = NULL;
	p_sys->hMem = NULL;
	p_sys->hMapFile = NULL;
}

static int createSharedMemory (vout_display_t *vd, struct vout_display_sys_t *p_sys, const char *name, const size_t vsize)
{
	p_sys->hMapFile = (void*)CreateFileMappingA(SYSMEMFILE, NULL, PAGE_READWRITE, 0, vsize, name);
	if (p_sys->hMapFile != NULL){
		p_sys->hMem = (uint8_t*)MapViewOfFile(p_sys->hMapFile, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (p_sys->hMem != NULL){
			return VLC_SUCCESS;
		}else{
			CloseHandle(p_sys->hMapFile);
			p_sys->hMapFile = NULL;
			msg_Err(vd, "MapViewOfFile() returned NULL\n");
        	return VLC_EGENERIC;
		}
	}else{
		msg_Err(vd, "CreateFileMapping() returned NULL\n");
        return VLC_EGENERIC;
	}
}
    
static int createInst (vout_display_t *vd, vout_display_sys_t *p_sys)
{
	char *psz_pipe, *pipename;
    char *psz_event, *eventname;
    char *psz_lock, *lockname;
    TSVMEM *svmem;


	psz_pipe = var_CreateGetString(vd, "svmem-pipe");
    if (psz_pipe == NULL){
        msg_Err(vd, "Pipe name not found. Using \"%s\"",VLC_SMEMNAME);
        pipename = (char*)VLC_SMEMNAME;
	}else if (strlen(psz_pipe) < 6){
		pipename = (char*)VLC_SMEMNAME;
	}else{
		pipename = psz_pipe;
	}

    psz_event = var_CreateGetString(vd, "svmem-event");
    if (psz_event == NULL){
        msg_Err(vd, "Event name not found. Using \"%s\"",VLC_SMEMEVENT);
        eventname = (char*)VLC_SMEMEVENT;
	}else if (strlen(psz_event) < 6){
		eventname = (char*)VLC_SMEMEVENT;
	}else{
		eventname = psz_event;
	}

    psz_lock = var_CreateGetString(vd, "svmem-lock");
    if (psz_lock == NULL){
        msg_Err(vd, "Lock object name not found. Using \"%s\"",VLC_SMEMLOCK);
        lockname = (char*)VLC_SMEMLOCK;
	}else if (strlen(psz_lock) < 6){
		lockname = (char*)VLC_SMEMLOCK;
	}else{
		lockname = psz_lock;
	}
	
	if (VLC_SUCCESS == createSharedMemory(vd, p_sys, pipename, sizeof(TSVMEM)+SVIDEOLENGTH)){
		p_sys->hUpdateEvent = (void*)CreateEventA(NULL, 0, 0, eventname);
		p_sys->hDataLock = CreateSemaphoreA(NULL, 1, 1, lockname);
		
		
		if (WaitForSingleObject(p_sys->hDataLock, 2000) == WAIT_OBJECT_0){
			svmem = p_sys->svmem = (TSVMEM*)p_sys->hMem;
			svmem->hdr.ssize = sizeof(TSVMEMHDR);
			svmem->hdr.vsize = SVIDEOLENGTH;
			svmem->hdr.version = 2;
			svmem->hdr.swidth = vd->fmt.i_width;
			svmem->hdr.sheight = vd->fmt.i_height;
			
			svmem->hdr.fsize = SVIDEOLENGTH;
			if (svmem->hdr.rwidth < 32)
				svmem->hdr.width = 1920;
			if (svmem->hdr.rheight < 32)
				svmem->hdr.height = 1080;
			if (!svmem->hdr.bpp)
				svmem->hdr.bpp = 24;
			svmem->pixels = 0;

			// tell listeners we're alive.
			svmem->hdr.time = 0;
			svmem->hdr.count = 0;
			ReleaseSemaphore(p_sys->hDataLock, 1, NULL);
			//SetEvent(p_sys->hUpdateEvent);
		}else{
			msg_Err(vd, "Could not create or obtain semaphore lock\n");
        	return VLC_EGENERIC;
		}
	}else{
		msg_Err(vd, "Could not create a shared memory block\n");
        return VLC_EGENERIC;
	}
	
	free(psz_pipe);
    free(psz_event);
    free(psz_lock);
    return VLC_SUCCESS;
}


/*****************************************************************************
 * Open: allocates video thread
 *****************************************************************************
 * This function allocates and initializes a vout method.
 *****************************************************************************/
int Open (vlc_object_t *object)
{

    vout_display_t *vd = (vout_display_t *)object;
    vout_display_sys_t *sys = calloc(1, sizeof(*sys));
    if (unlikely(!sys))
        return VLC_ENOMEM;
        
    TSVMEM *svmem;
    int i_width, i_height, i_pitch;
    int desWidth, desHeight;
	int w,h;

#if 0
	picture_sys_t *cfg = calloc(1, sizeof(picture_sys_t));
	if (cfg == NULL){
		free(sys);
		return VLC_ENOMEM;
	}
#endif

	createInst(vd, sys);
	
	i_width = var_CreateGetInteger(vd, "svmem-width");
	i_height = var_CreateGetInteger(vd, "svmem-height");

	char *psz_chroma = var_InheritString(vd, "svmem-chroma");
    vlc_fourcc_t chroma = (vlc_fourcc_t)vlc_fourcc_GetCodecFromString(VIDEO_ES, (void*)psz_chroma);
    free(psz_chroma);
    
    if (!chroma) {
        msg_Err(vd, "chroma should be 4 characters long");
        free(sys);
        return VLC_EGENERIC;
    }
    
	// use requested width/height if available and set
	if (WaitForSingleObject(sys->hDataLock, 2000) == WAIT_OBJECT_0){
		svmem = sys->svmem = (TSVMEM*)sys->hMem;
		if (svmem->hdr.rwidth >= 32 && svmem->hdr.rwidth <= 1920){
			if (svmem->hdr.rheight >= 32 && svmem->hdr.rheight <= 1080){
				i_width = svmem->hdr.rwidth;
				i_height = svmem->hdr.rheight;
				
			}
		}
		ReleaseSemaphore(sys->hDataLock, 1, NULL);
	}
	

    /* Define the video format */
	// set a default pitch. is updated later
    i_pitch = i_width*4;
    
    if (i_pitch*i_height > SVIDEOLENGTH){
		i_width = 1920;
		i_height = 1080;
	}
    if (i_width < 32) i_width = 32;
	if (i_height < 32) i_height = 32;
	
	desWidth = i_width;
	desHeight = i_height;
	video_format_t *source = &vd->source;
	const int bg_h = desHeight;
	const int bg_w = desWidth;
	const int fg_w = source->i_width;
	const int fg_h = source->i_height;
	const int fg_sar_num = 1;//source->i_sar_num;
	const int fg_sar_den = 1;//source->i_sar_den;
	const int bg_sar_den = 1;
	const int bg_sar_num = 1;

	w = bg_w;
	h = ( bg_w * fg_h * fg_sar_den * bg_sar_num ) / (float)( fg_w * fg_sar_num * bg_sar_den );
	if (h > desHeight){
		w = ( bg_h * fg_w * fg_sar_num * bg_sar_den ) / (float)( fg_h * fg_sar_den * bg_sar_num );
		h = bg_h;
	}

	if (w > desWidth)
		w = desWidth;
	if (h > desHeight)
		h = desHeight;

	video_format_t fmt = vd->fmt;
	
	if (WaitForSingleObject(sys->hDataLock, 2000) == WAIT_OBJECT_0){
		svmem = sys->svmem = (TSVMEM*)sys->hMem;
		svmem->hdr.swidth = fmt.i_width;
		svmem->hdr.sheight = fmt.i_height;
			
		svmem->hdr.fsize = w*h*3;
		svmem->hdr.width = w;
		svmem->hdr.height = h;
		svmem->hdr.bpp = 16;
		svmem->pixels = 0;

		svmem->hdr.time = 0;
		svmem->hdr.count = 0;
		ReleaseSemaphore(sys->hDataLock, 1, NULL);
	}else{
		msg_Err(vd, "Could not create or obtain semaphore lock\n");
       	return VLC_EGENERIC;
	}

    fmt.i_chroma = chroma;
    fmt.i_width  = w;
    fmt.i_height = h;

    fmt.i_x_offset = fmt.i_y_offset = 0;
    fmt.i_visible_width = fmt.i_width;
    fmt.i_visible_height = fmt.i_height;

    /* Define the bitmasks */
    switch (fmt.i_chroma)
    {
    case VLC_CODEC_RGB15:
        fmt.i_rmask = 0x7c00;
        fmt.i_gmask = 0x03e0;
        fmt.i_bmask = 0x001f;
		svmem->hdr.bpp = 15;
        i_pitch = w*2;
        break;
        
    case VLC_CODEC_RGB16:
        fmt.i_rmask = 0xf800;
        fmt.i_gmask = 0x07e0;
        fmt.i_bmask = 0x001f;
        svmem->hdr.bpp = 16;
        i_pitch = w*2;
        break;
        
    case VLC_CODEC_RGB24:
    	fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
    	svmem->hdr.bpp = 24;
        i_pitch = w*3;
        break;
        
    case VLC_CODEC_RGBA:
    case VLC_CODEC_RGB32:
        fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
        svmem->hdr.bpp = 32;
        i_pitch = w*4;
        break;
        
    default:
        fmt.i_rmask = 0;
        fmt.i_gmask = 0;
        fmt.i_bmask = 0;
        break;
    }
	svmem->hdr.fsize = i_pitch*h;
	
    sys->pool = NULL;
    sys->pitches[0] = i_pitch;
    sys->lines[0] = fmt.i_height;
    
    for (size_t i = 1; i < PICTURE_PLANE_MAX; i++){
		sys->pitches[i] = sys->pitches[0];
		sys->lines[i] = sys->lines[0];
	}
	sys->count = 1;
	
    /* */
    vout_display_info_t info = vd->info;
    info.needs_hide_mouse = true;

    /* */
    vd->sys     = sys;
    vd->fmt     = fmt;
    vd->info    = info;
    vd->pool    = Pool;
    vd->prepare = Prepare;
    vd->display = Display;
    vd->control = Control;

    //vout_display_SendEventDisplaySize(vd, fmt.i_width, fmt.i_height);
    vout_display_SendEventDisplaySize(vd, fmt.i_width, 64);
    vout_display_DeleteWindow(vd, NULL);
    
    return VLC_SUCCESS;
}

void Close (vlc_object_t *object)
{
    vout_display_t *vd = (vout_display_t *)object;
    vout_display_sys_t *sys = vd->sys;

   //picture_sys_t *p_sys = sys->p_sys;

	if (WaitForSingleObject(sys->hDataLock, 1000) == WAIT_OBJECT_0){
		sys->svmem->hdr.count = 0;
		sys->svmem->hdr.time = 0;
		sys->svmem->hdr.ssize = 0;
		ReleaseSemaphore(sys->hDataLock, 1, NULL);
		SetEvent(sys->hUpdateEvent);
	}
	closeSharedMemory(sys);

	if (sys->pool)
    	picture_pool_Release(sys->pool);
    free(sys);
}


void Prepare (vout_display_t *vd, picture_t *pic, subpicture_t *subpic)
{
    vout_display_sys_t *sys = vd->sys;
    picture_resource_t rsc = {.p_sys = NULL};
    uint8_t *pixels = (uint8_t*)&sys->svmem->pixels;

    for (unsigned i = 0; i < PICTURE_PLANE_MAX; i++) {
        rsc.p[i].p_pixels = pixels;
        rsc.p[i].i_lines  = sys->lines[i];
        rsc.p[i].i_pitch  = sys->pitches[i];
    }

    picture_t *locked = picture_NewFromResource(&vd->fmt, &rsc);
    if (likely(locked != NULL)) {
#if 0
		picture_CopyPixels(locked, pic);
#else
		if (WaitForSingleObject(sys->hDataLock, 1000) == WAIT_OBJECT_0){
			TSVMEM *svmem = sys->svmem;
    		svmem->hdr.time = GetTickCount();
			svmem->hdr.count++;
			
			picture_CopyPixels(locked, pic);
			ReleaseSemaphore(sys->hDataLock, 1, NULL);
			SetEvent(sys->hUpdateEvent);
		}
#endif
        picture_Release(locked);
    }

    (void)subpic;
}

picture_pool_t *Pool (vout_display_t *vd, unsigned count)
{
    vout_display_sys_t *sys = vd->sys;

    if (sys->pool == NULL)
        sys->pool = picture_pool_NewFromFormat(&vd->fmt, count);
    return sys->pool;
}

void Display (vout_display_t *vd, picture_t *pic, subpicture_t *subpic)
{
#if 1
	(void)vd;
#else
    vout_display_sys_t *sys = vd->sys;

	if (WaitForSingleObject(sys->hDataLock, 1000) == WAIT_OBJECT_0){
		TSVMEM *svmem = sys->svmem;
    	svmem->hdr.time = GetTickCount();
		svmem->hdr.count++;
	
		ReleaseSemaphore(sys->hDataLock, 1, NULL);
		SetEvent(sys->hUpdateEvent);
	}
#endif
    picture_Release(pic);
    VLC_UNUSED(subpic);
}

int Control (vout_display_t *vd, int query, va_list args)
{
	VLC_UNUSED(vd);
	VLC_UNUSED(query);
	VLC_UNUSED(args);
	
	return VLC_SUCCESS;
}
