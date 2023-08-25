

#if COMPILEMEONCE
#undef COMPILEMEONCE
#define COMPILEMEONCE 0


#include "config.h"
#include "displays.h"


#if USE_STRIP_RENDERER
#define TFT_HEIGHT_SPLIT (STRIP_RENDERER_HEIGHT)
#else
#define TFT_HEIGHT_SPLIT (TFT_HEIGHT)
#endif

#if (USE_EXTMEM_BUFFER && USE_BUFFER_LAYERS)
#define layersTotal (BUFFER_LAYERS_TOTAL)
static int layersActive = 0;
#else
#define layersTotal (1)
static int layersActive = 0;
#endif

static uint8_t STORAGETYPE tft_buffer[layersTotal][CALC_PITCH_16(TFT_WIDTH)*TFT_HEIGHT_SPLIT];



/*
layers_setActive
layers_getActive
layers_swap
layers_next
*/

int tft_setWriteLayer (const int layerIdx)
{
	int old = layersActive;
	if (layerIdx < layersTotal)
		layersActive = layerIdx;

	return old;
}

int tft_getWriteLayer ()
{
	return layersActive;
}

int tft_nextLayer ()
{
	int nextLayer = layersActive + 1;
	if (nextLayer >= layersTotal)
		nextLayer = 0;

	layersActive = nextLayer;
	return tft_getWriteLayer();
}

int tft_toggleLayer ()
{
	int nextLayer = (!tft_getWriteLayer())&0x01;
	tft_setWriteLayer(nextLayer);
	return tft_getWriteLayer();
}

void tft_init ()
{
#if USE_EXTMEM_BUFFER
#if ENABLE_EXTRAM_CACHE
	// enable PSRAM cache
	FLEXSPI2_AHBCR = FLEXSPI_AHBCR_PREFETCHEN | FLEXSPI_AHBCR_BUFFERABLEEN;
	FLEXSPI2_AHBRXBUF0CR0 = 0;	// disable BUF0/1/2, BUF3 will share entire buffer space between all AHB masters
	FLEXSPI2_AHBRXBUF1CR0 = 0;
	FLEXSPI2_AHBRXBUF2CR0 = 0;
	FLEXSPI2_AHBRXBUF3CR0 = FLEXSPI_AHBRXBUFCR0_PREFETCHEN;
#endif

	// increase PSRAM clocks to max stable, ~130MHZ
	CCM_CBCMR &= ~(CCM_CBCMR_FLEXSPI2_PODF_MASK | CCM_CBCMR_FLEXSPI2_CLK_SEL_MASK);		// clear settings
	CCM_CBCMR |=  (CCM_CBCMR_FLEXSPI2_PODF(3)   | CCM_CBCMR_FLEXSPI2_CLK_SEL(3));		// 133 MHz
#endif

	display_begin(TFT_SPEED);
}

void tft_update ()
{
	display_update((uint16_t*)tft_buffer[layersActive], 0, 0, TFT_WIDTH-1, TFT_HEIGHT_SPLIT-1);
}

void tft_update_area (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	display_update((uint16_t*)tft_buffer[layersActive], x1, y1, x2, y2);
}

static inline void tft_clearFrame (uint16_t *pixels, const uint16_t colour)
{
	int tPixels = TFT_WIDTH * TFT_HEIGHT_SPLIT;
	for (int i = 0; i < tPixels; i++)
		pixels[i] = colour;
}

void tft_clearBuffer (const uint16_t colour)
{
	uint16_t *pixels = (uint16_t*)tft_buffer[layersActive];
	tft_clearFrame(pixels, colour);
}

void tft_clear (const uint16_t colour)
{
	uint16_t *pixels = (uint16_t*)tft_buffer[layersActive];
	tft_clearFrame(pixels, colour);

	for (int y = 0; y < TFT_HEIGHT; y++)
		display_update(pixels, 0, y, TFT_WIDTH-1, y);
}

uint8_t *tft_getBuffer ()
{
	return (uint8_t*)tft_buffer[layersActive];
}

void tft_rotate (const uint8_t rotation)
{
	display_setRotation(rotation);
}

void tft_update_array (uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	display_update(pixels, x1, y1, x2, y2);
}

void tft_backlight (const uint8_t level)
{
	display_setBacklight(level);
}

#endif
