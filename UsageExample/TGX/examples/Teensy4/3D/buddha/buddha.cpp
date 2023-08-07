/********************************************************************
*
* tgx library example
* 
* Minimal example for displaying a mesh. 
* Happy buddha  (20 000 triangles) at 30FPS on an ILI9341 screen with 
* v-sync on (i.e. no screen tearing)
* 
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240)
*
********************************************************************/




#include <conio.h>
#include <windows.h>
#include <tgx.h> 
#include <stdio.h>
#include <string.h>
#include "../../../../../../libTeensyRawHid/libTeensyRawHid.h"

#include <Renderer3D.h>
#include <Renderer3D.cpp>

#include <Color.h>
#include <Color.cpp>

// let's not burden ourselves with the tgx:: prefix
using namespace tgx;

// the mesh to display
#include "3Dmodels/buddha/buddha.h"



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
static int SLX2 = DWIDTH/2;
static int SLY2 = DHEIGHT/2;

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


// we only use Gouraud shading with perspective projection and a z-buffer
const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD;

// the renderer object that performs the 3D drawings
Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;


// DTCM and DMAMEM buffers used to cache meshes into RAM
// which is faster than progmem: caching may lead to significant speedup. 

const int DTCM_buf_size = 200000; // adjust this value to fill unused DTCM but leave at least 10K for the stack to be sure
char buf_DTCM[DTCM_buf_size];

const int DMAMEM_buf_size = 170000; // adjust this value to fill unused DMAMEM,  leave at least 10k for additional serial objects. 
DMAMEM char buf_DMAMEM[DMAMEM_buf_size];

// pointer to the cashed mesh.
const Mesh3D<tgx::RGB565> * buddha_cached;


void setup()
{



    // setup the 3D renderer.
    renderer.setViewportSize(SLX,SLY); // viewport = screen 
    renderer.setOffset(0, 0); //  image = viewport
    renderer.setImage(&im); // set the image to draw onto (ie the screen framebuffer)
    renderer.setZbuffer(zbuf); // set the z buffer for depth testing
    renderer.setPerspective(45, ((float)SLX) / SLY, 1.0f, 100.0f);  // set the perspective projection matrix.     
    renderer.setMaterial(RGBf(0.85f, 0.55f, 0.25f), 0.2f, 0.7f, 0.8f, 64); // bronze color with a lot of specular reflexion. 
    renderer.setShaders(TGX_SHADER_GOURAUD);

    // cache the mesh in RAM. 
    buddha_cached = tgx::cacheMesh(&buddha, buf_DTCM, DTCM_buf_size,  buf_DMAMEM, DMAMEM_buf_size);

}

static uint64_t fcount = 0;

void update ()
{
	static uint64_t tstart = GetTickCount();
	
	if (ctx.rgbClamp)
		libTeensyRawHid_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libTeensyRawHid_WriteImage(&ctx, fb);
	
	//for (int y = 0; y < 480; y+=80)
		//libTeensyRawHid_WriteArea(&ctx, &fb[y*854], 0, y, ctx.width-1, y+80-1);
	
	
	fcount++;
	if (!(fcount%100)){
		uint64_t tend = GetTickCount();
		printf("%.1f fps\n", (float)fcount / (float)(tend - tstart)*1000.0f);
	}
}


static double a = 0.0;  // current angle
static double rt = 0.0; // sum of the mesh rendering times

static uint64_t time0 = GetTickCount();

                  

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
	ctx.rgbClamp = desc.u.cfg.rgbMax;
		
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

		
	setup();
	timeBeginPeriod(1);	
	uint64_t tstart = GetTickCount();
	time0 = GetTickCount();
	elapsedMicros em = 0;

    //while (!kbhit()){
    while (1){
	    // erase the screen
	    im.fillScreen(RGB565_Black);

	    // clear the z-buffer
	    renderer.clearZbuffer();

	    // position the model
	    renderer.setModelPosScaleRot({ 0, 0.5f, -35 }, { 13,13,13 }, a);

	    // draw the model onto the memory framebuffer

	    renderer.drawMesh(buddha_cached, false);
	    //rt += (1000.0 / ((double)em));

    
	    // update the screen (asynchronous). 
	    update();
  
	    // increase the angle by 3 degrees.
	    a += 3.0;

	    // print some info about the video driver every 100 frames
	    if (fcount%100 == 0){
			if (kbhit()) break;
		}
	
		em = GetTickCount() - time0;
	}
	
	uint64_t tend = GetTickCount();
	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	
	timeEndPeriod(1);
	libTeensyRawHid_CloseDisplay(&ctx);
}


/** end of file */

