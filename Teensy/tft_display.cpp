

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


static uint8_t STORAGETYPE tft_buffer[CALC_PITCH_16(TFT_WIDTH)*TFT_HEIGHT_SPLIT];



void tft_init ()
{
#if USE_EXTMEM_BUFFER
#if 1
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

	lcd.begin(TFT_SPEED);
}

void tft_update ()
{
	lcd.pushPixels16bit((uint16_t*)tft_buffer, 0, 0, TFT_WIDTH-1, TFT_HEIGHT_SPLIT-1);
}

void tft_update_area (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	lcd.pushPixels16bit((uint16_t*)tft_buffer, x1, y1, x2, y2);
}

static inline void tft_clearFrame (uint16_t *pixels, const uint16_t colour)
{
	int tPixels = TFT_WIDTH * TFT_HEIGHT_SPLIT;
	for (int i = 0; i < tPixels; i++)
		pixels[i] = colour;
}

void tft_clearBuffer (const uint16_t colour)
{
	uint16_t *pixels = (uint16_t*)tft_buffer;
	tft_clearFrame(pixels, colour);
}

void tft_clear (const uint16_t colour)
{
	uint16_t *pixels = (uint16_t*)tft_buffer;
	tft_clearFrame(pixels, colour);

	for (int y = 0; y < TFT_HEIGHT; y++)
		lcd.pushPixels16bit(pixels, 0, y, TFT_WIDTH-1, y);
}

uint8_t *tft_getBuffer ()
{
	return (uint8_t*)tft_buffer;
}

void tft_rotate (const uint8_t rotation)
{
	lcd.setRotation(rotation);
}

void tft_update_array (uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	lcd.pushPixels16bit(pixels, x1, y1, x2, y2);
}

void tft_backlight (const uint8_t level)
{
	lcd.setBacklight(level);
}

#endif
