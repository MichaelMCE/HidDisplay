/********************************************************************
*
* tgx library: Crazy clock demo.
*
* This example show how to use blitting of rotated/rescaled sprites
* with transparency. 
*
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240)
*
* CREDIT : Created by Bruno@diylab.de. Thanks !
********************************************************************/


// the tgx library 
#include <windows.h> 
#include <tgx.h>
#include <Renderer3D.h>
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>
#include <Color.h>
#include <Color.cpp>
#include <stdio.h>
#include <string.h>
#include "../../../../../../libTeensyRawHid/libTeensyRawHid.h"



// let's not burden ourselves with the tgx:: prefix
using namespace tgx;

// the sprites
#include "green.h"  // the clock, RGB565 format, no transparency
#include "long_hand.h"  // long hand, RGB32 format (with transparency)
#include "small_hand.h" // small hand, RGB32 format (with transparency)


// screen dimension (portrait mode)

#define FLASHMEM  
#define DMAMEM  
#define elapsedMillis uint64_t
#define elapsedMicros uint64_t
static teensyRawHidcxt_t ctx;


static int DWIDTH = 0;
static int DHEIGHT = 0;
static int SLX = DWIDTH;
static int SLY = DHEIGHT;
static float SLX2 = DWIDTH/2;
static float SLY2 = DHEIGHT/2;

#if 0
static uint16_t fb[SLX * SLY];

// internal framebuffer (150K) used by the ILI9431_T4 library for double buffering.
static uint16_t internal_fb[SLX * SLY]; 

// zbuffer in 16bits precision (150K in DMAMEM)
static DMAMEM uint16_t zbuf[SLX * SLY];              

// image that encapsulates the framebuffer fb.
static Image<RGB565> im(fb, SLX, SLY);
#else 
static uint16_t *fb;
static uint16_t *internal_fb; 
static uint16_t *zbuf;
static Image<RGB565> im;
#endif

/* draw the clock with given angle, scale and opacity */
void drawClock(float angle, float scale= 1.0f, float opacity = 1.0f)
{
    if (opacity == 1.0f){ // the clock does not use trnasparency so we can use the faster method when opacity = 1.0f 
        im.blitScaledRotated(green, { 240/2,240/2 }, { SLX2,SLY2 }, scale, angle);  
	}else{
        im.blitScaledRotated(green, { 240/2,240/2 }, { SLX2,SLY2 }, scale, angle, opacity);  
	}
} 


/* draw the small hand with given angle, scale and opacity */
void drawSmallHand(float angle, float scale= 1.0f, float opacity = 1.0f)
{ 
    // always use the method with blending, even when opacity = 1.0f because the hand has transparency
    im.blitScaledRotated(small_hand, { 22,117 }, { SLX2,SLY2 }, 0.55f * scale, angle, opacity);
} 


/* draw long hand with given angle, scale and opacity */
void drawLongHand(float angle, float scale= 1.0f, float opacity = 1.0f)
{
    // always use the method with blending, even when opacity = 1.0f because the hand has transparency
    im.blitScaledRotated(long_hand, { 14,197 }, { SLX2,SLY2 }, 0.5f * scale, angle, opacity);
} 


static uint64_t fcount = 0;

void update ()
{
	static uint64_t tstart = GetTickCount();
	
	libTeensyRawHid_WriteImageEx(&ctx, fb, 246);
	
	fcount++;
	
	if (!(fcount%100)){
		uint64_t tend = GetTickCount();
		printf("%.1f fps\n", (float)fcount / (float)(tend - tstart)*1000.0f);
	}
}


int openDisplayWait (const int timeMs)
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
	
	DWIDTH = ctx.width;
	DHEIGHT = ctx.height;
	
	SLX = DWIDTH;
	SLY = DHEIGHT;
	SLX2 = DWIDTH/2;
	SLY2 = DHEIGHT/2;

	fb = (uint16_t*)calloc(2, SLX * SLY);
	internal_fb = (uint16_t*)calloc(2, SLX * SLY);
	zbuf = (uint16_t*)calloc(2, SLX * SLY);


	static Image<RGB565> _im(fb, SLX, SLY);;
	memcpy((void*)&im, (void*)&_im, sizeof(_im));

	return 1;
}

int main ()
{

	if (!display_init())
		return 0;
	
	printf("%i %i\n", SLX, SLY);


	
    int t;
	uint64_t em = 0;
	uint64_t time0 = GetTickCount();
	uint64_t tstart = GetTickCount();
	timeBeginPeriod(1);


	while(em < 3000){
		
        int y1 = (em < 1000) ? (130 * em) / 1000 - 50 : 80;
        int y2 = (em < 1000) ? 240 - (110 * em) / 1000 : 130;
        im.fillScreen(RGB565_Black);  
        im.drawText("TGX library",iVec2{110,y1}, RGB565_Red, font_tgx_OpenSans_Bold_18, true);
        im.drawText("Crazy clock demo",iVec2{35,y2}, RGB565_White, font_tgx_OpenSans_Bold_28, true);      

       	update();
       	//Sleep(10);
       	em = GetTickCount() - time0;
	}

    em = 0;
    time0 = GetTickCount();
    
    while ((t = em) < 1000){      
    	
        float sc = 0.1f + 0.9f*t/1000.0f;
        im.fillScreen(RGB565_Black);  
        im.drawText("TGX library",iVec2{110,80}, RGB565_Red, font_tgx_OpenSans_Bold_18, true, 1.0f - t/1000.0f);
        im.drawText("Crazy clock demo",iVec2{35,130}, RGB565_White, font_tgx_OpenSans_Bold_28, true, 1.0f - t/1000.0f);
        drawClock(200 - t/5, sc);
        drawSmallHand(t/10, sc);
        drawLongHand(t/2, sc); 

      	update();
      	//Sleep(10);
      	em = GetTickCount() - time0;
	}

    // PART 2: rotation
    em = 0;
    time0 = GetTickCount();
    
    while ((t = em) < 10000){
        im.fillScreen(RGB565_Black);  
        drawClock(0, 1.0f);
        drawSmallHand(100 +360*sinf(t / 1500.0f), 1.0f);
        drawLongHand(500*cosf(t / 5000.0f), 1.0f); 

       	update();
		//Sleep(10);
       	em = GetTickCount() - time0;
	}  
	
   // PART 3: changing hands sizes
    while ((t = em) < 20000){
        im.fillScreen(RGB565_Black);  
        drawClock(0, 1.0f);
        drawSmallHand(100 +360*sinf(t / 1500.0f), 0.292893 + fabsf(sinf( 0.7853981 + (t-10000.0f) / 3000.0f)));
        drawLongHand(500*cosf(t / 5000.0f), 0.292893 + fabsf(cosf( 0.7853981 + (t-10000.0f) / 3000.0f))); 

      	update();
		//Sleep(10);
      	em = GetTickCount() - time0;
	}      

   // PART 4: rotating the whole clock
    while ((t = em) < 35000){
    	
        im.fillScreen(RGB565_Black);  
        drawClock( 150.0f*sinf((t-20000.0f) / 2000.0f), 1.0f);
        drawSmallHand(100 +360*sinf(t / 1500.0f), 0.292893 + fabsf(sinf( 0.7853981 + (t-10000.0f) / 3000.0f)));
        drawLongHand(500*cosf(t / 5000.0f), 0.292893 + fabsf(cosf( 0.7853981 + (t-10000.0f) / 3000.0f))); 

       	update();
       	//Sleep(10);
       	em = GetTickCount() - time0;
	}  

    // PART 5: fading
    while ((t = em) < 40000){
    	
        im.fillScreen(RGB565_Black);  
        drawClock( 150.0f*sinf((t-20000.0f) / 2000.0f), 1.0f - (t - 35000.0f)/5000.0f, 1.0f - (t - 35000.0f)/5000.0f);
        drawSmallHand(100 +360*sinf(t / 1500.0f), 0.292893 + fabsf(sinf( 0.7853981 + (t-10000.0f) / 3000.0f)), 1.0f - (t - 35000.0f)/5000.0f);
        drawLongHand(500*cosf(t / 5000.0f), 0.292893 + fabsf(cosf( 0.7853981 + (t-10000.0f) / 3000.0f)), 1.0f - (t - 35000.0f)/5000.0f); 
        
        update();
        //Sleep(10);
        em = GetTickCount() - time0;
	}  
     
	uint64_t tend = GetTickCount();
	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libTeensyRawHid_CloseDisplay(&ctx);
	
	return 1;
}
