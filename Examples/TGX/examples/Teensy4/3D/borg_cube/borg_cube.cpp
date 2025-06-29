/********************************************************************
*
* tgx library example.
*
* The Borg cube !!! ... Well, just a stupid textured cube in fact...
* - textured is drawn onto in real time
* - switch between ortoscopic and perspective projection
* 
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240)
*
********************************************************************/


// This example runs on teensy 4.0/4.1 with ILI9341 via SPI. 
// the screen driver library : https://github.com/vindar/ILI9341_T4

#include <conio.h>
#include <time.h>
#include <windows.h>
#include <tgx.h> 
#include <stdio.h>
#include <string.h>
#include <libHidDisplay.h>

#include <Renderer3D.h>
#include <Renderer3D.cpp>

#include <Color.h>
#include <Color.cpp>

#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>


// let's not burden ourselves with the tgx:: prefix
using namespace tgx;





#define FLASHMEM  
#define DMAMEM  
#define elapsedMillis uint64_t
#define elapsedMicros uint64_t
static teensyRawHidcxt_t ctx;


static char DEVNAME[64];
static int DWIDTH = 0;
static int DHEIGHT = 0;
static int SLX = DWIDTH;
static int SLY = DHEIGHT;
static int SLX2 = DWIDTH/2;
static int SLY2 = DHEIGHT/2;

float ratio = ((float)SLX) / SLY; // aspect ratio


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
#endif


// the texture image
static const int tex_size = 128;
static RGB565 texture_data[tex_size*tex_size];
static Image<RGB565> texture(texture_data, tex_size, tex_size);


// we only use bilinear texturing for power of 2 texture, combined texturing with flat shading, a z buffer and both perspective and orthographic projection
static const int LOADED_SHADERS = TGX_SHADER_ORTHO | TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_FLAT | TGX_SHADER_TEXTURE_BILINEAR |TGX_SHADER_TEXTURE_WRAP_POW2;

// the renderer object that performs the 3D drawings
static Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;


void setup ()
{

    // setup the 3D renderer with perspective projection
    renderer.setViewportSize(SLX,SLY);
    renderer.setOffset(0, 0);
    renderer.setImage(&im);
    renderer.setZbuffer(zbuf);
    renderer.setCulling(1);
    renderer.setTextureQuality(TGX_SHADER_TEXTURE_BILINEAR);
    renderer.setTextureWrappingMode(TGX_SHADER_TEXTURE_WRAP_POW2);
    renderer.setShaders(TGX_SHADER_FLAT | TGX_SHADER_TEXTURE );
    renderer.setPerspective(45, ratio, 1.0f, 100.0f);
    
    // initial texture color
    texture.fillScreen(RGB565_Black);
    
    srand(time(NULL));
}

             

/** draw a random rectangle on the texture */
void splash ()
{
    static int count = 0;    
    static RGB565 color;
    if (count == 0)
        color = RGB565((int)rand()%32, (int)rand()%64, (int)rand()%32); 
    count = (count + 1) % 400;
    iVec2 pos(rand()%tex_size, rand()%tex_size);
    int r = rand()%10;
    
    texture.drawRect(iBox2( pos.x - r, pos.x + r, pos.y - r, pos.y + r ), color);
}


static elapsedMillis em = 0;	// time
static int nbf = 0;				// number frames drawn
static int projtype = 0;		// current projection used. 
static uint64_t fcount = 0;

void update ()
{
	static uint64_t tstart = GetTickCount();
	
	/*if (ctx.rgbClamp)
		libHidDisplay_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libHidDisplay_WriteImage(&ctx, fb);*/
	
	for (int y = 0; y < ctx.height; y+=32)
		libHidDisplay_WriteArea(&ctx, &fb[y*ctx.width], 0, y, ctx.width-1, y+32-1);
	
	
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

	ratio = ((float)SLX) / SLY; // aspect ratio

	fb = (uint16_t*)calloc(2, SLX * SLY);
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
	time0 = GetTickCount();
	elapsedMicros em = 0;
	
	while(1){
    // model matrix
    fMat4 M;
    M.setRotate(em / 11.0f, { 0,1,0 });
    M.multRotate(em / 23.0f, { 1,0,0 });
    M.multRotate(em / 41.0f, { 0,0,1 });
    M.multTranslate({ 0, 0, -5 });

    im.fillScreen((projtype) ? RGB565_Black : RGB565_Gray); // erase the screen, black in perspective and grey in orthographic projection
    renderer.clearZbuffer(); // clear the z buffer        
    renderer.setModelMatrix(M);// position the model

    renderer.drawCube(&texture, & texture, & texture, & texture, & texture, & texture); // draw the textured cube


    // info about the projection type
    im.drawText((projtype) ? "Perspective projection" : "Orthographic projection", {3,50 }, RGB565_Red, font_tgx_OpenSans_Bold_32, false);

    
    // update the screen (async). 
    update();

    // add a random rect on the texture.
    splash();

    // switch between perspective and orthogonal projection every 1000 frames.
    if (nbf++ % 100 == 0)
        {
        projtype = 1 - projtype;

        if (projtype)
            renderer.setPerspective(45, ratio, 1.0f, 100.0f);
        else
            renderer.setOrtho(-1.8 * ratio, 1.8 * ratio, -1.8, 1.8, 1.0f, 100.0f);
        }
    }
 }      

/** end of file */
