/********************************************************************
*
* tgx library example : testing texture mapping.
*
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240)
*
********************************************************************/



#include <conio.h>
#include <windows.h>
#include <time.h>
#include <tgx.h> 
#include <stdio.h>
#include <string.h>
#include <libHidDisplay.h>


#include <Renderer3D.h>
#include <Renderer3D.cpp>
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>
#include <Color.h>
#include <Color.cpp>

using namespace tgx;


#include "3Dmodels/spot/spot.h"
#include "3Dmodels/bob/bob.h"
#include "3Dmodels/blub/blub.h"


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


// only load the shaders we need (note that TGX_SHADER_NOTEXTURE is needed in order to draw without texturing !). 
static const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD | TGX_SHADER_NOTEXTURE | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2;

// the renderer object that performs the 3D drawings
static Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;


// DTCM and DMAMEM buffers used to cache meshes into RAM
// which is faster than progmem: caching may lead to significant speedup. 

static const int DTCM_buf_size = 1000000; // adjust this value to fill unused DTCM but leave at least 20K for the stack to be sure
static char buf_DTCM[DTCM_buf_size];
static const int DMAMEM_buf_size = 3300000; // adjust this value to fill unused DMAMEM,  leave at least 10k for additional serial objects. 
static DMAMEM char buf_DMAMEM[DMAMEM_buf_size];
static const tgx::Mesh3D<tgx::RGB565> * cached_mesh; // pointer to the currently cached mesh. 


static uint64_t fcount = 0;
static elapsedMillis em; 
static float duration;
static uint64_t time0 = GetTickCount();




static void yield (uint32_t periodMs)
{
	Sleep(periodMs);
}

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
void drawInfo (tgx::Image<tgx::RGB565>& im, int shader, const tgx::Mesh3D<tgx::RGB565> & mesh)  // remark: need to keep the tgx:: prefix in function signatures because arduino messes with ino files....
{
    // count the number of triangles in the mesh (by iterating over linked meshes)
    const Mesh3D<RGB565> * m = &mesh;
    int nbt = 0;
    
    while (m != nullptr){
        nbt += m->nb_faces;
        m = m->next;
    }
    
    // display some info 
    char buf[80];
    im.drawText((mesh.name != nullptr ? mesh.name : "[unnamed mesh]"), { 3,26 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%d triangles", nbt);
    im.drawText(buf, { 3,SLY - 60}, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    sprintf(buf, "%s%s", (shader & TGX_SHADER_GOURAUD ? "Gouraud shading" : "flat shading"), (shader & TGX_SHADER_TEXTURE_BILINEAR ? " / texturing (bilinear)" : (shader & TGX_SHADER_TEXTURE_NEAREST ? " / texturing (nearest neighbour)" : "")));
    im.drawText(buf, { 3, SLY - 24}, RGB565_Red, font_tgx_OpenSans_Bold_32, false);
    
}
    

void setup()
{
	srand(time(NULL));

    // setup the 3D renderer.
    renderer.setViewportSize(SLX,SLY); // viewport = screen             
    renderer.setOffset(0, 0); //  image = viewport
    renderer.setImage(&im); // set the image to draw onto (ie the screen framebuffer)
    renderer.setZbuffer(zbuf); // set the z buffer for depth testing
    renderer.setPerspective(45, ((float)SLX) / SLY, 1.0f, 100.0f);  // set the perspective projection matrix. 
    renderer.setMaterial(RGBf(0.75f, 0.75f, 0.75f), 0.15f, 0.7f, 0.7f, 32);   
    renderer.setCulling(1);
    renderer.setTextureWrappingMode(TGX_SHADER_TEXTURE_WRAP_POW2);
}


void drawMesh (const Mesh3D<RGB565>* mesh, const float scale)
{
    // cache the first mesh to display in RAM to improve framerate
    cached_mesh  = tgx::cacheMesh(mesh, buf_DTCM, DTCM_buf_size,  buf_DMAMEM, DMAMEM_buf_size);
  
    const int maxT = 18000; // display model for 15 seconds. 
    
    time0 = GetTickCount();
    elapsedMillis em = 0;
    
    while (em < maxT && !kbhit()){
        // erase the screen
        im.fillScreen(RGB565_Black);

        // clear the z buffer
        renderer.clearZbuffer();

        // move the model to it correct position (depending on the current time). 
        fMat4 M;
        M.setScale(scale, scale, scale);
        M.multRotate((1440.0f * em) / maxT, { 0,1,0 }); // 4 rotations per display
        M.multRotate((800.0f * em) / maxT, { 1, 0, 0}); 
        M.multTranslate({ 0,0, -40 });
        renderer.setModelMatrix(M);

        // change shader type after every turn
        int part = (em * 3) / maxT;
 
        // choose the shader to use
        int shader = (part == 0) ? TGX_SHADER_GOURAUD : ((part == 1) ? (TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST) : (TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_BILINEAR));

        renderer.setShaders(shader);

        // draw the mesh on the image
        renderer.drawMesh(cached_mesh, false);
        
        // overlay some info 
        drawInfo(im, shader, *mesh);

        update();
        
        em = GetTickCount() - time0;

		// limit fps to 50
		yield(20);
	}
}


void run ()
{
    // random light orientation.
    const float angle = M_PI * (float)(rand()%360) / 180.0f;
    
    renderer.setLightDirection({cosf(angle) ,sinf(angle) ,-0.3f});
        
    drawMesh(&spot, 13.0f);
    drawMesh(&bob,  15.0f);
    drawMesh(&blub, 15.0f);
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
	
	printf("Display: %s\nWxH: %ix%i\n", (char*)DEVNAME, SLX, SLY);
	
	setup();
	timeBeginPeriod(1);	
	uint64_t tstart = GetTickCount();

	while(!kbhit()){
  		run();
  	}
	
	uint64_t tend = GetTickCount();

	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
}
  


/** end of file */

