#ifndef _RM68120_t41_p_H_
#define _RM68120_t41_p_H_

#if USE_FLEXTFT_RM68120


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


#define RM68120_TFTWIDTH   TFT_WIDTH	// RM68120 TFT width in default rotation
#define RM68120_TFTHEIGHT  TFT_HEIGHT	// RM68120 TFT height in default rotation


#define RM68120_CASET   0x2A00
#define RM68120_RASET   0x2B00
#define RM68120_RAMWR   0x2C00
#define RM68120_MADCTL  0x3600
#define RM68120_CASET   0x2A00
#define RM68120_RASET   0x2B00
#define RM68120_RAMWR   0x2C00
#define RM68120_MADCTL  0x3600
#define RM68120_COLMOD	0x3A00

#define MADCTL_MY  0x80  // Bottom to top
#define MADCTL_MX  0x40  // Right to left
#define MADCTL_MV  0x20  // Row/Column exchange
#define MADCTL_ML  0x10  // LCD refresh Bottom to top
#define MADCTL_RGB 0x00  // Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  // Blue-Green-Red pixel order
#define MADCTL_MH  0x04  // LCD refresh right to left
#define MADCTL_GS  0x01
#define MADCTL_SS  0x02

//MADCTL 0,1,2,3 for setting rotation and 4 for screenshot
#define MADCTL_ARRAY { 	MADCTL_MX | MADCTL_RGB, 							\
						MADCTL_MV | MADCTL_RGB, 							\
						MADCTL_MY | MADCTL_RGB,								\
						/*MADCTL_MX |*/ MADCTL_MY | MADCTL_MV | MADCTL_RGB } 

#ifdef __cplusplus


class RM68120_t41_p {
  public:
    RM68120_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
    void begin(uint8_t baud_div = 20);
    uint8_t getBusSpd();


    uint8_t setBitDepth(uint8_t bitDepth);
    uint8_t getBitDepth();

    void setFrameRate(uint8_t frRate);
    uint8_t getFrameRate();

    void setTearingEffect(bool tearingOn);
    bool getTearingEffect();
	void setBacklight (const uint8_t value);
	
	void sendCmd16Arg8 (const uint16_t cmd, const uint8_t arg);
	void sendCmd16 (const uint16_t cmd);

    void setTearingScanLine(uint16_t scanLine);
    uint16_t getTearingScanLine();

    void setRotation(uint8_t r);
    void invertDisplay(bool invert);
    void displayInfo();
    void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    void pushPixels16bit (uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void pushPixels16bitAsync (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    uint8_t readCommand(const uint16_t cmd);
    

    typedef void(*CBF)();
    CBF _callback;
    void onCompleteCB(CBF callback);


  
    
  private:

  	FlexIOHandler *pFlex;
  	IMXRT_FLEXIO_t *p;
  	const FlexIOHandler::FLEXIO_Hardware_t *hw;
   
    uint8_t _baud_div = 20; 

    uint8_t _bitDepth = 16;
    uint8_t _rotation = 0;
    const uint8_t MADCTL[4] = MADCTL_ARRAY;

    uint8_t _frameRate = 60;

    bool _bTearingOn = false;
    uint16_t _tearingScanLine = 0;

    int16_t _width, _height;
    int8_t  _dc, _cs, _rst, _bl;
    const uint8_t * _init_commands;

    uint8_t _dummy;
    uint8_t _curMADCTL;

    uint16_t _lastx1, _lastx2, _lasty1, _lasty2 = 9999;

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
    void setRegisters ();
    void setRegisters1 ();
    void setRegisters2 ();
    
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

    void SglBeatWR_nPrm_8  (uint32_t const cmd, uint8_t const *value,  uint32_t const length);
    //void sglBeatWR_nPrm_16 (uint32_t const cmd, const uint16_t *value, uint32_t const length);
    void sglBeatWR_nPrm_16 (uint32_t const cmd, uint16_t *value, const uint32_t length);
    void MulBeatWR_nPrm_IRQ(uint32_t const cmd, const void *value, uint32_t const length);
    
    void microSecondDelay();

    static void ISR();
    void flexIRQ_Callback();

    bool isCB = false;
    void _onCompleteCB();
    
    static RM68120_t41_p *IRQcallback;
    
};
#endif //__cplusplus
#endif //_IRM68120_t41_p_H_
#endif
