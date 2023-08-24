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
#include <process.h>
#include "../../../../../../libHidDisplay/libHidDisplay.h"

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

// zbuffer in 16bits precision (150K in DMAMEM)
static DMAMEM uint16_t zbuf[SLX * SLY];              

// image that encapsulates the framebuffer fb.
static Image<RGB565> im(fb, SLX, SLY);
#else 
static uint16_t *fb;
static uint16_t *zbuf;
static Image<RGB565> im;
static uint64_t fcount;
#endif


// we only use Gouraud shading with perspective projection and a z-buffer
static const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD;

// the renderer object that performs the 3D drawings
static Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;


// DTCM and DMAMEM buffers used to cache meshes into RAM
// which is faster than progmem: caching may lead to significant speedup. 

static const int DTCM_buf_size = 100000; // adjust this value to fill unused DTCM but leave at least 10K for the stack to be sure
static char buf_DTCM[DTCM_buf_size];

static const int DMAMEM_buf_size = 100000; // adjust this value to fill unused DMAMEM,  leave at least 10k for additional serial objects. 
static DMAMEM char buf_DMAMEM[DMAMEM_buf_size];

// pointer to the cashed mesh.
static const Mesh3D<tgx::RGB565> * buddha_cached;



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

void update ()
{
	static uint64_t tstart = GetTickCount();
	
#if 0
	if (ctx.rgbClamp)
		libHidDisplay_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libHidDisplay_WriteImage(&ctx, fb);
#endif

	fcount = libHidDisplay_WriteImageAsync(&ctx, fb);
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
			
		if (libHidDisplay_OpenDisplay(&ctx))
			return 1;
	}
	return 0;
}

int display_init ()
{
	if (!libHidDisplay_OpenDisplay(&ctx)){
		if (!openDisplayWait(500))
			return 0;
	}

	rawhid_header_t desc;
	libHidDisplay_GetConfig(&ctx, &desc);
		
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
	zbuf = (uint16_t*)calloc(2, SLX * SLY);

	static Image<RGB565> _im(fb, SLX, SLY);
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
	double angle = 0.0;  // current angle


    while (1){

	    // erase the screen
	    im.fillScreen(RGB565_Black);
	    
	    // clear the z-buffer
	    renderer.clearZbuffer();

	    // position the model
	    renderer.setModelPosScaleRot({ 0, 0.5f, -35 }, { 13,13,13 }, angle);
	    
	    // draw the model onto the memory framebuffer
	    renderer.drawMesh(buddha_cached, false);

	    // update the screen
	    update();
  
	    // increase the angle by 3 degrees.
	    angle += 3.0;

	    // print some info about the video driver every 100 frames
	    if (fcount%60 == 0){
			if (kbhit()) break;
		}

		// limit rendering to around 50fps
		Sleep(20);
	}
	
	uint64_t tend = GetTickCount();
	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, (int)(tend - tstart), (int)fcount);
	
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
	

	if (fb) free(fb);
	if (zbuf) free(zbuf);
}


/** end of file */

