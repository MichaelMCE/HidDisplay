#pragma once
#ifndef _DISPLAYS_H_
#define _DISPLAYS_H_


#include "FlexIo/ILI9486/ILI9486_t41_p.h"
#include "FlexIo/S6D04D1/S6D04D1_t41_p.h"
#include "FlexIo/ILI9806/ILI9806_t41_p.h"
#include "FlexIo/RM68120/RM68120_t41_p.h"
#include "FlexIo/LG4572B/LG4572B_t41_p.h"
#include "FlexIo/NT35510/NT35510_t41_p.h"
#include "FlexIo/NT35516/NT35516_t41_p.h"
#include "FlexIo/R61529/R61529_t41_p.h"
#include "FlexIo/R61408/R61408_t41_p.h"
#include "SPI/GC9A01A/GC9A01A_spi.h"
#include "SPI/ST7735/ST7735_spi.h"


#include "SPI/GC9A01A_rewrite/GC9A01A_spi.h"



#define CALC_PITCH_1(w)		(((w)>>3)+(((w)&0x07)!=0))	// 1bit packed, calculate number of storage bytes per row given width (of glyph)
#define CALC_PITCH_16(w)	((w)*sizeof(uint16_t))		// 16bit (8 bits per byte)
#define COLOUR_24TO16(c)	((uint16_t)(((((c)>>16)&0xF8)<<8) | ((((c)>>8)&0xFC)<<3) | (((c)&0xF8)>>3)))

// call me firstmost
void tft_init ();

// send buffer to display
void tft_update ();
void tft_update_area (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// clear display + buffer
void tft_clear (const uint16_t colour);

// clear buffer only
void tft_clearBuffer (const uint16_t colour);

// set display rotation. this does not rotate the buffer.
void tft_rotate (const uint8_t rotation);

// get a pointer to active layer
uint8_t *tft_getBuffer ();

// send an array/buffer directly to display, to the destination area
void tft_update_array (uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// adjust backlight PWN level. range is 0 to 127
void tft_backlight (const uint8_t level);

// set active layer
int tft_setWriteLayer (const int layerIdx);

// return active layer
int tft_getWriteLayer ();

// switch between layers 1 and 0
int tft_toggleLayer ();

// activate next layer in chain or back to 0 if end
int tft_nextLayer ();





#endif

