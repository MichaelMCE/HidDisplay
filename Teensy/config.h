#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_



#define USE_DMAMEM_BUFFER		0		// set this to 1 to use DMAMEM for the tft buffer
#define USE_EXTMEM_BUFFER		0		// set this to 1 to use PSRAM for the tft buffer
#define USE_STRIP_RENDERER		1		// enable/disables updates to display by STRIP_RENDERER_HEIGHT rows per iterations per frame
#define STRIP_RENDERER_HEIGHT	32		// as above. Intended for use with the larger panels
#define USE_STARTUP_IMAGE		1		// display a power on image at start up
#define USE_BUFFER_LAYERS		0		// enable muliple backbuffer lateys. Is valid with USE_EXTMEM_BUFFER/PSRAM only. Can not use with USE_STRIP_RENDERER
#define BUFFER_LAYERS_TOTAL		8		// make room for n backbuffer layers. Each layer size is = (WIDTH * HEIGHT * 2)
#define ENABLE_TOUCH_FT5216		1		// WIP. enable i2c touch controller and reports


// there can only be one
#define USE_FLEXTFT_ILI9486		0
#define USE_FLEXTFT_S6D04D1		0
#define USE_FLEXTFT_ILI9806		0
#define USE_FLEXTFT_RM68120		0
#define USE_FLEXTFT_R61529		0
#define USE_FLEXTFT_R61408		1
#define USE_FLEXTFT_LG4572B		0
#define USE_FLEXTFT_NT35510		0
#define USE_FLEXTFT_NT35516		0
#define USE_SPI_GC9A01A			0
#define USE_SPI_ST7735			0


#if USE_SPI_ST7735
#define TFT_WIDTH				160
#define TFT_HEIGHT				128
#define CFG_STRING				"160x128*2 ST7735 TFT Display"
#define TFT_SPEED				56			// Mhz
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_SPI_GC9A01A
#define TFT_WIDTH				240
#define TFT_HEIGHT				240
#define CFG_STRING				"240x240*2 GC9A01A IPS Display"
#define TFT_SPEED				100			// Mhz
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_S6D04D1
#define TFT_WIDTH				432
#define TFT_HEIGHT				240
#define CFG_STRING				"432x240*2 S6D04D1 IPS Display"
#define TFT_SPEED				20			// Mhz
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_R61529
#define TFT_WIDTH				480
#define TFT_HEIGHT				320
#define CFG_STRING				"480x320*2 R61529 IPS Display"
#define TFT_SPEED				20
#define TOUCH_ROTATION			TOUCH_DIR_SWAP_A_INVERT_V
#elif USE_FLEXTFT_ILI9486
#define TFT_WIDTH				480
#define TFT_HEIGHT				320
#define CFG_STRING				"480x320*2 ILI9486 TFT Display"
#define TFT_SPEED				40
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_RM68120
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 RM68120 TFT Display"
#define TFT_SPEED				40
#define TOUCH_ROTATION			TOUCH_DIR_SWAP_A_INVERT_H
#elif USE_FLEXTFT_R61408
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 R61408 IPS Display"
#define TFT_SPEED				24
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_LG4572B
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 LG4572B IPS Display"
#define TFT_SPEED				24
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_NT35510
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 NT35510 IPS Display"
#define TFT_SPEED				16
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#elif USE_FLEXTFT_ILI9806
#define TFT_WIDTH				854
#define TFT_HEIGHT				480
#define CFG_STRING				"854x480*2 ILI9806 IPS Display"
#define TFT_SPEED				24
#define TOUCH_ROTATION			TOUCH_DIR_RLBT
#elif USE_FLEXTFT_NT35516
#define TFT_WIDTH				960
#define TFT_HEIGHT				540
#define CFG_STRING				"960x540*2 NT35516 IPS Display"
#define TFT_SPEED				22
#define COL_CLAMP_MIN			0
//#define COL_CLAMP_MAX			246
#define TOUCH_ROTATION			TOUCH_DIR_NONE
#endif


// Pins
#define TFT_RS					10		// Register select (DC)
#define TFT_RST					11		// Reset
#define TFT_CS					12		// Chip select
#define TFT_BL					33		// Backlight
#define TFT_INTENSITY			60		// PWM value from 0 to 255
// Read is 37
// Write is 36

#define SPI_RST					8
#define SPI_DC					9
#define SPI_CS					10
#define SPI_MOSI				11
#define SPI_SCK					13


#define FT5216_INT				34		// FT5216 data ready interrupt pin
#define FT5216_ADDR				0x38	// i2c address
#define FT5216_SPEED			1000000	// set speed in hz
#define FT5216_obj				Wire2	// using i2c pins 24 & 25 on T4.1


#if (USE_DMAMEM_BUFFER)
#define STORAGETYPE DMAMEM 
#elif (USE_EXTMEM_BUFFER)
#define STORAGETYPE EXTMEM 
#else
#define STORAGETYPE  
#endif


#include "device_serial.h"
#include "displays.h"


#endif

