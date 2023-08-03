#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_



#define USE_DMAMEM_BUFFER		0		// set this to 1 to use DMAMEM for the tft buffer
#define USE_EXTMEM_BUFFER		0		// set this to 1 to use PSRAM for the tft buffer
#define USE_EXTMEM_ALLOC		0		// use this to divert uFont allocations to PSRAM. Beaware - this will cost 32k of RAM1
#define USE_STRIP_RENDERER		1		// enable/disables updates to display by STRIP_RENDERER_HEIGHT rows per iterations per frame
#define STRIP_RENDERER_HEIGHT	32		// as above. Intended for use with the larger panels
#define USE_STARTUP_IMAGE		1		// display a power on image at start up

// there can only be one
#define USE_FLEXTFT_ILI9486		0
#define USE_FLEXTFT_S6D04D1		0
#define USE_FLEXTFT_ILI9806		1
#define USE_FLEXTFT_RM68120		0
#define USE_FLEXTFT_R61529		0
#define USE_FLEXTFT_LG4572B		0
#define USE_FLEXTFT_NT35510		0
#define USE_FLEXTFT_NT35516		0



#if USE_FLEXTFT_S6D04D1
#define TFT_WIDTH				432
#define TFT_HEIGHT				240
#define CFG_STRING				"432x240*2 S6D04D1 IPS Display"
#define TFT_SPEED				20			// Mhz
#elif USE_FLEXTFT_R61529
#define TFT_WIDTH				480
#define TFT_HEIGHT				320
#define CFG_STRING				"480x320*2 R61529 IPS Display"
#define TFT_SPEED				22
#elif USE_FLEXTFT_ILI9486
#define TFT_WIDTH				480
#define TFT_HEIGHT				320
#define CFG_STRING				"480x320*2 ILI9486 TFT Display"
#define TFT_SPEED				20
#elif USE_FLEXTFT_RM68120
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 RM68120 TFT Display"
#define TFT_SPEED				40
#elif USE_FLEXTFT_LG4572B
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 LG4572B IPS Display"
#define TFT_SPEED				24
#elif USE_FLEXTFT_NT35510
#define TFT_WIDTH				800
#define TFT_HEIGHT				480
#define CFG_STRING				"800x480*2 NT35510 IPS Display"
#define TFT_SPEED				16
#elif USE_FLEXTFT_ILI9806
#define TFT_WIDTH				854
#define TFT_HEIGHT				480
#define CFG_STRING				"854x480*2 ILI9806 IPS Display"
#define TFT_SPEED				24
#elif USE_FLEXTFT_NT35516
#define TFT_WIDTH				960
#define TFT_HEIGHT				540
#define CFG_STRING				"960x540*2 NT35516 IPS Display"
#define TFT_SPEED				22
#define COL_CLAMP_MIN			0
//#define COL_CLAMP_MAX			246
#endif


// Pins
#define TFT_RS					10		// Register select (DC)
#define TFT_RST					11		// Reset
#define TFT_CS					12		// Chip select
#define TFT_BL					33		// Backlight
#define TFT_INTENSITY			50		// PWM value from 0 to 255
// Read is 37
// Write is 36



#if (USE_DMAMEM_BUFFER)
#define STORAGETYPE DMAMEM 
#elif (USE_EXTMEM_BUFFER)
#define STORAGETYPE EXTMEM 
#else
#define STORAGETYPE  
#endif


#include "displays.h"


#endif

