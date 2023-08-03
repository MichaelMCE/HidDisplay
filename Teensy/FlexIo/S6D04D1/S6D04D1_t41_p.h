#ifndef _S6D04D1_t41_p_H_
#define _S6D04D1_t41_p_H_

#if USE_FLEXTFT_S6D04D1

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



#define S6D04D1_TFTWIDTH   	TFT_WIDTH	// S6D04D1 TFT width in default rotation
#define S6D04D1_TFTHEIGHT  	TFT_HEIGHT	// S6D04D1 TFT height in default rotation 

#define S6D04D1_PWRCTL		0xF3
#define S6D04D1_VCMCTL		0xF4
#define S6D04D1_SRCCTL		0xF5
#define S6D04D1_SLPOUT		0x11
#define S6D04D1_MADCTL		0x36
#define S6D04D1_COLMOD		0x3A
#define S6D04D1_DISCTL		0xF2
#define S6D04D1_IFCTL		0xF6
#define S6D04D1_GATECTL		0xFD
#define S6D04D1_CASET		0x2A
#define S6D04D1_PASET		0x2B
#define S6D04D1_RAMWR		0x2C
#define S6D04D1_RAMRD		0x2E
#define S6D04D1_DISPON		0x29
#define S6D04D1_DISPOFF		0x28
#define S6D04D1_SLPIN		0x10
                	
#define S6D04D1_RPGAMCTL	0xF7
#define S6D04D1_WRDISBV		0x51
#define S6D04D1_WRCABCMB	0x5E
#define S6D04D1_MIECTL1		0xCA
#define S6D04D1_BCMODE		0xCB
#define S6D04D1_MIECTL2		0xCC
#define S6D04D1_MIDCTL3		0xCD
#define S6D04D1_RNGAMCTL	0xF8
#define S6D04D1_GPGAMCTL	0xF9
#define S6D04D1_GNGAMCTL	0xFA
#define S6D04D1_BPGAMCTL	0xFB
#define S6D04D1_BNGAMCTL	0xFC
#define S6D04D1_DCON		0xD9
#define S6D04D1_WRPWD		0xF0
#define S6D04D1_EDSTEST		0xFF
#define S6D04D1_TEON		0x35
#define S6D04D1_MDDICTL1	0xE0
#define S6D04D1_WRCTRLD		0x53
#define S6D04D1_WRCABC		0x55



typedef struct _setting_table {
    uint8_t command;
    uint8_t parameters;
    uint8_t parameter[15];
    uint8_t wait;
}setting_table_t;


#define MADCTL_RGB 0x00  // Red-Green-Blue pixel order
#define MADCTL_GS  0x01
#define MADCTL_SS  0x02
#define MADCTL_MH  0x04  // LCD refresh right to left
#define MADCTL_BGR 0x08  // Blue-Green-Red pixel order
#define MADCTL_ML  0x10  // LCD refresh Bottom to top
#define MADCTL_MV  0x20  // Row/Column exchange
#define MADCTL_MX  0x40  // Right to left
#define MADCTL_MY  0x80  // Bottom to top



//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#define MADCTL_ARRAY {	MADCTL_MX | MADCTL_BGR,								\
						MADCTL_MV | MADCTL_BGR,								\
						MADCTL_MY | MADCTL_BGR,								\
						MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR}


#ifdef __cplusplus
class S6D04D1_t41_p {
  public:
    S6D04D1_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
    void begin (const uint8_t baud_div = 20);

	void LCDSettingTableWrite (const setting_table_t *table);
	void init_display ();
    void displayInfo ();
    void setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void setBacklight (const uint8_t value);
    
    void pushPixels16bit (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void pushPixels16bitAsync (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

	void pushPixels (const uint16_t *pixels, uint32_t total, const int flags);


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
    void flexIRQ_Callback();

    bool isCB = false;
    void _onCompleteCB();
    
    static S6D04D1_t41_p *IRQcallback;
    
};
#endif //__cplusplus

#endif

#endif //_IS6D04D1_t41_p_H_
