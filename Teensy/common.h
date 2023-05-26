
#ifndef _COMMON_H_
#define _COMMON_H_

#define USE_DMAMEM_BUFFER		0		// set this to 1 to use DMAMEM for the tft buffer
#define USE_EXTMEM_BUFFER		0		// set this to 1 to use PSRAM for the tft buffer
#define USE_EXTMEM_ALLOC		0		// use this to divert uFont allocations to PSRAM. Beaware - this will cost 32k of RAM1


#define USE_FLEXTFT_ILI948x		1
#define USE_FLEXTFT_S6D04D1		0


#if USE_FLEXTFT_ILI948x
#define TFT_WIDTH				480
#define TFT_HEIGHT				320
#else
#define TFT_WIDTH				432
#define TFT_HEIGHT				240
#endif


#define TFT_RS					10		// register select
#define TFT_RST					11		// reset
#define TFT_CS					12		// chip select
#define TFT_BL					33		// backlight
#define TFT_INTENSITY			50		// PWM value from 0 to 255
#define TFT_SPEED				60		// Mhz


#if (USE_DMAMEM_BUFFER)
#define STORAGETYPE DMAMEM 
#elif (USE_EXTMEM_BUFFER)
#define STORAGETYPE EXTMEM 
#else
#define STORAGETYPE  
#endif


#include "ILI948x/ILI948x_t41_p.h"
#include "S6D04D1/S6D04D1_t41_p.h"
#include "displays.h"



#endif

