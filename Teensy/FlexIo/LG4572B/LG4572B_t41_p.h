#ifndef _LG4572B_t41_p_H_
#define _LG4572B_t41_p_H_

#if USE_FLEXTFT_LG4572B

#include "Arduino.h"
#include "FlexIO_t4.h"
 

#define SHIFTNUM			8		// number of shifters used (up to 8)
#define BYTES_PER_BEAT		(sizeof(uint8_t))
#define BEATS_PER_SHIFTER	(sizeof(uint32_t)/BYTES_PER_BEAT)
#define BYTES_PER_BURST		(sizeof(uint32_t)*SHIFTNUM)
#define SHIFTER_IRQ			(SHIFTNUM-1)
#define TIMER_IRQ			0
#define FLEXIO_ISR_PRIORITY 64		// interrupt is timing sensitive, so use relatively high priority (supersedes USB)


#define LG4572B_TFTWIDTH   	TFT_WIDTH		// LG4572B TFT width in default rotation
#define LG4572B_TFTHEIGHT  	TFT_HEIGHT		// LG4572B TFT height in default rotation 

/*
#define LG4572B_R480x864	0x80
#define LG4572B_R480x854	0x81
#define LG4572B_R480x640	0x82
#define LG4572B_R480x720	0x84
*/

#define LG4572B_NOP			0x00	// No Op, also reset write state (acording to PDF)
#define LG4572B_RESET		0x01	// Software reset
#define LG4572B_SLPOUT		0x11	// Sleep out (Exit sleep)
#define LG4572B_DISLYON		0x29	// Display ON
#define LG4572B_CASET       0x2A	// Column Address Set 
#define LG4572B_PASET       0x2B	// Page Address Set 
#define LG4572B_RAMWR       0x2C	// Memory Write 
#define LG4572B_RAMRD       0x2E	// Memory Read
#define LG4572B_TREFLON		0x35	// Tearing Effect Line ON 
#define LG4572B_MADCTL		0x36	// or 0xB9 - PANELCTRL.  doesn't respond to 0x36 dispite pdf indicating so
#define LG4572B_VSCROLLS	0x37	// Vertical Scrolling Start Address
#define LG4572B_IPIXFOR		0x3A	// Interface Pixel Format
#define LG4572B_FRCTRL1		0xB1	// Frame Rate Control 1
#define LG4572B_FRCTRL2		0xB2	// Frame Rate Control 2
#define LG4572B_DINVCTRL	0xB4	// Display Inversion Control	
#define LG4572B_GIP1		0xBC	// GIP 1
#define LG4572B_GIP2		0xBD	// GIP 2
#define LG4572B_GIP3		0xBE	// GIP 3
#define LG4572B_PCTRL1		0xC0	// Power Control 1
#define LG4572B_PCTRL2		0xC1	// Power Control 2
#define LG4572B_VCCTRL1		0xC7	// VCOM Control 1
#define LG4572B_BLCTRL1		0xC8	// Backlight Control 2
#define LG4572B_ENGSET		0xDF	// Engineering Setting
#define LG4572B_PGCTRL		0xE0	// Positive Gamma Control
#define LG4572B_NGCTRL		0xE1	// Negative Gamma Control
#define LG4572B_VOLTMSET	0xED	// VGMP / VGMN / VGSP / VGSN Voltage Measurement Set
#define LG4572B_PTCTRL1		0xF1	// Panel Timing Control 1
#define LG4572B_PTCTRL2		0xF2	// Panel Timing Control 2
#define LG4572B_DVDDVSET	0xF3	// DVDD Voltage Setting
#define LG4572B_PRSSET		0xF7	// Panel Resolution Selection Set
#define LG4572B_LVGLVSET	0xFC	// LVGL Voltage Setting
#define LG4572B_EXTCCSET	0xFF

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
#define LG4572B_IXFORMAT	(MADCTL_MY | MADCTL_MV | MADCTL_RGB)


typedef struct _setting_table {
    uint8_t command;
    uint8_t parameters;
    uint8_t parameter[35];
    uint8_t wait;
}setting_table_t;




//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#define MADCTL_ARRAY {	MADCTL_MX | MADCTL_RGB,								\
						MADCTL_MV | MADCTL_RGB,								\
						MADCTL_MY | MADCTL_RGB,								\
						/*MADCTL_MX |*/ MADCTL_MY | MADCTL_MV | MADCTL_RGB}


#ifdef __cplusplus
class LG4572B_t41_p {
  public:
    LG4572B_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
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
    
    void applyCommands (const uint8_t *commands);

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

    static void ISR();
    void flexIRQ_Callback();

    bool isCB = false;
    void _onCompleteCB();
    
    static LG4572B_t41_p *IRQcallback;
    
};
#endif //__cplusplus

#endif

#endif //_ILG4572B_t41_p_H_
