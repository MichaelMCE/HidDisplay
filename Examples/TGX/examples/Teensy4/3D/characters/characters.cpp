/********************************************************************
*
* tgx library example : displaying some nice meshes...
*
* EXAMPLE FOR TEENSY 4 / 4.1 (TEENSY 4.1 HAS ADDITIONNAL MODELS)
*
* DISPLAY: ILI9341 (320x240)
*
********************************************************************/



// This example runs on teensy 4.0/4.1 with ILI9341 via SPI. 
// the screen driver library : https://github.com/vindar/ILI9341_T4
#include <conio.h>
#include <windows.h>
#include <tgx.h> 
#include <stdio.h>
#include <string.h>
#include "../../../../../../libHidDisplay/libHidDisplay.h"


#include <Renderer3D.h>
#include <Renderer3D.cpp>
// font we use
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>

#include <Color.h>
#include <Color.cpp>

// let's not burden ourselves with the tgx:: prefix
using namespace tgx;


#include "3Dmodels/nanosuit/nanosuit.h"
#include "3Dmodels/R2D2/R2D2.h"
#include "3Dmodels/elementalist/elementalist.h"
#include "3Dmodels/sinbad/sinbad.h"
#include "3Dmodels/cyborg/cyborg.h"
#include "3Dmodels/dennis/dennis.h"
#include "3Dmodels/manga3/manga3.h"
#include "3Dmodels/naruto/naruto.h"
#include "3Dmodels/stormtrooper/stormtrooper.h"



#define FLASHMEM  
#define DMAMEM  
#define elapsedMillis 	uint64_t
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


// we only use nearest neighbour texturing for power of 2 textures, combined texturing with gouraud shading, a z buffer and perspective projection
const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST |TGX_SHADER_TEXTURE_WRAP_POW2;

// the renderer object that performs the 3D drawings
Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;


// shaders to use
const int shader = TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2;


const Mesh3D<RGB565>* meshes[9] = {&nanosuit_1, &elementalist_1, &sinbad_1, &cyborg, &naruto_1, &manga3_1, &dennis, &R2D2, &stormtrooper };


// DTCM and DMAMEM buffers used to cache meshes into RAM
// which is faster than progmem: caching may lead to significant speedup. 

const int DTCM_buf_size = 40*35000; // adjust this value to fill unused DTCM but leave at least 20K for the stack to be sure
char buf_DTCM[DTCM_buf_size];

const int DMAMEM_buf_size = 8*330000; // adjust this value to fill unused DMAMEM,  leave at least 10k for additional serial objects. 
DMAMEM char buf_DMAMEM[DMAMEM_buf_size];

const tgx::Mesh3D<tgx::RGB565> * cached_mesh; // pointer to the currently cached mesh. 


static uint64_t fcount = 0;

void update ()
{
	static uint64_t tstart = GetTickCount();
		
	/*if (ctx.rgbClamp)
		libHidDisplay_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libHidDisplay_WriteImage(&ctx, fb);*/
	
	
	/*libHidDisplay_WriteArea(&ctx, fb, 0, 0, ctx.width-1, (ctx.height>>1)-1);
	uint8_t *halfpixels = (uint8_t*)fb + ((ctx.height>>1) * ctx.width * 2);
	libHidDisplay_WriteArea(&ctx, halfpixels, 0, (ctx.height>>1), ctx.width-1, ctx.height-1);*/
	
	
	/*const int strips = 2;
	const int stripHeight = DHEIGHT/strips;

	for (int i = 0; i < strips; i++){
		uint8_t *pixels = (uint8_t*)fb + ((i*stripHeight) * ctx.width * 2);
		libHidDisplay_WriteArea(&ctx, pixels, 0, i*stripHeight, ctx.width-1, (i*stripHeight)+stripHeight-1);
	}*/


	fcount = libHidDisplay_WriteImageAsync(&ctx, fb);

	//fcount++;
	if (!(fcount%100)){
		uint64_t tend = GetTickCount();
		printf("%.2f fps\n", (float)fcount / (float)(tend - tstart)*1000.0f);
	}
}


static uint64_t time0 = GetTickCount();

/**
* This function computes the object position
* according to the current time.
* Return true when it is time to change model.
**/
bool  moveModel () // remark: need to keep the tgx:: prefix in function signatures because arduino messes with ino files....
{
    static elapsedMillis em = 0; // timer
    
    const float end1 = 6000;
    const float end2 = 2000;
    const float end3 = 6000;
    const float end4 = 2000;
    const float tot = end1 + end2 + end3 + end4;

    bool change = false;
    
	em = GetTickCount() - time0;
    
    // check if it is time to change the mesh. 
    while (em > tot){
    	em -= tot;
    	change = true;
    } 

	//em = GetTickCount() - time0;
    float t = em; // current time
    const float dilat = 9; // scale model
    const float roty = 360.0f * (t / 4000.0f); // rotate 1 turn every 4 seconds        
    float tz, ty;
    
    if (t < end1)
        { // far away
        tz = -25;
        ty = 0;
        }
    else
        {
        t -= end1;
        if (t < end2)
            { // zooming in
            t /= end2;
            tz = -25 + 15 * t;
            ty = -6 * t;
            }
        else
            {
            t -= end2;
            if (t < end3)
                { // close up
                tz = -10;
                ty = -6;
                }
            else
                { // zooming out
                t -= end3;
                t /= end4;
                tz = -10 - 15 * t;
                ty = -6 + 6 * t;
                }
            }
        }
    fMat4 M;

    M.setScale({ dilat, dilat, dilat }); // scale the model
    M.multRotate(-roty, { 0,1,0 }); // rotate around y
    M.multTranslate({ 0,ty, tz }); // translate
    renderer.setModelMatrix(M);
    
    return change;
}



/**
* Overlay some info about the current mesh on the screen
**/
void drawInfo(tgx::Image<tgx::RGB565>& im, int shader, const tgx::Mesh3D<tgx::RGB565>* mesh)  // remark: need to keep the tgx:: prefix in function signatures because arduino messes with ino files....
    {
    // count the number of triangles in the mesh (by iterating over linked meshes)
    const Mesh3D<RGB565>* m = mesh;
    int nbt = 0;
    
    while (m != nullptr){
        nbt += m->nb_faces;
        m = m->next;
    }
    
    // display some info 
    char buf[80];
    im.drawText((mesh->name != nullptr ? mesh->name : "[unnamed mesh]"), { 3, 24 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%d triangles", nbt);
    im.drawText(buf, { 3,SLY - 70 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%s%s", (shader & TGX_SHADER_GOURAUD ? "Gouraud shading" : "flat shading"), (shader & TGX_SHADER_TEXTURE_NEAREST ? " / texturing" : ""));
    im.drawText(buf, { 3, SLY - 28 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
}




// index of the mesh currently displayed
int meshindex = 0;

// number of frame drawn
int nbf = 0;


void setup()
    {


    // setup the 3D renderer.
    renderer.setViewportSize(SLX,SLY); // viewport = screen 
    renderer.setOffset(0, 0); //  image = viewport
    renderer.setImage(&im); // set the image to draw onto (ie the screen framebuffer)
    renderer.setZbuffer(zbuf); // set the z buffer for depth testing
    renderer.setPerspective(45, ((float)SLX) / SLY, 1.0f, 100.0f);  // set the perspective projection matrix. 
    renderer.setMaterial(RGBf(0.85f, 0.55f, 0.25f), 0.2f, 0.7f, 0.8f, 64); // bronze color with a lot of specular reflexion. 
    renderer.setCulling(1);
    renderer.setShaders(shader);


    // cache the first mesh to display in RAM to improve framerate
    cached_mesh  = tgx::cacheMesh(meshes[meshindex], buf_DTCM, DTCM_buf_size,  buf_DMAMEM, DMAMEM_buf_size);

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

    while (1){
    	// erase the screen
    	im.fillScreen(RGB565_Black);

   		 // clear the z buffer
    	renderer.clearZbuffer();


    	// move the model to it correct position (depending on current time). 
    	if (moveModel()) {
        	// selct next mesh
       		meshindex = (meshindex + 1) % (sizeof(meshes) / sizeof(meshes[0]));

        	// cache it in RAM to improve framerate
        	cached_mesh  = tgx::cacheMesh(meshes[meshindex], buf_DTCM, DTCM_buf_size,  buf_DMAMEM, DMAMEM_buf_size);
        	time0 = GetTickCount();
        }


    	// draw the mesh on the image
   		renderer.drawMesh(cached_mesh);

    	// overlay some info 
    	drawInfo(im, shader, meshes[meshindex]);

    
    	// update the screen
   		update();
   		
	    if (fcount%100 == 0){
			if (kbhit()) break;
		}
		
		Sleep(20);
	}
	
	uint64_t tend = GetTickCount();
	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
}


/** end of file */

