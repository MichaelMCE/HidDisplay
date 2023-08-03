#ifndef _R61529_t41_p_H_
#define _R61529_t41_p_H_

#if USE_FLEXTFT_R61529

#include "Arduino.h"
#include "FlexIO_t4.h"
 

#define BUS_WIDTH			16   	/*Available options are 8 or 16 */


#define SHIFTNUM			8		// number of shifters used (up to 8)
#define BYTES_PER_BEAT		(sizeof(uint8_t))
#define BEATS_PER_SHIFTER 	(sizeof(uint32_t)/BYTES_PER_BEAT)
#define BYTES_PER_BURST		(sizeof(uint32_t)*SHIFTNUM)
#define SHIFTER_IRQ 		(SHIFTNUM-1)
#define TIMER_IRQ 			0
#define FLEXIO_ISR_PRIORITY 64		// interrupt is timing sensitive, so use relatively high priority (supersedes USB)


#define R61529_TFTWIDTH   	TFT_WIDTH	// R61529 TFT width in default rotation
#define R61529_TFTHEIGHT  	TFT_HEIGHT	// R61529 TFT height in default rotation 



#define R61529_NOP			0x00	// No Op, also reset write state (acording to PDF)
#define R61529_RESET		0x01	// Software reset
#define R61529_SLPOUT		0x11	// Sleep out (Exit sleep)
#define R61529_DISLYON		0x29	// Display ON
#define R61529_CASET		0x2A	// Column Address Set 
#define R61529_PASET		0x2B	// Page Address Set 
#define R61529_RAMWR		0x2C	// Memory Write 
#define R61529_RAMRD		0x2E	// Memory Read
#define R61529_MADCTL		0x36	// Memory Address Control
#define R61529_SETPIXF		0x3A	// Set Pixel Format
#define R61529_MCAP			0xB0	// Manufacturer Command Access Protect
#define R61529_BLCTRL1		0xB8	// Backlight Control 1
#define R61529_BLCTRL2		0xB9	// Backlight Control 2
#define R61529_BLCTRL3		0xBA	// Backlight Control 3
#define R61529_FMAIS		0xB3	// Frame Memory Access and Interface Setting
#define R61529_DISMODE		0xB4	// Display Mode
#define R61529_PANDSET		0xC0	// Panel Driving Setting;
#define R61529_DTSNM		0xC1	// Display Timing Setting for Normal Mode
#define R61529_SGDTSET		0xC4	// Source/Gate Driving Timing Setting
#define R61529_DPIPCTL		0xC6	// DPI polarity control
#define R61529_GAMSETA		0xC8	// Gamma Setting A Set
#define R61529_GAMSETB		0xC9	// Gamma Setting B Set
#define R61529_GAMSETC		0xCA	// Gamma Setting C Set
#define R61529_PSCPSET		0xD0	// Power Setting Charge Pump Setting
#define R61529_VCOMSET		0xD1	// VCOM Setting
#define R61529_NVMACTL		0xE0	// NVM Access Control
#define R61529_DDBWCTL		0xE1	// Set DDB Write Control
#define R61529_NVMLCTL		0xE2	// NVM Load Control


#define MADCTL_RGB 0x00  // Red-Green-Blue pixel order
#define MADCTL_GS  0x01
#define MADCTL_SS  0x02
#define MADCTL_MH  0x04  // LCD refresh right to left
#define MADCTL_BGR 0x08  // Blue-Green-Red pixel order
#define MADCTL_ML  0x10  // LCD refresh Bottom to top
#define MADCTL_MV  0x20  // Row/Column exchange
#define MADCTL_MX  0x40  // Right to left
#define MADCTL_MY  0x80  // Bottom to top

// Set default pixel and rotation
#define R61529_IXFORMAT	(MADCTL_MY | MADCTL_MV | MADCTL_RGB)


typedef struct _setting_table {
    uint8_t command;
    uint8_t parameters;
    uint8_t parameter[24];
    uint8_t wait;
}setting_table_t;




//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#define MADCTL_ARRAY {	MADCTL_MX | MADCTL_BGR,								\
						MADCTL_MV | MADCTL_BGR,								\
						MADCTL_MY | MADCTL_BGR,								\
						MADCTL_MX /*| MADCTL_MY*/ | MADCTL_MV | MADCTL_BGR}


#ifdef __cplusplus
class R61529_t41_p {
  public:
    R61529_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
    void begin (const uint8_t baud_div = 20);

	void LCDSettingTableWrite (const setting_table_t *table);
	void init_display ();
    void displayInfo ();
    void setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void setBacklight (const uint8_t value);
    
    void pushPixels16bit (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void pushPixels16bitAsync (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    uint8_t _rotation = 0;
    const uint8_t MADCTL[5] = MADCTL_ARRAY;
    void setRotation (const uint8_t r);

    typedef void(*CBF)();
    CBF _callback;
    void onCompleteCB (CBF callback);
    
    
    
  private:

	FlexIOHandler *pFlex;
	IMXRT_FLEXIO_t *p;
	const FlexIOHandler::FLEXIO_Hardware_t *hw;
   
    uint8_t _baud_div = 20; 

    int16_t _width, _height;
    int8_t  _dc, _cs, _rst, _bl;

    uint8_t _dummy;
    uint8_t _curMADCTL;

    uint16_t _lastx1, _lastx2, _lasty1, _lasty2;

    volatile bool WR_IRQTransferDone = true;
    uint32_t MulBeatCountRemain;
    uint16_t *MulBeatDataRemain;
    uint32_t TotalSize; 

    /* variables used by ISR */
    volatile uint32_t bytes_remaining;
    volatile unsigned int bursts_to_complete;
    volatile uint32_t *readPtr;
    uint32_t finalBurstBuffer[SHIFTNUM];

    void displayInit();
    void CSLow();
    void CSHigh();
    void DCLow();
    void DCHigh();
    void gpioWrite();
    void gpioRead();
    
    void FlexIO_Init();
    void FlexIO_Config_SnglBeat();
    void FlexIO_Config_MultiBeat();
    void FlexIO_Config_SnglBeat_Read();

    void SglBeatWR_nPrm_8(uint32_t const cmd, uint8_t const *value , uint32_t const length);
    void SglBeatWR_nPrm_16(uint32_t const cmd, const uint16_t *value, uint32_t const length);
    void MulBeatWR_nPrm_IRQ(uint32_t const cmd,  const void *value, uint32_t const length);

    void SglBeatRD_nPrm_8(uint32_t const cmd, uint8_t const *value , uint32_t const length);
    
    void microSecondDelay();
    
    uint8_t readCommand(const uint16_t cmd);

    static void ISR();
    void flexIRQ_Callback();

    bool isCB = false;
    void _onCompleteCB();
    
    static R61529_t41_p *IRQcallback;
    
};
#endif //__cplusplus

#endif

#endif //_IR61529_t41_p_H_
