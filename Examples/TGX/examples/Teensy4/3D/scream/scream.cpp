/********************************************************************
*
* tgx library example : changing the geometry at runtime. 
* 
* 'Explosion' the scream Painting !
*
* EXAMPLE FOR TEENSY 4 / 4.1
*
* DISPLAY: ILI9341 (320x240)
*
********************************************************************/



#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <tgx.h>
#include <stdio.h>
#include <string.h>
#include <libHidDisplay.h>

using namespace tgx;

#include <Renderer3D.h>
#include <Renderer3D.cpp>
#include <font_tgx_OpenSans_Bold.h>
#include <font_tgx_OpenSans_Bold.cpp>
#include <Color.h>
#include <Color.cpp>

// the texture image of the sheet
#include "scream_texture.h"




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

// only load the shaders we need.
const int LOADED_SHADERS = TGX_SHADER_PERSPECTIVE | TGX_SHADER_ZBUFFER | TGX_SHADER_GOURAUD | TGX_SHADER_NOTEXTURE | TGX_SHADER_TEXTURE_NEAREST |TGX_SHADER_TEXTURE_WRAP_POW2;

// the renderer object that performs the 3D drawings
Renderer3D<RGB565, LOADED_SHADERS, uint16_t> renderer;

static const int N = 45; // [-1,1]x[-1,1] is subdivided in NxM subsquares
static const int M = 45; // total number of triangles is 2*N*M

const float dx = 2.0f/N; // each subsquare has size  dx x dy
const float dy = 2.0f/M;

fVec3 vertices[(N+1)*(M+1)];        //  arrays of vertices : grid with y coordinate giving the height of the sheet at that point
fVec3 normals[(N + 1) * (M + 1)];   // arrays of normals
fVec2 texcoords[(N+1)*(M+1)];       // array of texture coordinates
DMAMEM uint16_t faces[4*N*M];       // arrays of quads (in DMAMEM because when are running out of memory in DTCM).  


static inline float max (float a, float b)
{
	if (a > b) return a;
	else return b;
}


static uint64_t fcount = 0;
static uint64_t time0 = GetTickCount();

static void yield ()
{
	//Sleep(10);
}

void update ()
{
	static uint64_t tstart = GetTickCount();
	
	if (ctx.rgbClamp)
		libHidDisplay_WriteImageEx(&ctx, fb, ctx.rgbClamp);
	else
		libHidDisplay_WriteImage(&ctx, fb);
	
	fcount++;
	
	if (!(fcount%100)){
		uint64_t tend = GetTickCount();
		printf("%.1f fps\n", (float)fcount / (float)(tend - tstart)*1000.0f);
	}
}




/**
* Initialise the vertices, texcoords and faces arrays
**/
void initSheet()
{
    const float dx = 2.0f/(float)N; 
    const float dy = 2.0f/(float)M;
    
    for (int j = 0; j <= M; j++){
        for (int i = 0; i <= N; i++){
            vertices[(N + 1)*j + i] = fVec3(i*dx - 1, 0, j*dy - 1);
            texcoords[(N + 1)*j + i] = fVec2(i*dx / 2, j*dy / 2);
        }
    }
    
    for (int j = 0; j < M; j++){
        for (int i = 0; i < N; i++){
            faces[4 * (N * j + i) + 0] = (uint16_t)(j*(N+1) + i);
            faces[4 * (N * j + i) + 1] = (uint16_t)((j + 1) * (N + 1) + i);
            faces[4 * (N * j + i) + 2] = (uint16_t)((j+1)*(N+1) + i + 1);
            faces[4 * (N * j + i) + 3] = (uint16_t)(j * (N + 1) + i + 1);
       }
   }
}


/**
* Compute the face normal of the CCW triangle with index i1, i2, i3
* and add the vector to the normal vector of each of its vertices
**/
void faceN (int i1, int i2, int i3)
{
    const fVec3 P1 = vertices[i1]; 
    const fVec3 P2 = vertices[i2];
    const fVec3 P3 = vertices[i3];
    const fVec3 N = crossProduct(P1 - P3, P2 - P1);
    
    normals[i1] += N;
    normals[i2] += N;    
    normals[i3] += N;
}


/**
* Recompute the normal arrays. 
**/
static void updateNormals ()
{
    // set all normals to 0
    for (int k = 0; k < (N + 1) * (M + 1); k++)
    	normals[k] = fVec3(0, 0, 0);
    
    // add the normal of the adjacent faces
    for (int j = 0; j < M; j++){
        for (int i = 0; i < N; i++){
            faceN((N + 1) * j + i, (N + 1) * (j + 1) + i, (N + 1) * j + i + 1);
            faceN((N + 1) * (j + 1) + i, (N + 1) * (j + 1) + i + 1, (N + 1) * j + i + 1);
        }
    }
    
    // normalize the normals
    for (int k = 0; k < (N + 1) * (M + 1); k++)
    	normals[k].normalize();
}


/** sine cardinal function (could maybe speedup with a precomputed table ?)*/
static float sinc (float x)
{
    if (abs(x) < 0.01f) return 1.0f; 
    return sinf(x)/x; 
}


/** return uniform random number in [a,b[ */
static float unif (float a, float b)
{
   int bb = (fabs(a) + b) * 1000.0f;
   return ((rand()%bb) / 1000.0f) - fabs(a);
}


/** return the camera's curent position (depending on time) */
static tgx::fVec3 cameraPosition ()
{   
    static elapsedMillis em; 
    em = GetTickCount() - time0;
    
    const float T = 30.0f; // rotation period (in seconds). 
    const float d = 1.5f + 0.5f * sinf(em/10000.0f); // distance from sheet move closer / farther away with time. 
    const float w = em * 2.0f * M_PI / (1000.0f * T) + M_PI*0.6f;
    return fVec3(d * sinf(w), d*0.8f, d * cosf(w));
}



void explosion (fVec2 center, float h, float w, float s, float start_delay = 0)
{
    s /= 1000.0f; // normalise speed

    elapsedMillis et = 0; 
 	//time0 = GetTickCount();
 	
    const float t0 = start_delay;          // time the bump starts to grow. 
    const float t1 = start_delay + 2000;   // time of explosion 
    const float t2 = start_delay + 5000;   // time when explosion is finished

    bool exploded = false; 

	//et = GetTickCount() - time0;

    while (et < t2){
    	
        float t; 
		t = et; 

        // erase the screen
        im.fillScreen(RGB565_Black);

        // clear the z-buffer
        renderer.clearZbuffer();

        // amplitude multiplier 
        const float alpha = ((t <= t0) || (t >= t2)) ? 0 : ((t < t1) ? (t - t0) / (t1 - t0) : (1 - (t - t1) / (t2 - t1)));

        // compute heights
        for (int j = 0; j <= M; j++)
            {
            for (int i = 0; i <= N; i++)
                {
                const float x = dx*i - 1.0f - center.x;
                const float y = dy*j - 1.0f - center.y;
                const float r = sqrt(x * x + y * y); // distance to center. 
                
                vertices[(N + 1) * j + i].y = alpha * h * sinc(w* (r - max(0, t - t1)*s));
                }
            }

        // compute normals
        updateNormals();

        // increase time
        t += 0.5f;

        // set the camera position
        renderer.setLookAt(cameraPosition(), { 0,0,0 }, { 0,1,0 });

        // draw !        
        renderer.drawQuads(N * M, faces, vertices, faces, normals, faces, texcoords, &scream_texture);

       // remove the line above ('renderer.drawQuads(...') and uncomment the code below 
       // to draw the sheet without texturing but where the color depend in the height. 
       /*
       for(int j=0; j < N*M; j++)
          {
          const fVec3 V1 = vertices[faces[4*j]];
          const fVec3 V2 = vertices[faces[4*j + 1]];
          const fVec3 V3 = vertices[faces[4*j + 2]];
          const fVec3 V4 = vertices[faces[4*j + 3]];      

          const fVec3 N1 = normals[faces[4*j]];
          const fVec3 N2 = normals[faces[4*j + 1]];
          const fVec3 N3 = normals[faces[4*j + 2]];
          const fVec3 N4 = normals[faces[4*j + 3]];
     
          const float h1 = V1.y * 3.0f +0.4f;
          const float h2 = V2.y * 3.0f +0.4f;
          const float h3 = V3.y * 3.0f +0.4f;
          const float h4 = V4.y * 3.0f +0.4f;
          const RGBf C1(h1,h1*(1-h1)*3,1-h1);
          const RGBf C2(h2,h2*(1-h2)*3,1-h2);
          const RGBf C3(h3,h3*(1-h3)*3,1-h3);
          const RGBf C4(h4,h4*(1-h4)*3,1-h4);

          renderer.drawQuadWithVertexColor(V1,V2,V3,V4, C1, C2, C3, C4, &N1, &N2, &N3, &N4);
         }
        */

        // update the screen (asynchronous). 
        update();

        if (t < t1){
           // printf("-\n");
        }else if (exploded){
          // printf(".\n");
          // time0 = GetTickCount();
        }else{
           	//printf("[BOOM!]\n");
            exploded = true;
            //time0 = GetTickCount();
        }
        et = GetTickCount() - time0;
	}
}


void setup ()
{

    // setup the 3D renderer.
    renderer.setViewportSize(SLX,SLY); // viewport = screen     
    renderer.setOffset(0, 0); //  image = viewport
    renderer.setImage(&im); // set the image to draw onto (ie the screen framebuffer)
    renderer.setZbuffer(zbuf); // set the z buffer for depth testing    
    renderer.setPerspective(45, ((float)SLX) / SLY, 0.1f, 50.0f);  // set the perspective projection matrix. 
    renderer.setShaders(TGX_SHADER_GOURAUD | TGX_SHADER_TEXTURE_NEAREST | TGX_SHADER_TEXTURE_WRAP_POW2); // shader ot use
    renderer.setCulling(0); // in case we see below the sheet. 

    fMat4 MV;
    MV.setIdentity();
    renderer.setModelMatrix(MV); // identity model matrix. 

    // intialize the sheet. 
    initSheet(); 

    // the default seed give a nice sequence of explosion 
    srand(time(NULL));
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
	
    	
    // first an explosion carefully chosen...
	float h = 0.4;
	float w = 10;
	float s = 0.4;
    fVec2 center(0,0);
    
    explosion(center, h, w, s, 2000);

    // ...and then random explosions
    while (!kbhit()){ 
        h = unif(0.0f, 0.3f);   // bump height
        w = unif(3.0f, 18.0f);        // wave size
        s = unif(0.2f, 2.0f);       // wave speed;

        fVec2 center(unif(-0.7f, 0.7f), unif(-0.7f, 0.7f));  // explosion center   
        explosion(center, h,w,s,  (GetTickCount()-time0));
	}
	
	uint64_t tend = GetTickCount();

	printf("\n: %.1f fps, %i %i\n", (float)fcount / (float)(tend - tstart)*1000.0f, tend - tstart, fcount);
	
	timeEndPeriod(1);
	libHidDisplay_CloseDisplay(&ctx);
	
}
       


