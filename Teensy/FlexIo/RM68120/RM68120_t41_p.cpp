
/*

	  -: Compile with :-
	Optimisation: -02 / Faster
	Speed: 720mhz
	TFT_SPEED: 40Mhz (Baud)
	USE_STRIP_RENDERER: 1
	STRIP_RENDERER_HEIGHT: 30
	COL_CLAMP_MIN: Don't define or set to 0
	COL_CLAMP_MAX: Don't define or set to 0


*/


#include "RM68120_t41_p.h"

#if USE_FLEXTFT_RM68120


static RM68120_t41_p STORAGETYPE lcd = RM68120_t41_p(TFT_RS, TFT_CS, TFT_RST, TFT_BL);




FASTRUN void RM68120_t41_p::sendCmd16Arg8 (const uint16_t cmd, const uint8_t arg)
{
	uint16_t arg16[2] = {arg, 0};

   	sglBeatWR_nPrm_16(cmd, arg16, 1);
}

FASTRUN void RM68120_t41_p::sendCmd16 (const uint16_t cmd)
{
	uint16_t arg16[2] = {0, 0};

   	sglBeatWR_nPrm_16(cmd, arg16, 1);
}

FLASHMEM RM68120_t41_p::RM68120_t41_p (int8_t dc, int8_t cs, int8_t rst, int8_t bl) 
{  
	_dc = dc;
	_cs = cs;
	_rst = rst;
	_bl = bl;
}

FLASHMEM void RM68120_t41_p::setBacklight (const uint8_t value)
{
    analogWrite(_bl, value&0x7F);
}

FLASHMEM void RM68120_t41_p::begin (uint8_t baud_div) 
{
	switch (baud_div){					// with a base freq of 240mhz
	case 2:  _baud_div = 120; break;
	case 4:  _baud_div = 60; break;
	case 8:  _baud_div = 30; break;
	case 12: _baud_div = 20; break;
	case 13: _baud_div = 18; break;
	case 14: _baud_div = 17; break;
	case 15: _baud_div = 16; break;
	case 16: _baud_div = 14; break;
	case 18: _baud_div = 13; break;
	case 20: _baud_div = 12; break;
	case 22: _baud_div = 11; break;
	case 24: _baud_div = 10; break;
	case 27: _baud_div = 9; break;
	case 30: _baud_div = 8; break;
	case 34: _baud_div = 7; break;
	case 40: _baud_div = 6; break;
	case 48: _baud_div = 5; break;
	case 60: _baud_div = 4; break;
	case 80: _baud_div = 3; break;
	case 120:_baud_div = 2; break;
	case 240:_baud_div = 1; break;
	default: _baud_div = 20; break;
	}
	 
	pinMode(_cs, OUTPUT);	// CS / Chip Select
	pinMode(_dc, OUTPUT);	// DC / Register Select
	pinMode(_rst, OUTPUT);	// Reset
	pinMode(_bl, OUTPUT);	// Backlight
	
	*(portControlRegister(_cs)) = 0xFF;
	*(portControlRegister(_dc)) = 0xFF;
	*(portControlRegister(_rst)) = 0xFF;
	
	digitalWriteFast(_cs, HIGH);
	digitalWriteFast(_dc, HIGH);
	digitalWriteFast(_rst, HIGH);
	
	delay(15);
	digitalWrite(_rst, LOW);
	delay(15);
	digitalWriteFast(_rst, HIGH);
	delay(100);
	
	FlexIO_Init();
	FlexIO_Config_SnglBeat();
	displayInit();
	
	setBitDepth(_bitDepth);
	setRotation(3);
  
  
  
  /*
  setBitDepth(_bitDepth);

  setTearingEffect(_bTearingOn);
  if (_bTearingOn == true) {
    setTearingScanLine(_tearingScanLine);
  }  
  setFrameRate(_frameRate);
  */
 
  _width  = RM68120_TFTWIDTH;
  _height = RM68120_TFTHEIGHT;

}


FLASHMEM uint8_t RM68120_t41_p::setBitDepth (uint8_t bitDepth)  
{
	uint8_t bd;

	switch (bitDepth) {
    case 18:  _bitDepth = 18;
		bd = 0x66;
		break;
    case 24:  _bitDepth = 24;
		bd = 0x77;
		break;
    case 16:	// fallthrough
    default:  _bitDepth = 16;
		bd = 0x55;
		break;
	}

	gpioWrite();
	sendCmd16Arg8(RM68120_COLMOD, bd);
	delay(1);

	return _bitDepth;
}

FLASHMEM uint8_t RM68120_t41_p::getBitDepth()
{
	return _bitDepth;
}

#if 0
FLASHMEM void RM68120_t41_p::setFrameRate(uint8_t frRate) 
{
	_frameRate = frRate;
	
	uint8_t fr28Hz[2] = {0x00, 0x11}; // 28.78fps, 17 clocks
	uint8_t fr30Hz[2] = {0x10, 0x11}; // 30.38fps, 17 clocks
	uint8_t fr39Hz[2] = {0x50, 0x11}; // 39.06fps, 17 clocks
	uint8_t fr45Hz[2] = {0x70, 0x11}; // 45.57fps, 17 clocks
	uint8_t fr54Hz[2] = {0x90, 0x11}; // 54.69ps,  17 clocks
	uint8_t fr60Hz[2] = {0xA0, 0x11}; // 60.76fps, 17 clocks
	uint8_t fr68Hz[2] = {0xB0, 0x11}; // 68.36fps, 17 clocks (RM68120 default)
	uint8_t fr78Hz[2] = {0xC0, 0x11}; // 78.13fps, 17 clocks
	uint8_t fr91Hz[2] = {0xD0, 0x11}; // 91.15fps, 17 clocks
	
	 uint8_t frData[2];
	switch (frRate) {
	case 28: memcpy(frData, fr28Hz, sizeof fr28Hz); break;
	case 30: memcpy(frData, fr30Hz, sizeof fr30Hz); break;
	case 39: memcpy(frData, fr39Hz, sizeof fr39Hz); break;
	case 45: memcpy(frData, fr45Hz, sizeof fr45Hz); break;
	case 54: memcpy(frData, fr54Hz, sizeof fr54Hz); break;
	case 60: memcpy(frData, fr60Hz, sizeof fr60Hz); break;
	case 68: memcpy(frData, fr68Hz, sizeof fr68Hz); break;
	case 78: memcpy(frData, fr78Hz, sizeof fr78Hz); break;
	case 91: memcpy(frData, fr91Hz, sizeof fr91Hz); break;
	default: memcpy(frData, fr60Hz, sizeof fr60Hz); _frameRate = 60; break;
	}
	
	SglBeatWR_nPrm_8(RM68120_FRMCTR1, frData, 2);
}

FLASHMEM uint8_t RM68120_t41_p::getFrameRate()
{
	return _frameRate;
}
#endif

FLASHMEM void RM68120_t41_p::setTearingEffect (bool tearingOn)
{

/*  _bTearingOn = tearingOn;
  uint8_t mode = 0x00;
  
  CSLow();
  if (_bTearingOn == true) {
    SglBeatWR_nPrm_8(RM68120_TEON, &mode, 1);        //Tearing effect line on, mode 0 (V-Blanking)
  } else {
    SglBeatWR_nPrm_8(RM68120_TEOFF,0,0);
  }
  CSHigh();*/

}

FLASHMEM bool RM68120_t41_p::getTearingEffect ()
{
	return _bTearingOn;
}

FLASHMEM void RM68120_t41_p::setTearingScanLine (uint16_t scanLine)
{
 /* _tearingScanLine = scanLine;
  
  uint8_t params[2] = {(uint8_t)(_tearingScanLine << 8), (uint8_t)(_tearingScanLine & 0xFF)};
  SglBeatWR_nPrm_8(RM68120_TESLWR, params, 2);      //Tearing effect write scan line : 0x00 0x00 = line 0 (default), 0x00 0xA0 = line 160, 0x00 0xF0 = line 240
*/
}

FLASHMEM uint16_t RM68120_t41_p::getTearingScanLine()
{
  return _tearingScanLine;
}

FLASHMEM void RM68120_t41_p::setRotation (uint8_t r) 
{ 
  _rotation = r & 3;

  switch (_rotation) {
    case 0:	
    case 2: _width  = RM68120_TFTWIDTH;
            _height = RM68120_TFTHEIGHT;
            break;
    case 1: 
    case 3: _width  = RM68120_TFTHEIGHT;
            _height = RM68120_TFTWIDTH;
            break;
	}

		gpioWrite();
  sendCmd16Arg8(RM68120_MADCTL, MADCTL[_rotation]);
}

FLASHMEM void RM68120_t41_p::invertDisplay(bool invert) 
{
  //sendCmd16(invert ? RM68120_INVON : RM68120_INVOFF);
}


FASTRUN void RM68120_t41_p::setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
	gpioWrite();
	
	sendCmd16Arg8(RM68120_CASET + 0, x1 >> 8);
	sendCmd16Arg8(RM68120_CASET + 1, x1 & 0xFF);
	sendCmd16Arg8(RM68120_CASET + 2, x2 >> 8);
	sendCmd16Arg8(RM68120_CASET + 3, x2 & 0xFF);

	sendCmd16Arg8(RM68120_RASET + 0, y1 >> 8);
	sendCmd16Arg8(RM68120_RASET + 1, y1 & 0xFF);
	sendCmd16Arg8(RM68120_RASET + 2, y2 >> 8);
	sendCmd16Arg8(RM68120_RASET + 3, y2 & 0xFF);
}

FASTRUN void RM68120_t41_p::pushPixels16bit (uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
#if 0

	digitalWriteFast(36, HIGH);
	if (!((_lastx1 == x1) && (_lastx2 == x2) && (_lasty1 == y1) && (_lasty2 == y2))){
		setAddrWindow(x1, y1, x2, y2);
		_lastx1 = x1;  _lastx2 = x2;  _lasty1 = y1;  _lasty2 = y2;
	}
	digitalWriteFast(36, HIGH);
	
	uint32_t area = (x2-x1+1) * (y2-y1+1);
	sglBeatWR_nPrm_16(RM68120_RAMWR, pixels, area);
	digitalWriteFast(36, HIGH);

#else

	const int width = (x2 - x1) + 1;
	const int height = (y2 - y1) + 1;
	
	// refresh top down, line by line
	for (int i = 0; i < height; i++){
		setAddrWindow(x1, y1+i, x2, y1+i);
		digitalWriteFast(36, HIGH);
		
		sglBeatWR_nPrm_16(RM68120_RAMWR, &pixels[i * width], width);

		// seems to help stablize the controler before next row
		digitalWriteFast(36, HIGH);
		sendCmd16Arg8(0, 0);
		sendCmd16Arg8(0, 0);
		sendCmd16Arg8(0, 0);
		sendCmd16Arg8(0, 0);
		digitalWriteFast(36, HIGH);
	}
#endif
}


FLASHMEM void RM68120_t41_p::setRegisters () 
{

	gpioWrite();
    sendCmd16(0x0100); // Software Reset
    delay(120);

	sendCmd16Arg8(0xF000, 0x55);
    sendCmd16Arg8(0xF001, 0xAA);
    sendCmd16Arg8(0xF002, 0x52);
    sendCmd16Arg8(0xF003, 0x08);
    sendCmd16Arg8(0xF004, 0x01);

    //GAMMA SETING  RED
    sendCmd16Arg8(0xD100, 0x00);
    sendCmd16Arg8(0xD101, 0x00);
    sendCmd16Arg8(0xD102, 0x1b);
    sendCmd16Arg8(0xD103, 0x44);
    sendCmd16Arg8(0xD104, 0x62);
    sendCmd16Arg8(0xD105, 0x00);
    sendCmd16Arg8(0xD106, 0x7b);
    sendCmd16Arg8(0xD107, 0xa1);
    sendCmd16Arg8(0xD108, 0xc0);
    sendCmd16Arg8(0xD109, 0xee);
    sendCmd16Arg8(0xD10A, 0x55);
    sendCmd16Arg8(0xD10B, 0x10);
    sendCmd16Arg8(0xD10C, 0x2c);
    sendCmd16Arg8(0xD10D, 0x43);
    sendCmd16Arg8(0xD10E, 0x57);
    sendCmd16Arg8(0xD10F, 0x55);
    sendCmd16Arg8(0xD110, 0x68);
    sendCmd16Arg8(0xD111, 0x78);
    sendCmd16Arg8(0xD112, 0x87);
    sendCmd16Arg8(0xD113, 0x94);
    sendCmd16Arg8(0xD114, 0x55);
    sendCmd16Arg8(0xD115, 0xa0);
    sendCmd16Arg8(0xD116, 0xac);
    sendCmd16Arg8(0xD117, 0xb6);
    sendCmd16Arg8(0xD118, 0xc1);
    sendCmd16Arg8(0xD119, 0x55);
    sendCmd16Arg8(0xD11A, 0xcb);
    sendCmd16Arg8(0xD11B, 0xcd);
    sendCmd16Arg8(0xD11C, 0xd6);
    sendCmd16Arg8(0xD11D, 0xdf);
    sendCmd16Arg8(0xD11E, 0x95);
    sendCmd16Arg8(0xD11F, 0xe8);
    sendCmd16Arg8(0xD120, 0xf1);
    sendCmd16Arg8(0xD121, 0xfa);
    sendCmd16Arg8(0xD122, 0x02);
    sendCmd16Arg8(0xD123, 0xaa);
    sendCmd16Arg8(0xD124, 0x0b);
    sendCmd16Arg8(0xD125, 0x13);
    sendCmd16Arg8(0xD126, 0x1d);
    sendCmd16Arg8(0xD127, 0x26);
    sendCmd16Arg8(0xD128, 0xaa);
    sendCmd16Arg8(0xD129, 0x30);
    sendCmd16Arg8(0xD12A, 0x3c);
    sendCmd16Arg8(0xD12B, 0x4A);
    sendCmd16Arg8(0xD12C, 0x63);
    sendCmd16Arg8(0xD12D, 0xea);
    sendCmd16Arg8(0xD12E, 0x79);
    sendCmd16Arg8(0xD12F, 0xa6);
    sendCmd16Arg8(0xD130, 0xd0);
    sendCmd16Arg8(0xD131, 0x20);
    sendCmd16Arg8(0xD132, 0x0f);
    sendCmd16Arg8(0xD133, 0x8e);
    sendCmd16Arg8(0xD134, 0xff);
    
    //GAMMA SETING GREEN
    sendCmd16Arg8(0xD200, 0x00);
    sendCmd16Arg8(0xD201, 0x00);
    sendCmd16Arg8(0xD202, 0x1b);
    sendCmd16Arg8(0xD203, 0x44);
    sendCmd16Arg8(0xD204, 0x62);
    sendCmd16Arg8(0xD205, 0x00);
    sendCmd16Arg8(0xD206, 0x7b);
    sendCmd16Arg8(0xD207, 0xa1);
    sendCmd16Arg8(0xD208, 0xc0);
    sendCmd16Arg8(0xD209, 0xee);
    sendCmd16Arg8(0xD20A, 0x55);
    sendCmd16Arg8(0xD20B, 0x10);
    sendCmd16Arg8(0xD20C, 0x2c);
    sendCmd16Arg8(0xD20D, 0x43);
    sendCmd16Arg8(0xD20E, 0x57);
    sendCmd16Arg8(0xD20F, 0x55);
    sendCmd16Arg8(0xD210, 0x68);
    sendCmd16Arg8(0xD211, 0x78);
    sendCmd16Arg8(0xD212, 0x87);
    sendCmd16Arg8(0xD213, 0x94);
    sendCmd16Arg8(0xD214, 0x55);
    sendCmd16Arg8(0xD215, 0xa0);
    sendCmd16Arg8(0xD216, 0xac);
    sendCmd16Arg8(0xD217, 0xb6);
    sendCmd16Arg8(0xD218, 0xc1);
    sendCmd16Arg8(0xD219, 0x55);
    sendCmd16Arg8(0xD21A, 0xcb);
    sendCmd16Arg8(0xD21B, 0xcd);
    sendCmd16Arg8(0xD21C, 0xd6);
    sendCmd16Arg8(0xD21D, 0xdf);
    sendCmd16Arg8(0xD21E, 0x95);
    sendCmd16Arg8(0xD21F, 0xe8);
    sendCmd16Arg8(0xD220, 0xf1);
    sendCmd16Arg8(0xD221, 0xfa);
    sendCmd16Arg8(0xD222, 0x02);
    sendCmd16Arg8(0xD223, 0xaa);
    sendCmd16Arg8(0xD224, 0x0b);
    sendCmd16Arg8(0xD225, 0x13);
    sendCmd16Arg8(0xD226, 0x1d);
    sendCmd16Arg8(0xD227, 0x26);
    sendCmd16Arg8(0xD228, 0xaa);
    sendCmd16Arg8(0xD229, 0x30);
    sendCmd16Arg8(0xD22A, 0x3c);
    sendCmd16Arg8(0xD22B, 0x4a);
    sendCmd16Arg8(0xD22C, 0x63);
    sendCmd16Arg8(0xD22D, 0xea);
    sendCmd16Arg8(0xD22E, 0x79);
    sendCmd16Arg8(0xD22F, 0xa6);
    sendCmd16Arg8(0xD230, 0xd0);
    sendCmd16Arg8(0xD231, 0x20);
    sendCmd16Arg8(0xD232, 0x0f);
    sendCmd16Arg8(0xD233, 0x8e);
    sendCmd16Arg8(0xD234, 0xff);

    //GAMMA SETING BLUE
    sendCmd16Arg8(0xD300, 0x00);
    sendCmd16Arg8(0xD301, 0x00);
    sendCmd16Arg8(0xD302, 0x1b);
    sendCmd16Arg8(0xD303, 0x44);
    sendCmd16Arg8(0xD304, 0x62);
    sendCmd16Arg8(0xD305, 0x00);
    sendCmd16Arg8(0xD306, 0x7b);
    sendCmd16Arg8(0xD307, 0xa1);
    sendCmd16Arg8(0xD308, 0xc0);
    sendCmd16Arg8(0xD309, 0xee);
    sendCmd16Arg8(0xD30A, 0x55);
    sendCmd16Arg8(0xD30B, 0x10);
    sendCmd16Arg8(0xD30C, 0x2c);
    sendCmd16Arg8(0xD30D, 0x43);
    sendCmd16Arg8(0xD30E, 0x57);
    sendCmd16Arg8(0xD30F, 0x55);
    sendCmd16Arg8(0xD310, 0x68);
    sendCmd16Arg8(0xD311, 0x78);
    sendCmd16Arg8(0xD312, 0x87);
    sendCmd16Arg8(0xD313, 0x94);
    sendCmd16Arg8(0xD314, 0x55);
    sendCmd16Arg8(0xD315, 0xa0);
    sendCmd16Arg8(0xD316, 0xac);
    sendCmd16Arg8(0xD317, 0xb6);
    sendCmd16Arg8(0xD318, 0xc1);
    sendCmd16Arg8(0xD319, 0x55);
    sendCmd16Arg8(0xD31A, 0xcb);
    sendCmd16Arg8(0xD31B, 0xcd);
    sendCmd16Arg8(0xD31C, 0xd6);
    sendCmd16Arg8(0xD31D, 0xdf);
    sendCmd16Arg8(0xD31E, 0x95);
    sendCmd16Arg8(0xD31F, 0xe8);
    sendCmd16Arg8(0xD320, 0xf1);
    sendCmd16Arg8(0xD321, 0xfa);
    sendCmd16Arg8(0xD322, 0x02);
    sendCmd16Arg8(0xD323, 0xaa);
    sendCmd16Arg8(0xD324, 0x0b);
    sendCmd16Arg8(0xD325, 0x13);
    sendCmd16Arg8(0xD326, 0x1d);
    sendCmd16Arg8(0xD327, 0x26);
    sendCmd16Arg8(0xD328, 0xaa);
    sendCmd16Arg8(0xD329, 0x30);
    sendCmd16Arg8(0xD32A, 0x3c);
    sendCmd16Arg8(0xD32B, 0x4A);
    sendCmd16Arg8(0xD32C, 0x63);
    sendCmd16Arg8(0xD32D, 0xea);
    sendCmd16Arg8(0xD32E, 0x79);
    sendCmd16Arg8(0xD32F, 0xa6);
    sendCmd16Arg8(0xD330, 0xd0);
    sendCmd16Arg8(0xD331, 0x20);
    sendCmd16Arg8(0xD332, 0x0f);
    sendCmd16Arg8(0xD333, 0x8e);
    sendCmd16Arg8(0xD334, 0xff);


    //GAMMA SETING  RED
    sendCmd16Arg8(0xD400, 0x00);
    sendCmd16Arg8(0xD401, 0x00);
    sendCmd16Arg8(0xD402, 0x1b);
    sendCmd16Arg8(0xD403, 0x44);
    sendCmd16Arg8(0xD404, 0x62);
    sendCmd16Arg8(0xD405, 0x00);
    sendCmd16Arg8(0xD406, 0x7b);
    sendCmd16Arg8(0xD407, 0xa1);
    sendCmd16Arg8(0xD408, 0xc0);
    sendCmd16Arg8(0xD409, 0xee);
    sendCmd16Arg8(0xD40A, 0x55);
    sendCmd16Arg8(0xD40B, 0x10);
    sendCmd16Arg8(0xD40C, 0x2c);
    sendCmd16Arg8(0xD40D, 0x43);
    sendCmd16Arg8(0xD40E, 0x57);
    sendCmd16Arg8(0xD40F, 0x55);
    sendCmd16Arg8(0xD410, 0x68);
    sendCmd16Arg8(0xD411, 0x78);
    sendCmd16Arg8(0xD412, 0x87);
    sendCmd16Arg8(0xD413, 0x94);
    sendCmd16Arg8(0xD414, 0x55);
    sendCmd16Arg8(0xD415, 0xa0);
    sendCmd16Arg8(0xD416, 0xac);
    sendCmd16Arg8(0xD417, 0xb6);
    sendCmd16Arg8(0xD418, 0xc1);
    sendCmd16Arg8(0xD419, 0x55);
    sendCmd16Arg8(0xD41A, 0xcb);
    sendCmd16Arg8(0xD41B, 0xcd);
    sendCmd16Arg8(0xD41C, 0xd6);
    sendCmd16Arg8(0xD41D, 0xdf);
    sendCmd16Arg8(0xD41E, 0x95);
    sendCmd16Arg8(0xD41F, 0xe8);
    sendCmd16Arg8(0xD420, 0xf1);
    sendCmd16Arg8(0xD421, 0xfa);
    sendCmd16Arg8(0xD422, 0x02);
    sendCmd16Arg8(0xD423, 0xaa);
    sendCmd16Arg8(0xD424, 0x0b);
    sendCmd16Arg8(0xD425, 0x13);
    sendCmd16Arg8(0xD426, 0x1d);
    sendCmd16Arg8(0xD427, 0x26);
    sendCmd16Arg8(0xD428, 0xaa);
    sendCmd16Arg8(0xD429, 0x30);
    sendCmd16Arg8(0xD42A, 0x3c);
    sendCmd16Arg8(0xD42B, 0x4A);
    sendCmd16Arg8(0xD42C, 0x63);
    sendCmd16Arg8(0xD42D, 0xea);
    sendCmd16Arg8(0xD42E, 0x79);
    sendCmd16Arg8(0xD42F, 0xa6);
    sendCmd16Arg8(0xD430, 0xd0);
    sendCmd16Arg8(0xD431, 0x20);
    sendCmd16Arg8(0xD432, 0x0f);
    sendCmd16Arg8(0xD433, 0x8e);
    sendCmd16Arg8(0xD434, 0xff);

    //GAMMA SETING GREEN
    sendCmd16Arg8(0xD500, 0x00);
    sendCmd16Arg8(0xD501, 0x00);
    sendCmd16Arg8(0xD502, 0x1b);
    sendCmd16Arg8(0xD503, 0x44);
    sendCmd16Arg8(0xD504, 0x62);
    sendCmd16Arg8(0xD505, 0x00);
    sendCmd16Arg8(0xD506, 0x7b);
    sendCmd16Arg8(0xD507, 0xa1);
    sendCmd16Arg8(0xD508, 0xc0);
    sendCmd16Arg8(0xD509, 0xee);
    sendCmd16Arg8(0xD50A, 0x55);
    sendCmd16Arg8(0xD50B, 0x10);
    sendCmd16Arg8(0xD50C, 0x2c);
    sendCmd16Arg8(0xD50D, 0x43);
    sendCmd16Arg8(0xD50E, 0x57);
    sendCmd16Arg8(0xD50F, 0x55);
    sendCmd16Arg8(0xD510, 0x68);
    sendCmd16Arg8(0xD511, 0x78);
    sendCmd16Arg8(0xD512, 0x87);
    sendCmd16Arg8(0xD513, 0x94);
    sendCmd16Arg8(0xD514, 0x55);
    sendCmd16Arg8(0xD515, 0xa0);
    sendCmd16Arg8(0xD516, 0xac);
    sendCmd16Arg8(0xD517, 0xb6);
    sendCmd16Arg8(0xD518, 0xc1);
    sendCmd16Arg8(0xD519, 0x55);
    sendCmd16Arg8(0xD51A, 0xcb);
    sendCmd16Arg8(0xD51B, 0xcd);
    sendCmd16Arg8(0xD51C, 0xd6);
    sendCmd16Arg8(0xD51D, 0xdf);
    sendCmd16Arg8(0xD51E, 0x95);
    sendCmd16Arg8(0xD51F, 0xe8);
    sendCmd16Arg8(0xD520, 0xf1);
    sendCmd16Arg8(0xD521, 0xfa);
    sendCmd16Arg8(0xD522, 0x02);
    sendCmd16Arg8(0xD523, 0xaa);
    sendCmd16Arg8(0xD524, 0x0b);
    sendCmd16Arg8(0xD525, 0x13);
    sendCmd16Arg8(0xD526, 0x1d);
    sendCmd16Arg8(0xD527, 0x26);
    sendCmd16Arg8(0xD528, 0xaa);
    sendCmd16Arg8(0xD529, 0x30);
    sendCmd16Arg8(0xD52A, 0x3c);
    sendCmd16Arg8(0xD52B, 0x4a);
    sendCmd16Arg8(0xD52C, 0x63);
    sendCmd16Arg8(0xD52D, 0xea);
    sendCmd16Arg8(0xD52E, 0x79);
    sendCmd16Arg8(0xD52F, 0xa6);
    sendCmd16Arg8(0xD530, 0xd0);
    sendCmd16Arg8(0xD531, 0x20);
    sendCmd16Arg8(0xD532, 0x0f);
    sendCmd16Arg8(0xD533, 0x8e);
    sendCmd16Arg8(0xD534, 0xff);

    //GAMMA SETING BLUE
    sendCmd16Arg8(0xD600, 0x00);
    sendCmd16Arg8(0xD601, 0x00);
    sendCmd16Arg8(0xD602, 0x1b);
    sendCmd16Arg8(0xD603, 0x44);
    sendCmd16Arg8(0xD604, 0x62);
    sendCmd16Arg8(0xD605, 0x00);
    sendCmd16Arg8(0xD606, 0x7b);
    sendCmd16Arg8(0xD607, 0xa1);
    sendCmd16Arg8(0xD608, 0xc0);
    sendCmd16Arg8(0xD609, 0xee);
    sendCmd16Arg8(0xD60A, 0x55);
    sendCmd16Arg8(0xD60B, 0x10);
    sendCmd16Arg8(0xD60C, 0x2c);
    sendCmd16Arg8(0xD60D, 0x43);
    sendCmd16Arg8(0xD60E, 0x57);
    sendCmd16Arg8(0xD60F, 0x55);
    sendCmd16Arg8(0xD610, 0x68);
    sendCmd16Arg8(0xD611, 0x78);
    sendCmd16Arg8(0xD612, 0x87);
    sendCmd16Arg8(0xD613, 0x94);
    sendCmd16Arg8(0xD614, 0x55);
    sendCmd16Arg8(0xD615, 0xa0);
    sendCmd16Arg8(0xD616, 0xac);
    sendCmd16Arg8(0xD617, 0xb6);
    sendCmd16Arg8(0xD618, 0xc1);
    sendCmd16Arg8(0xD619, 0x55);
    sendCmd16Arg8(0xD61A, 0xcb);
    sendCmd16Arg8(0xD61B, 0xcd);
    sendCmd16Arg8(0xD61C, 0xd6);
    sendCmd16Arg8(0xD61D, 0xdf);
    sendCmd16Arg8(0xD61E, 0x95);
    sendCmd16Arg8(0xD61F, 0xe8);
    sendCmd16Arg8(0xD620, 0xf1);
    sendCmd16Arg8(0xD621, 0xfa);
    sendCmd16Arg8(0xD622, 0x02);
    sendCmd16Arg8(0xD623, 0xaa);
    sendCmd16Arg8(0xD624, 0x0b);
    sendCmd16Arg8(0xD625, 0x13);
    sendCmd16Arg8(0xD626, 0x1d);
    sendCmd16Arg8(0xD627, 0x26);
    sendCmd16Arg8(0xD628, 0xaa);
    sendCmd16Arg8(0xD629, 0x30);
    sendCmd16Arg8(0xD62A, 0x3c);
    sendCmd16Arg8(0xD62B, 0x4A);
    sendCmd16Arg8(0xD62C, 0x63);
    sendCmd16Arg8(0xD62D, 0xea);
    sendCmd16Arg8(0xD62E, 0x79);
    sendCmd16Arg8(0xD62F, 0xa6);
    sendCmd16Arg8(0xD630, 0xd0);
    sendCmd16Arg8(0xD631, 0x20);
    sendCmd16Arg8(0xD632, 0x0f);
    sendCmd16Arg8(0xD633, 0x8e);
    sendCmd16Arg8(0xD634, 0xff);

    sendCmd16Arg8(0xB000, 0x05);	//AVDD VOLTAGE SETTING
    sendCmd16Arg8(0xB001, 0x05);
    sendCmd16Arg8(0xB002, 0x05);
    
    sendCmd16Arg8(0xB100, 0x05);	//AVEE VOLTAGE SETTING
    sendCmd16Arg8(0xB101, 0x05);
    sendCmd16Arg8(0xB102, 0x05);

    sendCmd16Arg8(0xB600, 0x34);	//AVDD Boosting
    sendCmd16Arg8(0xB601, 0x34);
    sendCmd16Arg8(0xB603, 0x34);
    
    sendCmd16Arg8(0xB700, 0x24);	//AVEE Boosting
    sendCmd16Arg8(0xB701, 0x24);
    sendCmd16Arg8(0xB702, 0x24);
    
    sendCmd16Arg8(0xB800, 0x24);	//VCL Boosting
    sendCmd16Arg8(0xB801, 0x24);
    sendCmd16Arg8(0xB802, 0x24);
    
    sendCmd16Arg8(0xBA00, 0x14);	//VGLX VOLTAGE SETTING
    sendCmd16Arg8(0xBA01, 0x14);
    sendCmd16Arg8(0xBA02, 0x14);
    
    sendCmd16Arg8(0xB900, 0x24);	//VCL Boosting
    sendCmd16Arg8(0xB901, 0x24);
    sendCmd16Arg8(0xB902, 0x24);
   
    sendCmd16Arg8(0xBc00, 0x00);	//Gamma Voltage
    sendCmd16Arg8(0xBc01, 0xa0);	//vgmp=5.0
    sendCmd16Arg8(0xBc02, 0x00);
    sendCmd16Arg8(0xBd00, 0x00);
    sendCmd16Arg8(0xBd01, 0xa0);	//vgmn=5.0
    sendCmd16Arg8(0xBd02, 0x00);
    
    sendCmd16Arg8(0xBe01, 0x3d);	//VCOM Setting  3
    
    sendCmd16Arg8(0xF000, 0x55);	//ENABLE PAGE 0
    sendCmd16Arg8(0xF001, 0xAA);
    sendCmd16Arg8(0xF002, 0x52);
    sendCmd16Arg8(0xF003, 0x08);
    sendCmd16Arg8(0xF004, 0x00);
    
    sendCmd16Arg8(0xB400, 0x10);	//Vivid Color Function Control

    sendCmd16Arg8(0xBC00, 0x05);	//Z-INVERSION
    sendCmd16Arg8(0xBC01, 0x05);
    sendCmd16Arg8(0xBC02, 0x05);

    sendCmd16Arg8(0xB700, 0x22);	//GATE EQ CONTROL
    sendCmd16Arg8(0xB701, 0x22);	//GATE EQ CONTROL
    sendCmd16Arg8(0xC80B, 0x2A);	//DISPLAY TIMING CONTROL
    sendCmd16Arg8(0xC80C, 0x2A);	//DISPLAY TIMING CONTROL
    sendCmd16Arg8(0xC80F, 0x2A);	//DISPLAY TIMING CONTROL
    sendCmd16Arg8(0xC810, 0x2A);	//DISPLAY TIMING CONTROL
    
    
    sendCmd16Arg8(0xd000, 0x01);	//PWM_ENH_OE =1
    sendCmd16Arg8(0xb300, 0x10);	//DM_SEL =1
    sendCmd16Arg8(0xBd02, 0x07);	//VBPDA=07h
    sendCmd16Arg8(0xBe02, 0x07);	//VBPDb=07h
    sendCmd16Arg8(0xBf02, 0x07);	//VBPDc=07h
    
    sendCmd16Arg8(0xF000, 0x55);	//ENABLE PAGE 2
    sendCmd16Arg8(0xF001, 0xAA);
    sendCmd16Arg8(0xF002, 0x52);
    sendCmd16Arg8(0xF003, 0x08);
    sendCmd16Arg8(0xF004, 0x02);
    
    sendCmd16Arg8(0xc301, 0xa9);	//SDREG0 =0
    sendCmd16Arg8(0xfe01, 0x94);	//DS=14
    sendCmd16Arg8(0xf600, 0x60);	//OSC =60h
    //sendCmd16Arg8(0x3500, 0x00);	//TE ON

    sendCmd16Arg8(0xF000, 0x55);	//ENABLE PAGE 0
    sendCmd16Arg8(0xF001, 0xAA);
    sendCmd16Arg8(0xF002, 0x52);
    sendCmd16Arg8(0xF003, 0x08);
    sendCmd16Arg8(0xF004, 0x00);
    
    sendCmd16(0x1100);					// SLEEP OUT 
    delay(120);

    sendCmd16Arg8(0x3A00, 0x55);		// Bitdepth = RGB565
    delay(50);
    sendCmd16Arg8(0x3600, MADCTL[3]);	// Rotation
	delay(50);
    sendCmd16(0x2900);					// Display ON
    delay(50);
}

FLASHMEM void RM68120_t41_p::displayInit () 
{

	setBacklight(TFT_INTENSITY);
	
	if (_rst > -1){
		//Hardware reset
    	digitalWriteFast(_rst, HIGH);
    	delay(15); 
    	digitalWriteFast(_rst, LOW);
    	delay(15);
    	digitalWriteFast(_rst, HIGH);
    	delay(15);
	}else{
    	//Software reset
    	CSLow(); 
		gpioWrite();
    	sendCmd16(0x0100); // Software Reset
    	delay(120);
    	CSHigh(); 
	}


	CSLow(); 
	setRegisters();
	CSHigh();
}


FASTRUN void RM68120_t41_p::CSLow () 
{
	digitalWriteFast(_cs, LOW);       //Select TFT
}

FASTRUN void RM68120_t41_p::CSHigh () 
{
	digitalWriteFast(_cs, HIGH);       //Deselect TFT
}

FASTRUN void RM68120_t41_p::DCLow () 
{
	digitalWriteFast(_dc, LOW);       //Writing command to TFT
}

FASTRUN void RM68120_t41_p::DCHigh () 
{
	digitalWriteFast(_dc, HIGH);       //Writing data to TFT
}

FASTRUN inline void RM68120_t41_p::gpioWrite ()
{
	pFlex->setIOPinToFlexMode(36);
	pinMode(37, OUTPUT);
	digitalWriteFast(37, HIGH);
}

FASTRUN void RM68120_t41_p::FlexIO_Init ()
{
	/* Get a FlexIO channel */
    pFlex = FlexIOHandler::flexIOHandler_list[2]; // use FlexIO3
    /* Pointer to the port structure in the FlexIO channel */
    p = &pFlex->port();
    /* Pointer to the hardware structure in the FlexIO channel */
    hw = &pFlex->hardware();
    /* Basic pin setup */
    
    pinMode(19, OUTPUT); // FlexIO3:0 D0
    pinMode(18, OUTPUT); // FlexIO3:1 |
    pinMode(14, OUTPUT); // FlexIO3:2 |
    pinMode(15, OUTPUT); // FlexIO3:3 |
    pinMode(40, OUTPUT); // FlexIO3:4 |
    pinMode(41, OUTPUT); // FlexIO3:5 |
    pinMode(17, OUTPUT); // FlexIO3:6 |
    pinMode(16, OUTPUT); // FlexIO3:7 D7

#if (BUS_WIDTH == 16)
    pinMode(22, OUTPUT); // FlexIO3:8 D8
    pinMode(23, OUTPUT); // FlexIO3:9  |
    pinMode(20, OUTPUT); // FlexIO3:10 |
    pinMode(21, OUTPUT); // FlexIO3:11 |
    pinMode(38, OUTPUT); // FlexIO3:12 |
    pinMode(39, OUTPUT); // FlexIO3:13 |
    pinMode(26, OUTPUT); // FlexIO3:14 |
    pinMode(27, OUTPUT); // FlexIO3:15 D15
#endif
    
    pinMode(36, OUTPUT); // FlexIO3:18 WR
    pinMode(37, OUTPUT); // FlexIO3:19 RD

    digitalWrite(36,HIGH);
    digitalWrite(37,HIGH);

    /* High speed and drive strength configuration */
    *(portControlRegister(36)) = 0xFF;
    *(portControlRegister(37)) = 0xFF; 

    *(portControlRegister(19)) = 0xFF;
    *(portControlRegister(18)) = 0xFF;
    *(portControlRegister(14)) = 0xFF;
    *(portControlRegister(15)) = 0xFF;
    *(portControlRegister(40)) = 0xFF;
    *(portControlRegister(41)) = 0xFF;
    *(portControlRegister(17)) = 0xFF;
    *(portControlRegister(16)) = 0xFF;

#if (BUS_WIDTH == 16)
    *(portControlRegister(22)) = 0xFF;
    *(portControlRegister(23)) = 0xFF;
    *(portControlRegister(20)) = 0xFF;
    *(portControlRegister(21)) = 0xFF;
    *(portControlRegister(38)) = 0xFF;
    *(portControlRegister(39)) = 0xFF;
    *(portControlRegister(26)) = 0xFF;
    *(portControlRegister(27)) = 0xFF;
#endif

    /* Set clock */
    pFlex->setClockSettings(3, 1, 0); // (480 MHz source, 1+1, 1+0) >> 480/2/1 >> 240Mhz

    /* Set up pin mux */
    pFlex->setIOPinToFlexMode(36);
    pFlex->setIOPinToFlexMode(37);

    pFlex->setIOPinToFlexMode(19);
    pFlex->setIOPinToFlexMode(18);
    pFlex->setIOPinToFlexMode(14);
    pFlex->setIOPinToFlexMode(15);
    pFlex->setIOPinToFlexMode(40);
    pFlex->setIOPinToFlexMode(41);
    pFlex->setIOPinToFlexMode(17);
    pFlex->setIOPinToFlexMode(16);

#if (BUS_WIDTH == 16)
    pFlex->setIOPinToFlexMode(22);
    pFlex->setIOPinToFlexMode(23);
    pFlex->setIOPinToFlexMode(20);
    pFlex->setIOPinToFlexMode(21);
    pFlex->setIOPinToFlexMode(38);
    pFlex->setIOPinToFlexMode(39);
    pFlex->setIOPinToFlexMode(26);
    pFlex->setIOPinToFlexMode(27);
#endif
    /* Enable the clock */
    hw->clock_gate_register |= hw->clock_gate_mask;
    /* Enable the FlexIO with fast access */
    p->CTRL = FLEXIO_CTRL_FLEXEN;
    
}

FASTRUN void RM68120_t41_p::FlexIO_Config_SnglBeat ()
{
    gpioWrite();

    p->CTRL &= ~FLEXIO_CTRL_FLEXEN;
    p->CTRL |= FLEXIO_CTRL_SWRST;
    p->CTRL &= ~FLEXIO_CTRL_SWRST;

    

    /* Configure the shifters */
    p->SHIFTCFG[0] = 
       FLEXIO_SHIFTCFG_INSRC*(1)                                               /* Shifter input */
       |FLEXIO_SHIFTCFG_SSTOP(0)                                               /* Shifter stop bit disabled */
       | FLEXIO_SHIFTCFG_SSTART(0)                                             /* Shifter start bit disabled and loading data on enabled */
       | FLEXIO_SHIFTCFG_PWIDTH(BUS_WIDTH-1);                                            /* Bus width */
     
    p->SHIFTCTL[0] = 
        FLEXIO_SHIFTCTL_TIMSEL(0)                                              /* Shifter's assigned timer index */
      | FLEXIO_SHIFTCTL_TIMPOL*(0)                                             /* Shift on posedge of shift clock */
      | FLEXIO_SHIFTCTL_PINCFG(3)                                              /* Shifter's pin configured as output */
      | FLEXIO_SHIFTCTL_PINSEL(0)                                              /* Shifter's pin start index */
      | FLEXIO_SHIFTCTL_PINPOL*(0)                                             /* Shifter's pin active high */
      | FLEXIO_SHIFTCTL_SMOD(2);                                               /* Shifter mode as transmit */

    /* Configure the timer for shift clock */
    p->TIMCMP[0] = 
        (((1 * 2) - 1) << 8)                                                   /* TIMCMP[15:8] = number of beats x 2 – 1 */
      | ((_baud_div/2) - 1);                                                   /* TIMCMP[7:0] = baud rate divider / 2 – 1 */
    
    p->TIMCFG[0] = 
        FLEXIO_TIMCFG_TIMOUT(0)                                                /* Timer output logic one when enabled and not affected by reset */
      | FLEXIO_TIMCFG_TIMDEC(0)                                                /* Timer decrement on FlexIO clock, shift clock equals timer output */
      | FLEXIO_TIMCFG_TIMRST(0)                                                /* Timer never reset */
      | FLEXIO_TIMCFG_TIMDIS(2)                                                /* Timer disabled on timer compare */
      | FLEXIO_TIMCFG_TIMENA(2)                                                /* Timer enabled on trigger high */
      | FLEXIO_TIMCFG_TSTOP(0)                                                 /* Timer stop bit disabled */
      | FLEXIO_TIMCFG_TSTART*(0);                                              /* Timer start bit disabled */
    
    p->TIMCTL[0] = 
        FLEXIO_TIMCTL_TRGSEL((((0) << 2) | 1))                                 /* Timer trigger selected as shifter's status flag */
      | FLEXIO_TIMCTL_TRGPOL*(1)                                               /* Timer trigger polarity as active low */
      | FLEXIO_TIMCTL_TRGSRC*(1)                                               /* Timer trigger source as internal */
      | FLEXIO_TIMCTL_PINCFG(3)                                                /* Timer' pin configured as output */
      | FLEXIO_TIMCTL_PINSEL(18)                                               /* Timer' pin index: WR pin */
      | FLEXIO_TIMCTL_PINPOL*(1)                                               /* Timer' pin active low */
      | FLEXIO_TIMCTL_TIMOD(1);                                                /* Timer mode as dual 8-bit counters baud/bit */
    
    /* Enable FlexIO */
    p->CTRL |= FLEXIO_CTRL_FLEXEN;      

}

#include <USBHost_t36.h>
FASTRUN void RM68120_t41_p::sglBeatWR_nPrm_16 (uint32_t const cmd, uint16_t *value, const uint32_t length)
{
    NVIC_DISABLE_IRQ(IRQ_USBHS);
    NVIC_DISABLE_IRQ(IRQ_USBPHY0);
	__disable_irq();
	
	//delayNanoseconds(20);
    CSLow();
	//delayNanoseconds(20);
    DCLow();
	delayNanoseconds(40);
	

    p->SHIFTBUF[0] = cmd;
	while (0 == (p->SHIFTSTAT & (1 << 0))){
		
	}
	
	delayNanoseconds(20);
    DCHigh();
	delayNanoseconds(20);
    
    if (length){
    	delayNanoseconds(20);

		for (uint32_t i = 0; i < length; i++){
			p->SHIFTBUF[0] = *value++;

      		while (0 == (p->SHIFTSTAT & (3 << 0))){
      			
    		}
		}

	}

	//delayNanoseconds(20);
    CSHigh();
	//delayNanoseconds(20);

	__enable_irq();
	NVIC_ENABLE_IRQ(IRQ_USBPHY0);
	NVIC_ENABLE_IRQ(IRQ_USBHS);
}


#endif