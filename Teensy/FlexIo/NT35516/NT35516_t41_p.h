#ifndef _NT35516_t41_p_H_
#define _NT35516_t41_p_H_

#if USE_FLEXTFT_NT35516

#include "Arduino.h"
#include "FlexIO_t4.h"
 

#define BUS_WIDTH			16   		// Available options are 8 or 16
#define NT35516_TFTWIDTH   	TFT_WIDTH	// NT35516 TFT width in default rotation
#define NT35516_TFTHEIGHT  	TFT_HEIGHT	// NT35516 TFT height in default rotation 

#define NT35516_CASET		0x2A00
#define NT35516_RASET		0x2B00
#define NT35516_RAMWR		0x2C00
#define NT35516_MADCTL		0x3600
#define NT35516_COLMOD		0x3A00  
#define NT35516_SLPOUT		0x1100
#define NT35516_DISPON		0x2900

#define MADCTL_RGB		0x00  // Red-Green-Blue pixel order
#define MADCTL_RSMY		0x01  // Flips the display image top to down
#define MADCTL_RSMX		0x02  // Flips the display image left to right
#define MADCTL_MH		0x04  // Horizontal refresh direction control. LCD refresh right to left
#define MADCTL_BGR		0x08  // Blue-Green-Red pixel order
#define MADCTL_ML		0x10  // Vertical refresh direction control. LCD refresh Bottom to top
#define MADCTL_MV		0x20  // Row/Column Exchange
#define MADCTL_MX		0x40  // Column Address Order. Right to left
#define MADCTL_MY		0x80  // Row Address Order. Bottom to top

// Set default pixel and rotation
#define NT35516_IXFORMAT	(/*MADCTL_MX |*/ MADCTL_MV | MADCTL_RGB | MADCTL_RSMX)


//MADCTL
#define MADCTL_ARRAY {	MADCTL_MX | MADCTL_RGB,								\
						MADCTL_MV | MADCTL_RGB,								\
						MADCTL_MY | MADCTL_RGB,								\
						NT35516_IXFORMAT}




typedef struct _LCM_setting_table {
    uint16_t cmd;
    uint8_t count;
    uint8_t args[35];
}cmdTable_t;



#ifdef __cplusplus
class NT35516_t41_p {
  public:
    NT35516_t41_p (int8_t dc, int8_t cs = -1, int8_t rst = -1, int8_t bl = 33);
    void begin (const uint8_t baud_div = 20);

    void setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void setBacklight (const uint8_t value);
    
    void pushPixels16bit (uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    void pushPixels16bitAsync (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    uint8_t _rotation = 0;
    const uint8_t MADCTL[4] = MADCTL_ARRAY;
    void setRotation (const uint8_t r);

    
  private:
	FlexIOHandler *pFlex;
	IMXRT_FLEXIO_t *p;
	const FlexIOHandler::FLEXIO_Hardware_t *hw;

    int16_t _width, _height;
    int8_t _dc, _cs, _rst, _bl;
    uint8_t _baud_div = 20; 

    void CSLow ();
    void CSHigh ();
    void DCLow ();
    void DCHigh ();
    void gpioWrite ();

	void display_init ();
    void setDisplayRegisters (const cmdTable_t *table);
   	void sendCmd16Arg8 (const uint16_t cmd, const uint8_t arg);
	void sendCmd16 (const uint16_t cmd);
    
    void flexIO_init ();
    void flexIO_Config_snglBeat ();
    void sglBeatWR_nPrm_16 (uint32_t const cmd, uint16_t *value, uint32_t const length);
   
};
#endif //__cplusplus

#endif

#endif //_INT35516_t41_p_H_
