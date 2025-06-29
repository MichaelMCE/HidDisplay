/********************************************************************
*
* tgx library example : comparing flat vs Gouraud shading.
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
#include <libHidDisplay.h>



using namespace tgx;

#include <Renderer3D.h>
#include <Renderer3D.cpp>
// font we use
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>
#include <Color.h>
#include <Color.cpp>


// meshes (stored in PROGMEM)
#include "3Dmodels/teapot/teapot.h"
#include "3Dmodels/skull/skull.h"
#include "3Dmodels/suzanne/suzanne.h"
#include "3Dmodels/bunny/bunny.h"
#include "3Dmodels/dragon/dragon.h"



#define FLASHMEM  
#define DMAMEM  
#define elapsedMillis 	uint64_t

static teensyRawHidcxt_t ctx;
static char DEVNAME[64];
static int DWIDTH = 0;
static int DHEIGHT = 0;
static int SLX = DWIDTH;
static int SLY = DHEIGHT;
static int SLX2 = DWIDTH/2;
static int SLY2 = DHEIGHT/2;

#if 0
static uint16_t fb[SLX * SLY];

// internal framebuffer (150K) used by the ILI9431_T4 library for double buffering.
//static uint16_t internal_fb[SLX * SLY]; 

// zbuffer in 16bits precision (150K in DMAMEM)
static DMAMEM uint16_t zbuf[SLX * SLY];              

// image that encapsulates the framebuffer fb.
static Image<RGB565> im(fb, SLX, SLY);
#else 
static uint16_t *fb;
//static uint16_t *internal_fb; 
static uint16_t *zbuf;
static Image<RGB565> im;
#endif


// only load the shaders we need.
static const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_FLAT | TGX_SHADER_GOURAUD;

// the renderer object that performs the 3D drawings
static Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;



// DTCM and DMAMEM buffers used to cache meshes into RAM
// which is faster than progmem: caching may lead to significant speedup. 

static const int DTCM_buf_size = 20*160000; // adjust this value to fill unused DTCM but leave at least 20K for the stack to be sure
static char buf_DTCM[DTCM_buf_size];

static const int DMAMEM_buf_size = 20*190000; // adjust this value to fill unused DMAMEM,  leave at least 10k for additional serial objects. 
static DMAMEM char buf_DMAMEM[DMAMEM_buf_size];

static const tgx::Mesh3D<tgx::RGB565> * cached_mesh; // pointer to the currently cached mesh. 

static uint64_t time0 = GetTickCount();
static uint64_t fcount = 0;


void update ()
{
	static uint64_t tstart = GetTickCount();

#if 0
	if (ctx.rgbClamp)
		libHidDisplay_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libHidDisplay_WriteImage(&ctx, fb);

	fcount++;
#else
	fcount = libHidDisplay_WriteImageAsync(&ctx, fb);
#endif

	if (!(fcount%100)){
		uint64_t tend = GetTickCount();
		printf("%.1f fps\n", (float)fcount / (float)(tend - tstart)*1000.0f);
	}
}


/**
* Overlay some info about the current mesh on the screen
**/
void drawInfo (tgx::Image<tgx::RGB565>& im, int t, const tgx::Mesh3D<tgx::RGB565>& mesh)  // remark: need to keep the tgx:: prefix in function signatures because arduino messes with ino files....
{
    // count the number of triangles in the mesh (by iterating over linked meshes)
    const Mesh3D<RGB565>* m = &mesh;
    int nbt = 0;
    
    while (m != nullptr){
        nbt += m->nb_faces;
        m = m->next;
    }
    
    // display some info 
    char buf[80];
    im.drawText((mesh.name != nullptr ? mesh.name : "[unnamed mesh]"), { 3, 24 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%d triangles", nbt);
    im.drawText(buf, { 3,SLY - 60 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%s", (t == 0) ? "Wireframe" : ((t == 1) ? "Flat shading" : "Gouraud shading"));
    im.drawText(buf, { 3, SLY - 26 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
}



void setup ()
{
    // setup the 3D renderer.
    renderer.setViewportSize(SLX,SLY); // viewport = screen         
    renderer.setOffset(0, 0); //  image = viewport
    renderer.setImage(&im); // set the image to draw onto (ie the screen framebuffer)
    renderer.setZbuffer(zbuf); // set the z buffer for depth testing
    renderer.setPerspective(45, ((float)SLX) / SLY, 1.0f, 100.0f);  // set the perspective projection matrix. 
    renderer.setCulling(1);
}



void drawMesh (const Mesh3D<RGB565>* mesh, float scale, float tilt = 0.0f)
{
    // cache the first mesh to display in RAM to improve framerate
    cached_mesh  = tgx::cacheMesh(mesh, buf_DTCM, DTCM_buf_size,  buf_DMAMEM, DMAMEM_buf_size);
    
    const int maxT = 12000; // display model for 12 seconds. 
    elapsedMillis em = 0;
    time0 = GetTickCount();
    
    
    while (em < maxT && !kbhit()){
        // erase the screen
        im.fillScreen(RGB565_Black);

        // clear the z buffer
        renderer.clearZbuffer();

        // move the model to it correct position (depending on the current time). 
        fMat4 M;
        M.setScale(scale, scale, scale);
        M.multRotate(tilt, { 0,0,1 }); // 4 rotations per display
        M.multRotate((1440.0f * em) / maxT, { 0,1,0 }); // 4 rotations per display
        M.multTranslate({ 0,0, -40 });
        renderer.setModelMatrix(M);

        // change shader type after every turn        
        int t = (((em * 3) / maxT) % 3);

        if (t == 0){
            renderer.drawWireFrameMesh(cached_mesh);
        }else if (t == 1){
            renderer.setShaders(TGX_SHADER_FLAT);
            renderer.drawMesh(cached_mesh, false);
		}else{
            renderer.setShaders(TGX_SHADER_GOURAUD);
            renderer.drawMesh(cached_mesh, false);
		}

        // overlay some info 
        drawInfo(im, t, *cached_mesh);
        
        // update the screen (asynchronously)
        update();
        em = GetTickCount() - time0;
        
        Sleep(15);
	}
}


void run ()
{
	
    renderer.setMaterial(RGBf(0.15f, 0.7f, 0.39f), 0.2f, 0.8f, 0.5f, 8); // teapot
    drawMesh(&teapot, 15, 30);

    renderer.setMaterial(RGBf(1.0f, 1.0f, 1.0f), 0.15f, 0.7f, 0.8f, 48); // bunny
    drawMesh(&bunny, 12);

    renderer.setMaterial(RGBf(166 / 256.0f, 130 / 256.0f, 110.0f / 256.0f), 0.15f, 0.7f, 0.4f, 16); // skull
    drawMesh(&skull_1, 12);

    // let's have some fun with lightning
    renderer.setLightAmbiant({ 0, 0, 1.0f });  // blue
    renderer.setLightDiffuse({ 1.0f, 0, 0 });  // red    
    renderer.setLightSpecular({ 1.0f, 1.0f, 1.0f }); // white

    renderer.setMaterial(RGBf(1.0f, 1.0f, 1.0f), 0.2f, 0.8f, 0.8f, 32); // suzanne
    drawMesh(&suzanne, 13);

    // back to normal lightning
    renderer.setLightAmbiant({ 1.0f, 1.0f, 1.0f }); // white
    renderer.setLightDiffuse({ 1.0f, 1.0f, 1.0f }); // white
    renderer.setLightSpecular({ 1.0f, 1.0f, 1.0f }); // white

    renderer.setMaterial(RGBf(0.85f, 0.55f, 0.25f), 0.2f, 0.7f, 0.8f, 64); // dragon
    drawMesh(&dragon, 15);

    // chooose new random light orientation.
    const float angle = M_PI * (float)(rand()%360) / 180.0f;
    renderer.setLightDirection({ cosf(angle) , sinf(angle) , -0.3f });
    
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

	rawhid_header_t desc;
	libHidDisplay_GetConfig(&ctx, &desc);
		
	ctx.width = desc.u.cfg.width;
	ctx.height = desc.u.cfg.height;
	ctx.pitch = desc.u.cfg.pitch;
	ctx.rgbClamp = desc.u.cfg.rgbMax;
	
	strncpy(DEVNAME, (char*)(&desc.u.cfg.string[0]), 32);
	DWIDTH = ctx.width;
	DHEIGHT = ctx.height;
	
	SLX = DWIDTH;
	SLY = DHEIGHT;
	SLX2 = DWIDTH/2;
	SLY2 = DHEIGHT/2;

	fb = (uint16_t*)calloc(2, SLX * SLY);
	//internal_fb = (uint16_t*)calloc(2, SLX * SLY);
	zbuf = (uint16_t*)calloc(2, SLX * SLY);


	static Image<RGB565> _im(fb, SLX, SLY);;
	memcpy((void*)&im, (void*)&_im, sizeof(_im));

	return 1;
}

int main ()
{

	if (!display_init())
		return 0;
	
	printf("Display: %s\nWxH: %ix%i\n", (char*)DEVNAME, SLX, SLY);

	
	setup();
	timeBeginPeriod(1);	
	uint64_t tstart = GetTickCount();
	
    while (!kbhit()){
    	run();
    }

   	uint64_t tend = GetTickCount();
	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
    
}




