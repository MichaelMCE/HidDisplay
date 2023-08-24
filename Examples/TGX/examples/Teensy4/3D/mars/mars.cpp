/********************************************************************
*
* tgx library example.
*
* Mars demo : demonstrate how to use a skybox. 
* 
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240) with ILI9341_T4 driver. 
*
* REMARKS: THe demo is very computationaly intensive. For best results:
* 
* 1. Use a very SPI speed for the ILI9341 speed: with short wires, most
*    OLI9341 controller can work with speed above 70Mhz...
*  
* 2. Compile with "fastest" option (all optimization enabled). 
*
* 3. (Optional) Compile the code for Teensy4@720Mhz : smoother result :-)  
* 
********************************************************************/


// This example runs on teensy 4.0/4.1 with ILI9341 via SPI. 
// the screen driver library : https://github.com/vindar/ILI9341_T4


#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tgx.h> 
#include <stdio.h>
#include <string.h>
#include "../../../../../../libHidDisplay/libHidDisplay.h"


using namespace tgx;

#include <Renderer3D.h>
#include <Renderer3D.cpp>
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>
#include <Color.h>
#include <Color.cpp>

// assets (image and 3D models)
#include "earth_small.h"
#include "marble.h"
#include "mars_front.h"
#include "mars_back.h"
#include "mars_left.h"
#include "mars_right.h"
#include "mars_bottom.h"
#include "mars_top_neb.h"
#include "falcon/falcon_vs.h"


//#include "earth_small.cpp"
#include "marble.cpp"
#include "mars_front.cpp"
#include "mars_back.cpp"
#include "mars_left.cpp"
#include "mars_right.cpp"
#include "mars_bottom.cpp"
#include "mars_top_neb.cpp"



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

static uint64_t fcount = 0;
#endif

// all the shaders we will use with the renderer
static const int LOADED_SHADER = TGX_SHADER_ZBUFFER | TGX_SHADER_PERSPECTIVE | TGX_SHADER_GOURAUD | TGX_SHADER_FLAT | TGX_SHADER_NOTEXTURE |TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_WRAP_POW2;

// the 3D renderer object.
static Renderer3D<RGB565, LOADED_SHADER, uint16_t> renderer;

// additional memory in DMAMEM (175Kb)
// used to load temporary objects. 
static int dma_buf[512 * 512]; // 260kb in DMAMEM


// marble image texture in dma memory
static tgx::Image<tgx::RGB565> marble_dma;




/*************************************************************
*
* VERY DIRTY CODE BELOW: CLEANUP NEEDED....
*
**************************************************************/


TGX_NOINLINE void setup ()
{    
  /*  Serial.begin(9600);

    // output debug infos to serial port. 
    tft.output(&Serial);                
    
    // initialize the ILI9341 screen
    while (!tft.begin(SPI_SPEED));

    // ok. turn on backlight
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, HIGH);

    // setup the screen driver 
    tft.setRotation(3); // portrait mode
    tft.setFramebuffer(internal_fb); // double buffering
    tft.setDiffBuffers(&diff1, &diff2); // 2 diff buffers
    tft.setDiffGap(5); // small gap
    tft.setRefreshRate(85); // refresh at 85hz
    tft.setVSyncSpacing(2); // lock the framerate at 85/2 = 42fps. 
    tft.setLateStartRatio(0.7f);
*/
    const float ratio = ((float)SLX) / SLY;

    // setup the 3D renderer with perspective projection
    renderer.setViewportSize(SLX, SLY);
    renderer.setOffset(0, 0);
    renderer.setImage(&im);
    renderer.setZbuffer(zbuf);
    renderer.setPerspective(50, ratio, 10.0f, 8000.0f);
    renderer.setTextureWrappingMode(TGX_SHADER_TEXTURE_WRAP_POW2);

    // set the lighning direction (to match the sun position in the skybox). 
    tgx::fVec3 lightdir(-0.40f, -0.30f, 1.0f);
    renderer.setLightDirection(lightdir);
    renderer.setLightAmbiant(tgx::RGBf(1.0, 1.0, 1.0));

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
	
	// limit framerate
	Sleep(20);
}


/** 
* Redraw screen and overlay the current framerate 
**/
TGX_NOINLINE FLASHMEM void redraw (bool full_redraw = false)
{
    // add fps counter (on top right, color white on semi-transparent black background)
    //tft.overlayFPS(fb, 0, 0xFFFF, 0, 0.3f); 

    // update the screen (async). 
    update();
}


/*****************************************
*
*  Animation helper functions
*
******************************************/



static elapsedMillis em; 
static float duration;
static uint64_t time0 = GetTickCount();

static void yield ()
{
	//Sleep(10);
}



/**
* Start an animation with a given lenght (in seconds)
**/
void start (float duration_sec)
{
    em = 0;
    time0 = GetTickCount();
    duration = (int)(duration_sec * 1000);
}


/**
* Return the current time fror the animation (in [0,1]). 
**/
float time ()
{
    const float t = ((uint64_t)em) / (float)duration;
    return ((t < 1) ? t : 1.0f);
}


/**
* Return true while the animation is ongoing. 
**/
bool ongoing()
{
    return (time() < 1.0f);
}

/**
* Mapping for smooth animation.
**/
float slide (float t, const float t_start = 0.0f, const float t_end = 1.0f)
{
    if (t < t_start) return 0.0f;
    if (t > t_end) return 1.0f;
    const float s = (t - t_start) / (t_end - t_start);

    return (1.0f - cosf(M_PI * s))/ 2.0;
}


const float skybox_size = 4000;
const float skybox_ref_height = 0;


TGX_NOINLINE FLASHMEM void drawSkyBox()
{
    renderer.setCulling(0);
    renderer.setMaterialAmbiantStrength(1.8f);
    renderer.setMaterialDiffuseStrength(0.0f);
    renderer.setMaterialSpecularStrength(0.0f);
    renderer.setModelPosScaleRot({ 0,skybox_ref_height,0 }, { skybox_size,skybox_size,skybox_size }, 180 , { 0,1,0 } );
    renderer.setShaders(TGX_SHADER_FLAT | TGX_SHADER_TEXTURE_NEAREST);
    renderer.drawCube(&mars_front, &mars_back, &mars_top_neb, &mars_bottom, &mars_left, &mars_right);
}


TGX_NOINLINE FLASHMEM void drawBase(bool use_dma_tex = false)
{
    renderer.setCulling(1);    
    renderer.setMaterialSpecularExponent(6);
    renderer.setMaterialAmbiantStrength(0.2f);
    renderer.setMaterialDiffuseStrength(0.9f);
    renderer.setMaterialSpecularStrength(0.4f);
    renderer.setShaders(TGX_SHADER_FLAT | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_WRAP_POW2);

    const float aa = -0.05;
    const float ee = 0.01;
    
    const tgx::fVec2 v_front_ABCD[4] = { tgx::fVec2(0.2f + aa, 0.8f - aa),tgx::fVec2(0.2f + aa, 1.0f - ee),tgx::fVec2(0.8f - aa, 1.0f - ee),tgx::fVec2(0.8f - aa, 0.8f - aa) };
    const tgx::fVec2 v_back_EFGH[4] =  { tgx::fVec2(0.8f - aa, 0.2f + aa),tgx::fVec2(0.8f - aa, 0.0f + ee),tgx::fVec2(0.2f + aa, 0.0f + ee),tgx::fVec2(0.2f + aa, 0.2f + aa) };
    const tgx::fVec2 v_top_HADE[4] =   { tgx::fVec2(0.2f + aa, 0.2f + aa),tgx::fVec2(0.2f + aa, 0.8f - aa),tgx::fVec2(0.8f - aa, 0.8f - aa),tgx::fVec2(0.8f - aa, 0.2f + aa) };
    const tgx::fVec2 v_bottom_BGFC[4] = {tgx::fVec2(0.2f + aa, 0.2f + aa),tgx::fVec2(0.2f + aa, 0.8f - aa),tgx::fVec2(0.8f - aa, 0.8f - aa),tgx::fVec2(0.8f - aa, 0.2f + aa) };
    const tgx::fVec2 v_left_HGBA[4] = {  tgx::fVec2(0.2f + aa, 0.2f + aa),tgx::fVec2(0.0f + ee, 0.2f + aa),tgx::fVec2(0.0f + ee, 0.8f - aa),tgx::fVec2(0.2f + aa, 0.8f - aa) };
    const tgx::fVec2 v_right_DCFE[4] = { tgx::fVec2(0.8f - aa, 0.8f - aa),tgx::fVec2(1.0f - ee, 0.8f - aa),tgx::fVec2(1.0f - ee, 0.2f + aa),tgx::fVec2(0.8f - aa, 0.2f + aa) };
    
    renderer.drawCube(
        v_front_ABCD, & marble,
        v_back_EFGH, & marble,
        v_top_HADE, use_dma_tex ? (&marble_dma) : (&marble),
        v_bottom_BGFC, & marble,
        v_left_HGBA, & marble,
        v_right_DCFE, & marble);
}


TGX_NOINLINE void drawSphere(const int shader, const Image<RGB565> * texture = nullptr)
{
     renderer.setCulling(1);     
    if (texture){
        renderer.setShaders(shader);
        renderer.drawAdaptativeSphere(texture, 3.0f);
        
    }else{
        renderer.setShaders(shader);
        renderer.drawAdaptativeSphere(3.0f);
    }
}


TGX_NOINLINE void setModelPosScaleRot(const fVec3& center = fVec3{ 0,0,0 }, const fVec3& scale = fVec3(1, 1, 1), float rot_angle = 0, const fVec3& rot_dir = fVec3{ 0,1,0 })
{
     renderer.setModelPosScaleRot(center, scale, rot_angle, rot_dir);
}

void loadMarbleTexture ()
{
	memset(dma_buf, 0, sizeof(dma_buf));
	
    memcpy(dma_buf, marble.data(), 128 * 128 * 2);
    marble_dma.set(dma_buf, iVec2(128, 128), 128);
}

void loadEarthTexture ()
{
	memset(dma_buf, 0, sizeof(dma_buf));
	
    memcpy(dma_buf, earth_small_data, 256 * 256 * 2);
    earth_small.set(dma_buf, iVec2(256, 256), 256);
}


void loadFalconTexture ()
{
	memset(dma_buf, 0, sizeof(dma_buf));
	
    memcpy(dma_buf, FalcPlan_texture_data, 256*256*2);
    FalcPlan_texture.set(dma_buf, iVec2(256, 256), 256); // 128kb

    tgx::fVec3* va = (tgx::fVec3*)(dma_buf + (128 * 256));
	memcpy(va, falcon_vs_vert_array, sizeof(falcon_vs_vert_array));

    tgx::fVec3* vt = (tgx::fVec3*)(dma_buf + (128 * 256) + (sizeof(falcon_vs_vert_array) + 3)/4);
    memcpy(vt, falcon_vs_tex_array, sizeof(falcon_vs_tex_array));

    tgx::fVec3* vn = (tgx::fVec3*)(dma_buf + (128 * 256) + (sizeof(falcon_vs_vert_array) + 3)/4 + (sizeof(falcon_vs_tex_array) + 3)/4);
    memcpy(vn, falcon_vs_norm_array, sizeof(falcon_vs_norm_array));
    
    tgx::Mesh3D<tgx::RGB565> *mesh = &falcon_vs_1;


    while (mesh != nullptr){

        mesh->vertice = va;
        mesh->texcoord = vt;
        mesh->normal = vn;

        mesh = mesh->next;
	}
}

TGX_NOINLINE FLASHMEM void movie ()
{
    //tft.setVSyncSpacing(2);

    // look around

    start(12); // (15);

    while (ongoing() && !kbhit()){
        renderer.clearZbuffer();
        const float t = time();
        const float a = slide(t, 0.1f, 1.0f) * 2 * M_PI;        
        renderer.setLookAt({ 0, 0, 0 },  { sinf(a), 0, cosf(a) }, { 0,1,0 });
        drawSkyBox();
        redraw();
        
        em = GetTickCount() - time0;
    }
    // ends at (0,0,0), looking toward (0,0,-1)
    
    // look up

    start(4); 
    while (ongoing())
        {
        renderer.clearZbuffer();
        const float t = time();
        float a = slide(t, 0, 0.8f); 
        a = a*a*a*(M_PI / 2);
        float sa = sinf(a);
        float ca = cosf(a);
        renderer.setLookAt({ 0 , 0, 0 }, { 0, sa , ca }, ((sa != 1) ? tgx::fVec3{0, 1, 0} : tgx::fVec3{ 0, 0, -1 }));
        drawSkyBox();
        redraw();
        
		em = GetTickCount() - time0;
    }
    // ends at (0, 0, 0), looking toward (0, 1, 0)
    
    const float base_width = 45;
    const float base_height = 8;
    const float base_y = -50; 

    const float looking_dist = -200; 

    loadMarbleTexture();

    // base falling

    start(4);  
    while (ongoing())
        {
        renderer.clearZbuffer();
        const float t = time();
        const float h = 1000 - (1000 - base_y) * t;
        const float sw = t < 0.8f ? base_width * (t / 0.8f) : base_width;
        const float sh = t < 0.8f ? base_height * (t / 0.8f) : base_height;

        tgx::fVec3 eye = { 0, 0, looking_dist * t};
        tgx::fVec3 obj = { 0, h , 0 };

        renderer.setLookAt(eye, { 0, (h > 0) ? h : 0, 0 }, (t == 0) ? tgx::fVec3{ 0, 0, -1 } : tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(obj, { sw, sh, sw }, slide(t)*360*5, {0,1,0});
        drawBase();

        redraw();
        em = GetTickCount() - time0;
   }


    // base bouncing
    start(3.5); // 4
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        const float h = base_y -  base_y * sinf(6 * M_PI * t) / (pow(2 * t + 1, 3) * (1+t));

        tgx::fVec3 eye = { 0, 0, looking_dist };
        tgx::fVec3 base_pos = { 0, h , 0 };

        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width }, 0, { 0,1,0 });
        drawBase();

        redraw();
        em = GetTickCount() - time0;
    }


    tgx::fVec3 base_pos = { 0, base_y , 0 };

    // marble glowing before sphere

    start(3); // 3
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 0, looking_dist };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        loadMarbleTexture();

        float r = (1 - cosf(t * 4 * M_PI))/2;
        float op = (1 - cosf(t * 3 * M_PI))/4 + 0.2f;

        marble_dma.drawSpot({ 64,64 }, 60 * r,tgx::RGBf(t,0,0), op);
        drawBase(true);
        drawSkyBox();
        redraw();
        em = GetTickCount() - time0;
    }



    loadEarthTexture();

    const float sphere_r = 60;
    const float sphere_dy = 68;

    // Wire-frame sphere

    start(3); // 3
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 0, looking_dist };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        const float a = slide(t, 0 , 0.8f); 
        tgx::fVec3 wsphere_pos = { 0, base_y + sphere_dy/3 + 2*a*sphere_dy/3 , 0 };
        float wsphere_size = sphere_r * a;

        setModelPosScaleRot(wsphere_pos, { wsphere_size, wsphere_size, wsphere_size }, 360 * a, { 0,1,0 });
        renderer.setMaterialColor(tgx::RGBf(1.0f, 0.0f, 0.0f));
        renderer.setCulling(0);
        renderer.drawWireFrameSphere(25, 25);

        redraw();
         em = GetTickCount() - time0;
    }

    tgx::fVec3 sphere_pos = { 0, base_y + sphere_dy, 0 };


    start(2); // 2
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();       

        tgx::fVec3 eye = { 0, 0, looking_dist };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        const float a = slide(t, 0.2f,0.6f);           


        if (a < 1.0)
            {
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });
            renderer.setMaterialColor(tgx::RGBf(1.0f - a, 0.0f, a));
            renderer.setCulling(0);
            renderer.drawWireFrameSphere(25, 25);
            }

        renderer.setCulling(1);
        setModelPosScaleRot(sphere_pos, { a * sphere_r, a * sphere_r, a * sphere_r });
        renderer.setMaterialSpecularExponent(32);
        renderer.setMaterialAmbiantStrength(0.1f);
        renderer.setMaterialDiffuseStrength(0.5f);
        renderer.setMaterialSpecularStrength(0.9f);
        renderer.setMaterialColor(tgx::RGBf(0, 0, 1));
        drawSphere(TGX_SHADER_GOURAUD);

        
        if (a < 1.0)
            {
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });
            renderer.setMaterialColor(tgx::RGBf(1.0f - a, 0.0f, a));
            renderer.setCulling(1);
            renderer.drawWireFrameSphere(25, 25);
            }
            

        redraw();
        em = GetTickCount() - time0;
    }

    const float climb = 40; 



    // rotating around the base
    start(20); // 20
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();
        const float u = slide(t,0, 0.95);


        renderer.setLookAt({ 350 * sinf(u * 6 * M_PI), t* climb , +150 - 350 * cosf(u * 6 * M_PI) } , { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });


        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        if (t< 0.33)
            { 
            renderer.setCulling(1);
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });

            renderer.setMaterialSpecularExponent(32);
            renderer.setMaterialAmbiantStrength(0.1f);
            renderer.setMaterialDiffuseStrength(0.5f);
            renderer.setMaterialSpecularStrength(0.9f);
            renderer.setMaterialColor(tgx::RGBf(0, 0, 1));
            drawSphere(TGX_SHADER_GOURAUD);
            }
        else if (t < 0.50)
            {
            float a =  (0.50f - t) / 0.17f;
            renderer.setCulling(1);
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });

            renderer.setMaterialSpecularExponent(32);
            renderer.setMaterialAmbiantStrength(0.1f *a);
            renderer.setMaterialDiffuseStrength(0.5f *a);
            renderer.setMaterialSpecularStrength(0.9f * a);
            renderer.setMaterialColor(tgx::RGBf(0, 0, 1) * a);

            //renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST);
            //renderer.drawMesh(&falcon_1, true, true);

            drawSphere(TGX_SHADER_GOURAUD);
            }

        else if (t < 0.66)
            {
            float a = 1.0f - (0.66f - t) / 0.16f;
            renderer.setCulling(1);
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });

            renderer.setMaterialSpecularExponent(32);
            renderer.setMaterialAmbiantStrength(0.3f * a);
            renderer.setMaterialDiffuseStrength(0.8f * a);
            renderer.setMaterialSpecularStrength(0.9f * a);

            drawSphere(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_WRAP_POW2, &earth_small);
            }
        else 
            {
            renderer.setCulling(1);
            setModelPosScaleRot(sphere_pos, { sphere_r, sphere_r, sphere_r });
         
            renderer.setMaterialSpecularExponent(32);
            renderer.setMaterialAmbiantStrength(0.3f);
            renderer.setMaterialDiffuseStrength(0.8f);
            renderer.setMaterialSpecularStrength(0.9f);

            drawSphere(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_WRAP_POW2, &earth_small);
            }


        drawSkyBox();

        redraw();
        em = GetTickCount() - time0;
      }


    start(15); // 15
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, climb + t*climb, looking_dist * ((t < 0.7f) ? ((t < 0.35f) ? (1 - (1.5f)*t) : (1 - 1.5f*0.35f + 1.5f*(t-0.35f))) : 1) };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });


        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        renderer.setCulling(1);

        const float angle = t * t * t * t * 20;
        const float r = sphere_r * ((t < 0.7f) ? 1 : ((1.0f - t) / 0.3f));
        setModelPosScaleRot(sphere_pos, { r, r, r },  angle * 360);
         
        renderer.setMaterialSpecularExponent(32);
        renderer.setMaterialAmbiantStrength(0.3f);
        renderer.setMaterialDiffuseStrength(0.8f);
        renderer.setMaterialSpecularStrength(0.9f);        

        drawSphere(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_BILINEAR | TGX_SHADER_TEXTURE_WRAP_POW2, &earth_small);
        drawSkyBox();

        redraw();
        em = GetTickCount() - time0;
    }


    

    // marble glowing before falcon   
    start(1.5f); // 3
    while (ongoing())
        {
        renderer.clearZbuffer();
        tgx::fVec3 eye = { 0, 2 * climb , looking_dist };
        renderer.setLookAt(eye, { 0, 0, 0 }, tgx::fVec3{ 0, 1, 0 });
        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();
        drawSkyBox();
        redraw();
        em = GetTickCount() - time0;
        }


    start(3); // 3
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 2 * climb , looking_dist };

        renderer.setLookAt(eye, { 0, 0, 0 }, tgx::fVec3{ 0, 1, 0 });

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        loadMarbleTexture();

        float r = (1 - cosf(t * 4 * M_PI))/2;
        float op = (1 - cosf(t * 3 * M_PI))/4 + 0.2f;

        marble_dma.drawSpot({ 64,64 }, 60 * r,tgx::RGBf(t,0,0), op);
        drawBase(true);
        drawSkyBox();
        redraw();
        em = GetTickCount() - time0;
        }




    loadFalconTexture();

    const float falcon_base = -20;



    // Wire-frame falcon
    start(2); // 2
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 2*climb , looking_dist };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        const float a = slide(t, 0.0 , 0.9f); 
        tgx::fVec3 falcon_pos = { 0, a * falcon_base + (1-a)*base_y , 0 };
        float falcon_size = sphere_r * 1.5f * a ;

        setModelPosScaleRot(falcon_pos, { falcon_size, falcon_size, falcon_size }, 360 * a, { 0,1,0 });
        renderer.setMaterialColor(tgx::RGBf(1.0f, 0.0f, 0.0f));
        renderer.drawWireFrameMesh(&falcon_vs_1, true);

        redraw();
        em = GetTickCount() - time0;
        }



    // falcon tranforming from wireframe to solid
    start(2); // 2
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 2*climb , looking_dist };
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        drawSkyBox();

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        const float a = slide(t, 0 , 0.8f); 
        tgx::fVec3 falcon_pos = { 0, falcon_base , 0 };
        float falcon_max_size = sphere_r * 1.5f;
        float falcon_size = sphere_r * 1.5f * a ;

        setModelPosScaleRot(falcon_pos, { falcon_size, falcon_size, falcon_size }, 0, { 0,1,0 });
        renderer.setCulling(1);
        renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);
        renderer.drawMesh(&falcon_vs_1, true, true);

        if (a < 1)
            {
            setModelPosScaleRot(falcon_pos, { falcon_max_size, falcon_max_size, falcon_max_size }, 0, { 0,1,0 });
            renderer.setMaterialColor(tgx::RGBf(1-a, 0, 0));
            renderer.drawWireFrameMesh(&falcon_vs_1, true);
            }

        redraw();
        em = GetTickCount() - time0;
        }


    start(20); // 20

    //tft.setDiffCompareMask(0, 0, 0); // blur to improve framerate
    
   // observation
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        const float a = slide(t , 0.0f, 0.9f) * 4.0f * M_PI;

        const float l = (cosf(a/2.0) + 1.5) / 2.5;

        tgx::fVec3 eye = { (looking_dist * l) * sinf(a), (float)(2.0f*climb - (a  * climb / (float)(2.0f*M_PI))) , (looking_dist * l) *cosf(a)};
        
        renderer.setLookAt(eye, { 0, 0, 0 },  tgx::fVec3{ 0, 1, 0 });

        tgx::fVec3 falcon_pos = { 0, falcon_base , 0 };
        float falcon_size = sphere_r * 1.5f;

        setModelPosScaleRot(falcon_pos, { falcon_size, falcon_size, falcon_size }, 0, { 0,1,0 });
        renderer.setCulling(1);
        renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);
        renderer.drawMesh(&falcon_vs_1, true, true);

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        drawSkyBox();

        redraw();   
        em = GetTickCount() - time0;  
        }

   // tft.setDiffCompareMask(0, 0, 0);


    // montee
    start(3); // 2
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 0 , looking_dist };
        tgx::fVec3 falcon_pos = { 0, falcon_base + 50*slide(t, 0.3f,0.9f), 0 };

        renderer.setLookAt(eye, { 0, 0, 0 }, tgx::fVec3{ 0, 1, 0 });

        float falcon_size = sphere_r * 1.5f;

        setModelPosScaleRot(falcon_pos, { falcon_size, falcon_size, falcon_size }, 0, { 0,1,0 });
        renderer.setCulling(1);
        renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);
        
        renderer.drawMesh(&falcon_vs_1, true, true);

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        drawSkyBox();

        redraw();
        em = GetTickCount() - time0;
        }

    // allumage et depart
    falcon_vs_15.texture = &Engine02_texture;


    start(2.5); // 2.5
    while (ongoing())
        {
        renderer.clearZbuffer();

        const float t = time();

        tgx::fVec3 eye = { 0, 0 , looking_dist };
        tgx::fVec3 falcon_pos = { 0, falcon_base + 50 - (falcon_base + 50)*slide(t, 0.3f,0.9f), 1000 * t  * t };

        renderer.setLookAt(eye, { 0, 0, 0 }, tgx::fVec3{ 0, 1, 0 });

        float falcon_size = sphere_r * 1.5f;

        setModelPosScaleRot(falcon_pos, { falcon_size, falcon_size, falcon_size }, 0, { 0,1,0 });
        renderer.setCulling(1);
        renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);
        
        renderer.drawMesh(&falcon_vs_1, true, true);

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        drawSkyBox();

        redraw();
        em = GetTickCount() - time0;
        }

    // tournant lointain
    const float rturn = 250; 
    

    start(2.5); //2
    while (ongoing())
    {
        renderer.clearZbuffer();

        const float t = time();

        const float a = 1.25 * t * M_PI;

        tgx::fVec3 eye = { 0, 0 , looking_dist };
        tgx::fVec3 falcon_pos = { 0, rturn - rturn*cosf(a), 1000 + rturn * sinf(a) };

        renderer.setLookAt(eye, falcon_pos, tgx::fVec3{ 0, 1, 0 });

        float falcon_size = sphere_r * 1.5f;

        tgx::fMat4 M;
        M.setScale({ falcon_size, falcon_size, falcon_size });
        if (t > 0.3f) M.multRotate(180 * (t - 0.3f) / 0.7f, { 0,0,1 });
        M.multRotate(-180*a / M_PI, { 1,0,0 });
        M.multTranslate(falcon_pos);
        renderer.setModelMatrix(M);

        renderer.setCulling(1);
        renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);

        renderer.drawMesh(&falcon_vs_1, true, true);

        setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
        drawBase();

        drawSkyBox();

        redraw();
        em = GetTickCount() - time0;
    }

    // redescente vers camera
    {


        start(1); // 1

        const float aa = 1.25 * M_PI;
        const float py = rturn - rturn * cosf(aa);
        const float pz = 1000 + rturn * sinf(aa);

        while (ongoing())
        {
            renderer.clearZbuffer();

            const float t = time();

            tgx::fVec3 eye = { 0, 0 , looking_dist };
            tgx::fVec3 falcon_pos = { 0, py - 500 * t, pz - 500 * t };

            renderer.setLookAt(eye, falcon_pos, tgx::fVec3{ 0, 1, 0 });

            float falcon_size = sphere_r * 1.5f;

            tgx::fMat4 M;
            M.setScale({ falcon_size, falcon_size, falcon_size });
            M.multRotate(180, { 0,0,1 });
            M.multRotate(-225, { 1,0,0 });
            M.multTranslate(falcon_pos);
            renderer.setModelMatrix(M);

            renderer.setCulling(1);
            renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);

            renderer.drawMesh(&falcon_vs_1, true, true);

            setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
            drawBase();

            drawSkyBox();

            redraw();
            em = GetTickCount() - time0;
        }
    }

    // tournant devant
    const float rdist2 = 260;
    {

        start(0.8); // 0.8

        const float aa = 1.25f * M_PI;
        const float py = rturn - rturn * cosf(aa) - 500 + rdist2*cosf(M_PI/4);
        const float pz = 1000 + rturn * sinf(aa) - 500 - rdist2*cosf(M_PI / 4);

        while (ongoing())
        {
            renderer.clearZbuffer();

            const float t = time();
            const float a = 0.75f * t * M_PI;

            tgx::fVec3 eye = { 0, 0 , looking_dist };
            tgx::fVec3 falcon_pos = { 0, py - rdist2*sinf(a + M_PI/4) , pz + rdist2 * cosf(a + M_PI / 4) };

            renderer.setLookAt(eye, falcon_pos, tgx::fVec3{ 0, 1, 0 });

            float falcon_size = sphere_r * 1.5f;

            tgx::fMat4 M;
            M.setScale({ falcon_size, falcon_size, falcon_size });
            M.multRotate(180, { 0,0,1 });
            M.multRotate(-225 + 135*t, { 1,0,0 });
            M.multTranslate(falcon_pos);
            renderer.setModelMatrix(M);

            renderer.setCulling(1);
            renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);

            renderer.drawMesh(&falcon_vs_1, true, true);

            setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
            drawBase();

            drawSkyBox();

            redraw();
            em = GetTickCount() - time0;
        }
    }


    // monté finale
    {

        start(2); // 2

        const float a = 0.75f * M_PI;
        const float aa = 1.25f * M_PI;
        const float py = rturn - rturn * cosf(aa) - 500 + rdist2*cosf(M_PI/4) - rdist2 * sinf(a + M_PI/4);
        const float pz = 1000 + rturn * sinf(aa) - 500 - rdist2*cosf(M_PI/4) + rdist2 * cosf(a + M_PI/4);

        while (ongoing())
        {
            renderer.clearZbuffer();

            const float t = time();
            
            tgx::fVec3 eye = { 0, 0 , looking_dist };
            tgx::fVec3 falcon_pos = { 0, py + 3000*t , pz };

            renderer.setLookAt(eye, falcon_pos, tgx::fVec3{ 0, 1, 0 });

            float falcon_size = sphere_r * 1.5f;

            tgx::fMat4 M;

            const float s = sqrt(1 - t);
            M.setScale({ falcon_size*s, falcon_size* s, falcon_size* s });
            M.multRotate(180 + t * 400, { 0,0,1 });
            M.multRotate(-90, { 1,0,0 });
            M.multTranslate(falcon_pos);
            renderer.setModelMatrix(M);

            renderer.setCulling(1);
            renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2);

            renderer.drawMesh(&falcon_vs_1, true, true);

            setModelPosScaleRot(base_pos, { base_width, base_height, base_width });
            drawBase();

            drawSkyBox();

            redraw();
            em = GetTickCount() - time0;
        }
    }

    falcon_vs_15.texture = &Engine01_texture;
}



TGX_NOINLINE FLASHMEM void ending ()
{
    //tft.setVSyncSpacing(1);


    start(4); // 2
    
    while (ongoing() && !kbhit()){
        const float t = time();
        const float a = (1.0f - slide(t, 0, 0.8f)) * 210;
        const float a2 = a * a;

        for (int y = 0; y < SLY; y++){
            for (int x = 0; x < SLX; x++){
                if ((float)((x-SLX2) * (x-SLX2) + (y-SLY2) * (y-SLY2)) >= a2){
                    im(x, y) = tgx::RGB565_Black;
				}
			}
		}
		
        update();
        yield();
        em = GetTickCount() - time0;
	}
    //tft.setVSyncSpacing(2);
}



TGX_NOINLINE FLASHMEM void beginning ()
    {
    //tft.setVSyncSpacing(1);
    tgx::Image<tgx::RGB565> imdma(dma_buf, SLX, SLY, SLX); 
    
    renderer.setImage(&imdma); 
    renderer.clearZbuffer();
    renderer.setLookAt({ 0, 0, 0 }, { 0, 0, 1 }, { 0,1,0 });
    drawSkyBox();
    renderer.setImage(&im);


    start(4); // 2
    
    while (ongoing() && !kbhit()){
        const float t = time();
        float a = slide(t, 0.0f, 1.0f);
        a = a * a * 210;
        const float a2 = a * a;
        for (int y = 0; y < SLY; y++){
            for (int x = 0; x < SLX; x++){
                if ((float)((x - SLX2) * (x - SLX2) + (y - SLY2) * (y - SLY2)) < a2){
                    im(x, y) = imdma(x,y);
				}
			}
		}
		
        update();
        yield();
        em = GetTickCount() - time0;
	}
    //tft.setVSyncSpacing(2);
}


void intro ()
{

    elapsedMillis em = 0;
    time0 = GetTickCount();
    
    while (em < 2500 && !kbhit()){
        int y1 = (em < 1700) ? (130 * em) / 1700 - 50 : 80;
        int y2 = (em < 1700) ? 240 - (110 * em) / 1700 : 130;
        im.fillScreen(RGB565_Black);
        im.drawText("TGX library", iVec2{ 110, y1 }, RGB565_Red, font_tgx_OpenSans_Bold_18, true);
        im.drawText("Mars demo", iVec2{ 80, y2 }, RGB565_White, font_tgx_OpenSans_Bold_24, true);        
        
        update();
        yield();
        em = GetTickCount() - time0;
    }

    //tft.setVSyncSpacing(2);
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

    //while (!kbhit()){
		intro();        
        beginning();
        movie();
        ending();
        
		//Sleep(1);
	//}
	
	uint64_t tend = GetTickCount();

	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
}
       

/** end of file */
