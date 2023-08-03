#ifndef _ILI9806_t41_p_H_
#define _ILI9806_t41_p_H_

#if USE_FLEXTFT_ILI9806

#include "Arduino.h"
#include "FlexIO_t4.h"
 

#define BUS_WIDTH			16   	// Available options are 8 or 16
#define SHIFTNUM			8		// number of shifters used (up to 8)
#define BYTES_PER_BEAT		(sizeof(uint8_t))
#define BEATS_PER_SHIFTER	(sizeof(uint32_t)/BYTES_PER_BEAT)
#define BYTES_PER_BURST		(sizeof(uint32_t)*SHIFTNUM)
#define SHIFTER_IRQ			(SHIFTNUM-1)
#define TIMER_IRQ			0
#define FLEXIO_ISR_PRIORITY 64		// interrupt is timing sensitive, so use relatively high priority (supersedes USB)


#define ILI9806_TFTWIDTH   	TFT_WIDTH	// ILI9806 TFT width in default rotation
#define ILI9806_TFTHEIGHT  	TFT_HEIGHT	// ILI9806 TFT height in default rotation 

#define ILI9806_R480x864	0x80
#define ILI9806_R480x854	0x81
#define ILI9806_R480x640	0x82
#define ILI9806_R480x720	0x84


#define ILI9806_NOP			0x00	// No Op, also reset write state (acording to PDF)
#define ILI9806_SOFTRST		0x01	// Software reset
#define ILI9806_SLPOUT		0x11	// Sleep out (Exit sleep)
#define ILI9806_DISLYON		0x29	// Display ON
#define ILI9806_CASET       0x2A	// Column Address Set 
#define ILI9806_PASET       0x2B	// Page Address Set 
#define ILI9806_RAMWR       0x2C	// Memory Write 
#define ILI9806_RAMRD       0x2E	// Memory Read
#define ILI9806_TREFLON		0x35	// Tearing Effect Line ON 
#define ILI9806_VSCROLLS	0x37	// Vertical Scrolling Start Address
#define ILI9806_IPIXFOR		0x3A	// Interface Pixel Format
#define ILI9806_FRCTRL1		0xB1	// Frame Rate Control 1
#define ILI9806_FRCTRL2		0xB2	// Frame Rate Control 2
#define ILI9806_DINVCTRL	0xB4	// Display Inversion Control	
#define ILI9806_MADCTL		0xB9	// PANELCTRL.  doesn't respond to 0x36 dispite pdf indicating so
#define ILI9806_GIP1		0xBC	// GIP 1
#define ILI9806_GIP2		0xBD	// GIP 2
#define ILI9806_GIP3		0xBE	// GIP 3
#define ILI9806_PCTRL1		0xC0	// Power Control 1
#define ILI9806_PCTRL2		0xC1	// Power Control 2
#define ILI9806_VCCTRL1		0xC7	// VCOM Control 1
#define ILI9806_BLCTRL1		0xC8	// Backlight Control 2
#define ILI9806_ENGSET		0xDF	// Engineering Setting
#define ILI9806_PGCTRL		0xE0	// Positive Gamma Control
#define ILI9806_NGCTRL		0xE1	// Negative Gamma Control
#define ILI9806_VOLTMSET	0xED	// VGMP / VGMN / VGSP / VGSN Voltage Measurement Set
#define ILI9806_PTCTRL1		0xF1	// Panel Timing Control 1
#define ILI9806_PTCTRL2		0xF2	// Panel Timing Control 2
#define ILI9806_DVDDVSET	0xF3	// DVDD Voltage Setting
#define ILI9806_PRSSET		0xF7	// Panel Resolution Selection Set
#define ILI9806_LVGLVSET	0xFC	// LVGL Voltage Setting
#define ILI9806_EXTCCSET	0xFF

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
#define ILI9806_IXFORMAT	(MADCTL_MY | MADCTL_MV | MADCTL_RGB)


typedef struct _setting_table {
    uint8_t command;
    uint8_t parameters;
    uint8_t parameter[21];
    uint8_t wait;
}setting_table_t;




//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#define MADCTL_ARRAY {	MADCTL_MX | MADCTL_RGB,								\
						MADCTL_MV | MADCTL_RGB,								\
						MADCTL_MY | MADCTL_RGB,								\
						/*MADCTL_MX |*/ MADCTL_MY | MADCTL_MV | MADCTL_RGB}


#ifdef __cplusplus
class ILI9806_t41_p {
  public:
    ILI9806_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
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
    uint8_t readCommand (const uint16_t cmd);

    static void ISR();
    void FlexIRQ_Callback();

    bool isCB = false;
    void _onCompleteCB();
    
    static ILI9806_t41_p *IRQcallback;
    
};
#endif //__cplusplus

#endif

#endif //_IILI9806_t41_p_H_
