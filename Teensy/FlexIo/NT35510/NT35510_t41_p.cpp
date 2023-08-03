
/*
	Tab @ 4 spaces
*/


#if USE_FLEXTFT_NT35510

#include "NT35510_t41_p.h"

static NT35510_t41_p STORAGETYPE lcd = NT35510_t41_p(TFT_RS, TFT_CS, TFT_RST, TFT_BL);


FASTRUN void NT35510_t41_p::sendCmd16Arg8 (const uint16_t cmd, const uint8_t arg)
{
	uint16_t arg16[2] = {arg, 0};

   	SglBeatWR_nPrm_16(cmd, arg16, 1);
}

FASTRUN void NT35510_t41_p::sendCmd16 (const uint16_t cmd)
{
   	SglBeatWR_nPrm_16(cmd, 0, 0);
}

FLASHMEM NT35510_t41_p::NT35510_t41_p (int8_t dc, int8_t cs, int8_t rst, int8_t bl) 
{  
  _dc = dc;
  _cs = cs;
  _rst = rst;
  _bl = bl;
}

FLASHMEM void NT35510_t41_p::setBacklight (const uint8_t value)
{
    analogWrite(_bl, value);
}

FLASHMEM void NT35510_t41_p::begin (uint8_t baud_div) 
{
	switch (baud_div){					// with a base freq of 240mhz
    case 2:  _baud_div = 120; break;
    case 4:  _baud_div = 60; break;
    case 8:  _baud_div = 30; break;
    case 12: _baud_div = 20; break;
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
  
  pinMode(_cs, OUTPUT); // CS
  pinMode(_dc, OUTPUT); // DC
  pinMode(_rst, OUTPUT); // RST
  pinMode(_bl, OUTPUT);
  
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
  displayInit();
  
  //setBitDepth(_bitDepth);
  setRotation(3);
  
  /*
  setBitDepth(_bitDepth);

  setTearingEffect(_bTearingOn);
  if (_bTearingOn == true) {
    setTearingScanLine(_tearingScanLine);
  }  
  setFrameRate(_frameRate);
  */
 
  _width  = NT35510_TFTWIDTH;
  _height = NT35510_TFTHEIGHT;

  
}


#define NT35510_RDID1 0x0400
#define NT35510_RDID2 0x0401
#define NT35510_RDID3 0x0402

#if 0
FASTRUN void NT35510_t41_p::displayInfo()
{
  CSLow();

  sendCmd16(0x2300);  
  sendCmd16(NT35510_RDID1);

  
  Serial.printf("Manufacturer ID: 0x%02X\n",    readCommand(NT35510_RDID1)); 
  Serial.printf("Module Version ID: 0x%02X\n",  readCommand(NT35510_RDID2)); 
  Serial.printf("Module ID: 0x%02X\n",          readCommand(NT35510_RDID3)); 
/*	Serial.printf("Display Power Mode: 0x%02X\n", readCommand(NT35510_RDMODE));
	Serial.printf("MADCTL Mode: 0x%02X\n",        readCommand(NT35510_RDMADCTL));
	Serial.printf("Pixel Format: 0x%02X\n",       readCommand(NT35510_RDCOLMOD));
	Serial.printf("Image Format: 0x%02X\n",       readCommand(NT35510_RDIMGFMT)); 
  Serial.printf("Signal Mode: 0x%02X\n",        readCommand(NT35510_RDDSM)); 
  uint8_t sdRes = readCommand(NT35510_RDSELFDIAG);
  Serial.printf("Self Diagnostic: %s (0x%02X)\n", sdRes == 0xc0 ? "OK" : "Failed", sdRes);*/
  CSHigh();
}
#endif

FLASHMEM uint8_t NT35510_t41_p::setBitDepth (uint8_t bitDepth)  
{
  uint8_t bd = 0x55;

  switch (bitDepth) {
    case 16:  _bitDepth = 16;
              bd = 0x55;
              break;
    case 18:  _bitDepth = 18;
              bd = 0x66;
              break;
    case 24:  //Unsupported
              return _bitDepth;
              break;
    default:  //Unsupported
              return _bitDepth;
              break;
  }
 
  sendCmd16Arg8(NT35510_COLMOD, bd);

  //Insert small delay here as rapid calls appear to fail
  delay(10);

  return _bitDepth;
}

FLASHMEM uint8_t NT35510_t41_p::getBitDepth()
{
  return _bitDepth;
}

FLASHMEM void NT35510_t41_p::setFrameRate(uint8_t frRate) 
{
 _frameRate = frRate;

  uint8_t fr28Hz[2] = {0x00, 0x11}; // 28.78fps, 17 clocks
  uint8_t fr30Hz[2] = {0x10, 0x11}; // 30.38fps, 17 clocks
  uint8_t fr39Hz[2] = {0x50, 0x11}; // 39.06fps, 17 clocks
  uint8_t fr45Hz[2] = {0x70, 0x11}; // 45.57fps, 17 clocks
  uint8_t fr54Hz[2] = {0x90, 0x11}; // 54.69ps, 17 clocks
  uint8_t fr60Hz[2] = {0xA0, 0x11}; // 60.76fps, 17 clocks
  uint8_t fr68Hz[2] = {0xB0, 0x11}; // 68.36fps, 17 clocks (NT35510 default)
  uint8_t fr78Hz[2] = {0xC0, 0x11}; // 78.13fps, 17 clocks
  uint8_t fr91Hz[2] = {0xD0, 0x11}; // 91.15fps, 17 clocks

  uint8_t frData[2];
  //Select parameters for frame rate
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

//  SglBeatWR_nPrm_8(NT35510_FRMCTR1, frData, 2);
  
}

FLASHMEM uint8_t NT35510_t41_p::getFrameRate()
{
  return _frameRate;
}

FLASHMEM void NT35510_t41_p::setTearingEffect(bool tearingOn)
{

/*  _bTearingOn = tearingOn;
  uint8_t mode = 0x00;
  
  CSLow();
  if (_bTearingOn == true) {
    SglBeatWR_nPrm_8(NT35510_TEON, &mode, 1);        //Tearing effect line on, mode 0 (V-Blanking)
  } else {
    SglBeatWR_nPrm_8(NT35510_TEOFF,0,0);
  }
  CSHigh();*/

}

FLASHMEM bool NT35510_t41_p::getTearingEffect()
{
  return _bTearingOn;
}

FLASHMEM void NT35510_t41_p::setTearingScanLine(uint16_t scanLine)
{
 /* _tearingScanLine = scanLine;
  
  uint8_t params[2] = {(uint8_t)(_tearingScanLine << 8), (uint8_t)(_tearingScanLine & 0xFF)};
  SglBeatWR_nPrm_8(NT35510_TESLWR, params, 2);      //Tearing effect write scan line : 0x00 0x00 = line 0 (default), 0x 0xA0 = line 160, 0x 0xF0 = line 240
*/
}

FLASHMEM uint16_t NT35510_t41_p::getTearingScanLine()
{
  return _tearingScanLine;
}

FLASHMEM void NT35510_t41_p::setRotation (uint8_t r) 
{ 
  _rotation = r & 3;

  switch (_rotation) {
    case 0:	
    case 2: _width  = NT35510_TFTWIDTH;
            _height = NT35510_TFTHEIGHT;
            break;
    case 1: 
    case 3: _width  = NT35510_TFTHEIGHT;
            _height = NT35510_TFTWIDTH;
            break;
	}

  sendCmd16Arg8(NT35510_MADCTL, MADCTL[_rotation]);
}

FLASHMEM void NT35510_t41_p::invertDisplay(bool invert) 
{
  //sendCmd16(invert ? NT35510_INVON : NT35510_INVOFF);
}

FLASHMEM void NT35510_t41_p::onCompleteCB(CBF callback)
{
  _callback = callback;
  isCB = true;
}

FASTRUN void NT35510_t41_p::setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
/*  uint8_t Command;
  uint8_t CommandValue[4];

  Command = 0x2A00;
  CommandValue[0U] = x1 >> 8U;
  CommandValue[1U] = x1 & 0xFF;
  CommandValue[2U] = x2 >> 8U;
  CommandValue[3U] = x2 & 0xFF;
  SglBeatWR_nPrm_16(Command, CommandValue, 4U);

  Command = 0x2B00;
  CommandValue[0U] = y1 >> 8U;
  CommandValue[1U] = y1 & 0xFF;
  CommandValue[2U] = y2 >> 8U;
  CommandValue[3U] = y2 & 0xFF;
  SglBeatWR_nPrm_16(Command, CommandValue, 4U);
*/
  //SglBeatWR_nPrm_8(NT35510_RAMWR); //Set NT35510 to expect RAM data for pixels, resets column and page regs


	sendCmd16Arg8(NT35510_CASET + 0, x1 >> 8);
	sendCmd16Arg8(NT35510_CASET + 1, x1 & 0xFF);
	sendCmd16Arg8(NT35510_CASET + 2, x2 >> 8);
	sendCmd16Arg8(NT35510_CASET + 3, x2 & 0xFF);

	sendCmd16Arg8(NT35510_RASET + 0, y1 >> 8);
	sendCmd16Arg8(NT35510_RASET + 1, y1 & 0xFF);
	sendCmd16Arg8(NT35510_RASET + 2, y2 >> 8);
	sendCmd16Arg8(NT35510_RASET + 3, y2 & 0xFF);

	sendCmd16(NT35510_RAMWR);


  CSHigh();
}

FASTRUN void NT35510_t41_p::pushPixels16bit (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
/*  while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
  }
  uint32_t area = (x2-x1+1)*(y2-y1+1);
  if (!((_lastx1 == x1) && (_lastx2 == x2) && (_lasty1 == y1) && (_lasty2 == y2))) {
  setAddrWindow( x1, y1, x2, y2);
     _lastx1 = x1;  _lastx2 = x2;  _lasty1 = y1;  _lasty2 = y2;
  }
  SglBeatWR_nPrm_16(NT35510_RAMWR, pcolors, area);
  */
   setAddrWindow( x1, y1, x2, y2);
  	volatile uint8_t row[NT35510_TFTWIDTH*3];
	memset((void*)row, 0, sizeof(row));

	int i16 = 0;
	
	for (int volatile i = 0; i < NT35510_TFTWIDTH*3; i+=3){
		row[i]   = ((pcolors[i16]>>11) & 0x1F) << 3;
		row[i+1] = ((pcolors[i16]>>5)  & 0x3F) << 2;
		row[i+2] = (pcolors[i16]       & 0x1F) << 3;
		//row[i+1] = 1 << 6;
		
		i16++;
	}

	int len = (NT35510_TFTWIDTH * 3) / 2;
	SglBeatWR_nPrm_16(NT35510_RAMWR, (uint16_t*)row, len);
  
  
}

FASTRUN void NT35510_t41_p::pushPixels16bitAsync (const uint16_t * pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
  }
  uint32_t area = (x2-x1+1)*(y2-y1+1);
  if (!((_lastx1 == x1) && (_lastx2 == x2) && (_lasty1 == y1) && (_lasty2 == y2))) {
  setAddrWindow( x1, y1, x2, y2);
  _lastx1 = x1;  _lastx2 = x2; _lasty1 = y1;  _lasty2 = y2;
  }
  MulBeatWR_nPrm_IRQ(NT35510_RAMWR, pcolors, area);
}


///////////////////
//Private functions
///////////////////

FLASHMEM void NT35510_t41_p::displayInit () 
{
	
	setBacklight(TFT_INTENSITY);
	
  if (_rst > -1) {
    //Hardware reset
    digitalWriteFast(_rst, HIGH);
    delay(15); 
    digitalWriteFast(_rst, LOW);
    delay(15);
    digitalWriteFast(_rst, HIGH);
    delay(15);
  } else {
    //Software reset
    CSLow(); 
    sendCmd16(0x0100); // Software Reset
    delay(120); 
    CSHigh(); 
  }

  CSLow(); 
/*  SglBeatWR_nPrm_8(NT35510_SLPOUT, 0 , 0);
  delay(15);
  SglBeatWR_nPrm_8(NT35510_DISPOFF, 0 , 0);
  delay(15); 
  */
  
/*
  uint8_t * commands = (uint8_t*)_init_commands;
  uint8_t cmd, x, numArgs;
  while((cmd = *commands++) > 0) {
    x = *commands++;
    numArgs = x & 0x7F;
    SglBeatWR_nPrm_8(cmd, commands , numArgs);
    commands += numArgs;
  }

  SglBeatWR_nPrm_8(NT35510_DISPON, 0 , 0);
  delay(15); 
*/

    //sendCmd16(0x0100); // Software Reset
	//delay(100);
	
	sendCmd16(0x0100); // Software Reset
	delay(130);
	
//	sendCmd16(0x2900);
//	delay(100);


	sendCmd16Arg8(0xf000, 0x55);	
	sendCmd16Arg8(0xf001, 0xaa);	
	sendCmd16Arg8(0xf002, 0x52);	
	sendCmd16Arg8(0xf003, 0x08);	
	sendCmd16Arg8(0xf004, 0x01);	
	                                           
	sendCmd16Arg8(0xbc01, 0x86);	
	sendCmd16Arg8(0xbc02, 0x6a);	
	sendCmd16Arg8(0xbd01, 0x86);	
	sendCmd16Arg8(0xbd02, 0x6a);	
	sendCmd16Arg8(0xbe01, 0x67);	
	                                           
	sendCmd16Arg8(0xd100, 0x00);	
	sendCmd16Arg8(0xd101, 0x5d);	
	sendCmd16Arg8(0xd102, 0x00);	
	sendCmd16Arg8(0xd103, 0x6b);	
	sendCmd16Arg8(0xd104, 0x00);	
	sendCmd16Arg8(0xd105, 0x84);	
	sendCmd16Arg8(0xd106, 0x00);	
	sendCmd16Arg8(0xd107, 0x9c);	
	sendCmd16Arg8(0xd108, 0x00);	
	sendCmd16Arg8(0xd109, 0xb1);	
	sendCmd16Arg8(0xd10a, 0x00);	
	sendCmd16Arg8(0xd10b, 0xd9);	
	sendCmd16Arg8(0xd10c, 0x00);	
	sendCmd16Arg8(0xd10d, 0xfd);	
	sendCmd16Arg8(0xd10e, 0x01);	
	sendCmd16Arg8(0xd10f, 0x38);	
	sendCmd16Arg8(0xd110, 0x01);	
	sendCmd16Arg8(0xd111, 0x68);	
	sendCmd16Arg8(0xd112, 0x01);	
	sendCmd16Arg8(0xd113, 0xb9);	
	sendCmd16Arg8(0xd114, 0x01);	
	sendCmd16Arg8(0xd115, 0xfb);	
	sendCmd16Arg8(0xd116, 0x02);	
	sendCmd16Arg8(0xd117, 0x63);	
	sendCmd16Arg8(0xd118, 0x02);	
	sendCmd16Arg8(0xd119, 0xb9);	
	sendCmd16Arg8(0xd11a, 0x02);	
	sendCmd16Arg8(0xd11b, 0xbb);	
	sendCmd16Arg8(0xd11c, 0x03);	
	sendCmd16Arg8(0xd11d, 0x03);	
	sendCmd16Arg8(0xd11e, 0x03);	
	sendCmd16Arg8(0xd11f, 0x46);	
	sendCmd16Arg8(0xd120, 0x03);	
	sendCmd16Arg8(0xd121, 0x69);	
	sendCmd16Arg8(0xd122, 0x03);	
	sendCmd16Arg8(0xd123, 0x8f);	
	sendCmd16Arg8(0xd124, 0x03);	
	sendCmd16Arg8(0xd125, 0xa4);	
	sendCmd16Arg8(0xd126, 0x03);	
	sendCmd16Arg8(0xd127, 0xb9);	
	sendCmd16Arg8(0xd128, 0x03);	
	sendCmd16Arg8(0xd129, 0xc7);	
	sendCmd16Arg8(0xd12a, 0x03);	
	sendCmd16Arg8(0xd12b, 0xc9);	
	sendCmd16Arg8(0xd12c, 0x03);	
	sendCmd16Arg8(0xd12d, 0xcb);	
	sendCmd16Arg8(0xd12e, 0x03);	
	sendCmd16Arg8(0xd12f, 0xcb);	
	sendCmd16Arg8(0xd130, 0x03);	
	sendCmd16Arg8(0xd131, 0xcb);	
	sendCmd16Arg8(0xd132, 0x03);	
	sendCmd16Arg8(0xd133, 0xcc);	
	                                           
	sendCmd16Arg8(0xd200, 0x00);	
	sendCmd16Arg8(0xd201, 0x5d);	
	sendCmd16Arg8(0xd202, 0x00);	
	sendCmd16Arg8(0xd203, 0x6b);	
	sendCmd16Arg8(0xd204, 0x00);	
	sendCmd16Arg8(0xd205, 0x84);	
	sendCmd16Arg8(0xd206, 0x00);	
	sendCmd16Arg8(0xd207, 0x9c);	
	sendCmd16Arg8(0xd208, 0x00);	
	sendCmd16Arg8(0xd209, 0xb1);	
	sendCmd16Arg8(0xd20a, 0x00);	
	sendCmd16Arg8(0xd20b, 0xd9);	
	sendCmd16Arg8(0xd20c, 0x00);	
	sendCmd16Arg8(0xd20d, 0xfd);	
	sendCmd16Arg8(0xd20e, 0x01);	
	sendCmd16Arg8(0xd20f, 0x38);	
	sendCmd16Arg8(0xd210, 0x01);	
	sendCmd16Arg8(0xd211, 0x68);	
	sendCmd16Arg8(0xd212, 0x01);	
	sendCmd16Arg8(0xd213, 0xb9);	
	sendCmd16Arg8(0xd214, 0x01);	
	sendCmd16Arg8(0xd215, 0xfb);	
	sendCmd16Arg8(0xd216, 0x02);	
	sendCmd16Arg8(0xd217, 0x63);	
	sendCmd16Arg8(0xd218, 0x02);	
	sendCmd16Arg8(0xd219, 0xb9);	
	sendCmd16Arg8(0xd21a, 0x02);	
	sendCmd16Arg8(0xd21b, 0xbb);	
	sendCmd16Arg8(0xd21c, 0x03);	
	sendCmd16Arg8(0xd21d, 0x03);	
	sendCmd16Arg8(0xd21e, 0x03);	
	sendCmd16Arg8(0xd21f, 0x46);	
	sendCmd16Arg8(0xd220, 0x03);	
	sendCmd16Arg8(0xd221, 0x69);	
	sendCmd16Arg8(0xd222, 0x03);	
	sendCmd16Arg8(0xd223, 0x8f);	
	sendCmd16Arg8(0xd224, 0x03);	
	sendCmd16Arg8(0xd225, 0xa4);	
	sendCmd16Arg8(0xd226, 0x03);	
	sendCmd16Arg8(0xd227, 0xb9);	
	sendCmd16Arg8(0xd228, 0x03);	
	sendCmd16Arg8(0xd229, 0xc7);	
	sendCmd16Arg8(0xd22a, 0x03);	
	sendCmd16Arg8(0xd22b, 0xc9);	
	sendCmd16Arg8(0xd22c, 0x03);	
	sendCmd16Arg8(0xd22d, 0xcb);	
	sendCmd16Arg8(0xd22e, 0x03);	
	sendCmd16Arg8(0xd22f, 0xcb);	
	sendCmd16Arg8(0xd230, 0x03);	
	sendCmd16Arg8(0xd231, 0xcb);	
	sendCmd16Arg8(0xd232, 0x03);	
	sendCmd16Arg8(0xd233, 0xcc);	
	                                           
	                                           
	sendCmd16Arg8(0xd300, 0x00);	
	sendCmd16Arg8(0xd301, 0x5d);	
	sendCmd16Arg8(0xd302, 0x00);	
	sendCmd16Arg8(0xd303, 0x6b);	
	sendCmd16Arg8(0xd304, 0x00);	
	sendCmd16Arg8(0xd305, 0x84);	
	sendCmd16Arg8(0xd306, 0x00);	
	sendCmd16Arg8(0xd307, 0x9c);	
	sendCmd16Arg8(0xd308, 0x00);	
	sendCmd16Arg8(0xd309, 0xb1);	
	sendCmd16Arg8(0xd30a, 0x00);	
	sendCmd16Arg8(0xd30b, 0xd9);	
	sendCmd16Arg8(0xd30c, 0x00);	
	sendCmd16Arg8(0xd30d, 0xfd);	
	sendCmd16Arg8(0xd30e, 0x01);	
	sendCmd16Arg8(0xd30f, 0x38);	
	sendCmd16Arg8(0xd310, 0x01);	
	sendCmd16Arg8(0xd311, 0x68);	
	sendCmd16Arg8(0xd312, 0x01);	
	sendCmd16Arg8(0xd313, 0xb9);	
	sendCmd16Arg8(0xd314, 0x01);	
	sendCmd16Arg8(0xd315, 0xfb);	
	sendCmd16Arg8(0xd316, 0x02);	
	sendCmd16Arg8(0xd317, 0x63);	
	sendCmd16Arg8(0xd318, 0x02);	
	sendCmd16Arg8(0xd319, 0xb9);	
	sendCmd16Arg8(0xd31a, 0x02);	
	sendCmd16Arg8(0xd31b, 0xbb);	
	sendCmd16Arg8(0xd31c, 0x03);	
	sendCmd16Arg8(0xd31d, 0x03);	
	sendCmd16Arg8(0xd31e, 0x03);	
	sendCmd16Arg8(0xd31f, 0x46);	
	sendCmd16Arg8(0xd320, 0x03);	
	sendCmd16Arg8(0xd321, 0x69);	
	sendCmd16Arg8(0xd322, 0x03);	
	sendCmd16Arg8(0xd323, 0x8f);	
	sendCmd16Arg8(0xd324, 0x03);	
	sendCmd16Arg8(0xd325, 0xa4);	
	sendCmd16Arg8(0xd326, 0x03);	
	sendCmd16Arg8(0xd327, 0xb9);	
	sendCmd16Arg8(0xd328, 0x03);	
	sendCmd16Arg8(0xd329, 0xc7);	
	sendCmd16Arg8(0xd32a, 0x03);	
	sendCmd16Arg8(0xd32b, 0xc9);	
	sendCmd16Arg8(0xd32c, 0x03);	
	sendCmd16Arg8(0xd32d, 0xcb);	
	sendCmd16Arg8(0xd32e, 0x03);	
	sendCmd16Arg8(0xd32f, 0xcb);	
	sendCmd16Arg8(0xd330, 0x03);	
	sendCmd16Arg8(0xd331, 0xcb);	
	sendCmd16Arg8(0xd332, 0x03);	
	sendCmd16Arg8(0xd333, 0xcc);	
	                                           
	sendCmd16Arg8(0xd400, 0x00);	
	sendCmd16Arg8(0xd401, 0x5d);	
	sendCmd16Arg8(0xd402, 0x00);	
	sendCmd16Arg8(0xd403, 0x6b);	
	sendCmd16Arg8(0xd404, 0x00);	
	sendCmd16Arg8(0xd405, 0x84);	
	sendCmd16Arg8(0xd406, 0x00);	
	sendCmd16Arg8(0xd407, 0x9c);	
	sendCmd16Arg8(0xd408, 0x00);	
	sendCmd16Arg8(0xd409, 0xb1);	
	sendCmd16Arg8(0xd40a, 0x00);	
	sendCmd16Arg8(0xd40b, 0xd9);	
	sendCmd16Arg8(0xd40c, 0x00);	
	sendCmd16Arg8(0xd40d, 0xfd);	
	sendCmd16Arg8(0xd40e, 0x01);	
	sendCmd16Arg8(0xd40f, 0x38);	
	sendCmd16Arg8(0xd410, 0x01);	
	sendCmd16Arg8(0xd411, 0x68);	
	sendCmd16Arg8(0xd412, 0x01);	
	sendCmd16Arg8(0xd413, 0xb9);	
	sendCmd16Arg8(0xd414, 0x01);	
	sendCmd16Arg8(0xd415, 0xfb);	
	sendCmd16Arg8(0xd416, 0x02);	
	sendCmd16Arg8(0xd417, 0x63);	
	sendCmd16Arg8(0xd418, 0x02);	
	sendCmd16Arg8(0xd419, 0xb9);	
	sendCmd16Arg8(0xd41a, 0x02);	
	sendCmd16Arg8(0xd41b, 0xbb);	
	sendCmd16Arg8(0xd41c, 0x03);	
	sendCmd16Arg8(0xd41d, 0x03);	
	sendCmd16Arg8(0xd41e, 0x03);	
	sendCmd16Arg8(0xd41f, 0x46);	
	sendCmd16Arg8(0xd420, 0x03);	
	sendCmd16Arg8(0xd421, 0x69);	
	sendCmd16Arg8(0xd422, 0x03);	
	sendCmd16Arg8(0xd423, 0x8f);	
	sendCmd16Arg8(0xd424, 0x03);	
	sendCmd16Arg8(0xd425, 0xa4);	
	sendCmd16Arg8(0xd426, 0x03);	
	sendCmd16Arg8(0xd427, 0xb9);	
	sendCmd16Arg8(0xd428, 0x03);	
	sendCmd16Arg8(0xd429, 0xc7);	
	sendCmd16Arg8(0xd42a, 0x03);	
	sendCmd16Arg8(0xd42b, 0xc9);	
	sendCmd16Arg8(0xd42c, 0x03);	
	sendCmd16Arg8(0xd42d, 0xcb);	
	sendCmd16Arg8(0xd42e, 0x03);	
	sendCmd16Arg8(0xd42f, 0xcb);	
	sendCmd16Arg8(0xd430, 0x03);	
	sendCmd16Arg8(0xd431, 0xcb);	
	sendCmd16Arg8(0xd432, 0x03);	
	sendCmd16Arg8(0xd433, 0xcc);	
	                                           
	                                           
	sendCmd16Arg8(0xd500, 0x00);	
	sendCmd16Arg8(0xd501, 0x5d);	
	sendCmd16Arg8(0xd502, 0x00);	
	sendCmd16Arg8(0xd503, 0x6b);	
	sendCmd16Arg8(0xd504, 0x00);	
	sendCmd16Arg8(0xd505, 0x84);	
	sendCmd16Arg8(0xd506, 0x00);	
	sendCmd16Arg8(0xd507, 0x9c);	
	sendCmd16Arg8(0xd508, 0x00);	
	sendCmd16Arg8(0xd509, 0xb1);	
	sendCmd16Arg8(0xd50a, 0x00);	
	sendCmd16Arg8(0xd50b, 0xD9);	
	sendCmd16Arg8(0xd50c, 0x00);	
	sendCmd16Arg8(0xd50d, 0xfd);	
	sendCmd16Arg8(0xd50e, 0x01);	
	sendCmd16Arg8(0xd50f, 0x38);	
	sendCmd16Arg8(0xd510, 0x01);	
	sendCmd16Arg8(0xd511, 0x68);	
	sendCmd16Arg8(0xd512, 0x01);	
	sendCmd16Arg8(0xd513, 0xb9);	
	sendCmd16Arg8(0xd514, 0x01);	
	sendCmd16Arg8(0xd515, 0xfb);	
	sendCmd16Arg8(0xd516, 0x02);	
	sendCmd16Arg8(0xd517, 0x63);	
	sendCmd16Arg8(0xd518, 0x02);	
	sendCmd16Arg8(0xd519, 0xb9);	
	sendCmd16Arg8(0xd51a, 0x02);	
	sendCmd16Arg8(0xd51b, 0xbb);	
	sendCmd16Arg8(0xd51c, 0x03);	
	sendCmd16Arg8(0xd51d, 0x03);	
	sendCmd16Arg8(0xd51e, 0x03);	
	sendCmd16Arg8(0xd51f, 0x46);	
	sendCmd16Arg8(0xd520, 0x03);	
	sendCmd16Arg8(0xd521, 0x69);	
	sendCmd16Arg8(0xd522, 0x03);	
	sendCmd16Arg8(0xd523, 0x8f);	
	sendCmd16Arg8(0xd524, 0x03);	
	sendCmd16Arg8(0xd525, 0xa4);	
	sendCmd16Arg8(0xd526, 0x03);	
	sendCmd16Arg8(0xd527, 0xb9);	
	sendCmd16Arg8(0xd528, 0x03);	
	sendCmd16Arg8(0xd529, 0xc7);	
	sendCmd16Arg8(0xd52a, 0x03);	
	sendCmd16Arg8(0xd52b, 0xc9);	
	sendCmd16Arg8(0xd52c, 0x03);	
	sendCmd16Arg8(0xd52d, 0xcb);	
	sendCmd16Arg8(0xd52e, 0x03);	
	sendCmd16Arg8(0xd52f, 0xcb);	
	sendCmd16Arg8(0xd530, 0x03);	
	sendCmd16Arg8(0xd531, 0xcb);	
	sendCmd16Arg8(0xd532, 0x03);	
	sendCmd16Arg8(0xd533, 0xcc);	
	                                           
	sendCmd16Arg8(0xd600, 0x00);	
	sendCmd16Arg8(0xd601, 0x5d);	
	sendCmd16Arg8(0xd602, 0x00);	
	sendCmd16Arg8(0xd603, 0x6b);	
	sendCmd16Arg8(0xd604, 0x00);	
	sendCmd16Arg8(0xd605, 0x84);	
	sendCmd16Arg8(0xd606, 0x00);	
	sendCmd16Arg8(0xd607, 0x9c);	
	sendCmd16Arg8(0xd608, 0x00);	
	sendCmd16Arg8(0xd609, 0xb1);	
	sendCmd16Arg8(0xd60a, 0x00);	
	sendCmd16Arg8(0xd60b, 0xd9);	
	sendCmd16Arg8(0xd60c, 0x00);	
	sendCmd16Arg8(0xd60d, 0xfd);	
	sendCmd16Arg8(0xd60e, 0x01);	
	sendCmd16Arg8(0xd60f, 0x38);	
	sendCmd16Arg8(0xd610, 0x01);	
	sendCmd16Arg8(0xd611, 0x68);	
	sendCmd16Arg8(0xd612, 0x01);	
	sendCmd16Arg8(0xd613, 0xb9);	
	sendCmd16Arg8(0xd614, 0x01);	
	sendCmd16Arg8(0xd615, 0xfb);	
	sendCmd16Arg8(0xd616, 0x02);	
	sendCmd16Arg8(0xd617, 0x63);	
	sendCmd16Arg8(0xd618, 0x02);	
	sendCmd16Arg8(0xd619, 0xb9);	
	sendCmd16Arg8(0xd61a, 0x02);	
	sendCmd16Arg8(0xd61b, 0xbb);	
	sendCmd16Arg8(0xd61c, 0x03);	
	sendCmd16Arg8(0xd61d, 0x03);	
	sendCmd16Arg8(0xd61e, 0x03);	
	sendCmd16Arg8(0xd61f, 0x46);	
	sendCmd16Arg8(0xd620, 0x03);	
	sendCmd16Arg8(0xd621, 0x69);	
	sendCmd16Arg8(0xd622, 0x03);	
	sendCmd16Arg8(0xd623, 0x8f);	
	sendCmd16Arg8(0xd624, 0x03);	
	sendCmd16Arg8(0xd625, 0xa4);	
	sendCmd16Arg8(0xd626, 0x03);	
	sendCmd16Arg8(0xd627, 0xb9);	
	sendCmd16Arg8(0xd628, 0x03);	
	sendCmd16Arg8(0xd629, 0xc7);	
	sendCmd16Arg8(0xd62a, 0x03);	
	sendCmd16Arg8(0xd62b, 0xc9);	
	sendCmd16Arg8(0xd62c, 0x03);	
	sendCmd16Arg8(0xd62d, 0xcb);	
	sendCmd16Arg8(0xd62e, 0x03);	
	sendCmd16Arg8(0xd62f, 0xcb);	
	sendCmd16Arg8(0xd630, 0x03);	
	sendCmd16Arg8(0xd631, 0xcb);	
	sendCmd16Arg8(0xd632, 0x03);	
	sendCmd16Arg8(0xd633, 0xcc);	
	                                           
	sendCmd16Arg8(0xba00, 0x24);	
	sendCmd16Arg8(0xba01, 0x24);	
	sendCmd16Arg8(0xba02, 0x24);	
	                                           
	sendCmd16Arg8(0xb900, 0x24);	
	sendCmd16Arg8(0xb901, 0x24);	
	sendCmd16Arg8(0xb902, 0x24);	
	
	sendCmd16Arg8(0xf000, 0x55);          
	sendCmd16Arg8(0xf001, 0xaa);	
	sendCmd16Arg8(0xf002, 0x52);	
	sendCmd16Arg8(0xf003, 0x08);	
	sendCmd16Arg8(0xf004, 0x00);	
	                                           
	                                           
	sendCmd16Arg8(0xb100, 0xcc);	
	                                           
	                                           
	sendCmd16Arg8(0xbc00, 0x05);	
	sendCmd16Arg8(0xbc01, 0x05);	
	sendCmd16Arg8(0xbc02, 0x05);	
	                                           
	sendCmd16Arg8(0xb800, 0x01);	
	sendCmd16Arg8(0xb801, 0x03);	
	sendCmd16Arg8(0xb802, 0x03);	
	sendCmd16Arg8(0xb803, 0x03);	
	                                           
	                                           
	sendCmd16Arg8(0xbd02, 0x07);	
	sendCmd16Arg8(0xbd03, 0x31);	
	sendCmd16Arg8(0xbe02, 0x07);	
	sendCmd16Arg8(0xbe03, 0x31);	
	sendCmd16Arg8(0xbf02, 0x07);	
	sendCmd16Arg8(0xbf03, 0x31);	
	                                           
	                                           
	sendCmd16Arg8(0xff00, 0xaa);	
	sendCmd16Arg8(0xff01, 0x55);	
	sendCmd16Arg8(0xff02, 0x25);	
	sendCmd16Arg8(0xff03, 0x01);	
	sendCmd16Arg8(0xff04, 0x00);	
	
	sendCmd16Arg8(0xf304, 0x11);	
	sendCmd16Arg8(0xf306, 0x10);	
	sendCmd16Arg8(0xf308, 0x00);	
	                                           
	sendCmd16Arg8(0x3500, 0x00);	
	
	sendCmd16Arg8(0x2a00, 0x00);
	sendCmd16Arg8(0x2a01, 0x00);	
	sendCmd16Arg8(0x2a02, 0x01);	
	sendCmd16Arg8(0x2a03, 0xdf);	
	                                           
	sendCmd16Arg8(0x2b00, 0x00);  
	sendCmd16Arg8(0x2b01, 0x00);	
	sendCmd16Arg8(0x2b02, 0x03);	
	sendCmd16Arg8(0x2b03, 0x1f);	
	
	
	
	sendCmd16(0x1100);
	delay(150);
	
	sendCmd16(0x2900);
	delay(50);
	sendCmd16Arg8(0x3600, 0x00);
	sendCmd16Arg8(0x3A00, 0x77);
	delay(50);

  CSHigh();
}


FASTRUN void NT35510_t41_p::CSLow() 
{
  digitalWriteFast(_cs, LOW);       //Select TFT
}

FASTRUN void NT35510_t41_p::CSHigh() 
{
  digitalWriteFast(_cs, HIGH);       //Deselect TFT
}

FASTRUN void NT35510_t41_p::DCLow() 
{
  digitalWriteFast(_dc, LOW);       //Writing command to TFT
}

FASTRUN void NT35510_t41_p::DCHigh() 
{
  digitalWriteFast(_dc, HIGH);       //Writing data to TFT
}
/*
FASTRUN void NT35510_t41_p::microSecondDelay()
{
  for (uint32_t i=0; i<99; i++) __asm__("nop\n\t");
}*/
#pragma GCC push_options
#pragma GCC optimize ("O0")   
FASTRUN void NT35510_t41_p::microSecondDelay ()
{
	for (uint32_t volatile  i = 0; i < 100; i++)
		__asm__ volatile ("nop\n\t");
	//delayMicroseconds(1);
}
#pragma GCC pop_options



FASTRUN void NT35510_t41_p::gpioWrite(){
  pFlex->setIOPinToFlexMode(36);
  pinMode(37, OUTPUT);
  digitalWriteFast(37, HIGH);
}

FASTRUN void NT35510_t41_p::gpioRead(){
  pFlex->setIOPinToFlexMode(37);
  pinMode(36, OUTPUT);
  digitalWriteFast(36, HIGH);
}

FASTRUN void NT35510_t41_p::FlexIO_Init()
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
    hw->clock_gate_register |= hw->clock_gate_mask  ;
    /* Enable the FlexIO with fast access */
    p->CTRL = FLEXIO_CTRL_FLEXEN;
    
}

FASTRUN void NT35510_t41_p::FlexIO_Config_SnglBeat_Read()
{
    gpioWrite();

    p->CTRL &= ~FLEXIO_CTRL_FLEXEN;
    p->CTRL |= FLEXIO_CTRL_SWRST;
    p->CTRL &= ~FLEXIO_CTRL_SWRST;

    gpioRead();

    /* Configure the shifters */
    p->SHIFTCFG[3] = 
         FLEXIO_SHIFTCFG_SSTOP(0)                                              /* Shifter stop bit disabled */
       | FLEXIO_SHIFTCFG_SSTART(0)                                             /* Shifter start bit disabled and loading data on enabled */
       | FLEXIO_SHIFTCFG_PWIDTH(BUS_WIDTH-1);                                  /* Bus width */
     
    p->SHIFTCTL[3] = 
        FLEXIO_SHIFTCTL_TIMSEL(0)                                              /* Shifter's assigned timer index */
      | FLEXIO_SHIFTCTL_TIMPOL*(1)                                             /* Shift on negative edge of shift clock */
      | FLEXIO_SHIFTCTL_PINCFG(1)                                              /* Shifter's pin configured as output */
      | FLEXIO_SHIFTCTL_PINSEL(0)                                              /* Shifter's pin start index */
      | FLEXIO_SHIFTCTL_PINPOL*(0)                                             /* Shifter's pin active high */
      | FLEXIO_SHIFTCTL_SMOD(1);                                               /* Shifter mode as transmit */

    /* Configure the timer for shift clock */
    p->TIMCMP[0] = 
        (((1 * 2) - 1) << 8)                                                   /* TIMCMP[15:8] = number of beats x 2 – 1 */
      | ((60/2) - 1);                                                          /* TIMCMP[7:0] = baud rate divider / 2 – 1 */
    
    p->TIMCFG[0] = 
        FLEXIO_TIMCFG_TIMOUT(0)                                                /* Timer output logic one when enabled and not affected by reset */
      | FLEXIO_TIMCFG_TIMDEC(0)                                                /* Timer decrement on FlexIO clock, shift clock equals timer output */
      | FLEXIO_TIMCFG_TIMRST(0)                                                /* Timer never reset */
      | FLEXIO_TIMCFG_TIMDIS(2)                                                /* Timer disabled on timer compare */
      | FLEXIO_TIMCFG_TIMENA(2)                                                /* Timer enabled on trigger high */
      | FLEXIO_TIMCFG_TSTOP(1)                                                 /* Timer stop bit enabled */
      | FLEXIO_TIMCFG_TSTART*(0);                                              /* Timer start bit disabled */
    
    p->TIMCTL[0] = 
        FLEXIO_TIMCTL_TRGSEL((((3) << 2) | 1))                                 /* Timer trigger selected as shifter's status flag */
      | FLEXIO_TIMCTL_TRGPOL*(1)                                               /* Timer trigger polarity as active low */
      | FLEXIO_TIMCTL_TRGSRC*(1)                                               /* Timer trigger source as internal */
      | FLEXIO_TIMCTL_PINCFG(3)                                                /* Timer' pin configured as output */
      | FLEXIO_TIMCTL_PINSEL(19)                                               /* Timer' pin index: RD pin */
      | FLEXIO_TIMCTL_PINPOL*(1)                                               /* Timer' pin active low */
      | FLEXIO_TIMCTL_TIMOD(1);                                                /* Timer mode as dual 8-bit counters baud/bit */
 
    /* Enable FlexIO */
   p->CTRL |= FLEXIO_CTRL_FLEXEN;      

}


FASTRUN void NT35510_t41_p::FlexIO_Config_SnglBeat()
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
       | FLEXIO_SHIFTCFG_PWIDTH(BUS_WIDTH-1);                                  /* Bus width */
     
    p->SHIFTCTL[0] = 
        FLEXIO_SHIFTCTL_TIMSEL(0)                                              /* Shifter's assigned timer index */
      | FLEXIO_SHIFTCTL_TIMPOL * (0)                                             /* Shift on posedge of shift clock */
      | FLEXIO_SHIFTCTL_PINCFG(3)                                              /* Shifter's pin configured as output */
      | FLEXIO_SHIFTCTL_PINSEL(0)                                              /* Shifter's pin start index */
      | FLEXIO_SHIFTCTL_PINPOL * (0)                                             /* Shifter's pin active high */
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

FASTRUN void NT35510_t41_p::FlexIO_Config_MultiBeat()
{
    //uint32_t i;
    uint8_t beats = SHIFTNUM * BEATS_PER_SHIFTER;                                     //Number of beats = number of shifters * beats per shifter
    /* Disable and reset FlexIO */
    p->CTRL &= ~FLEXIO_CTRL_FLEXEN;
    p->CTRL |= FLEXIO_CTRL_SWRST;
    p->CTRL &= ~FLEXIO_CTRL_SWRST;

    gpioWrite();

    /* Configure the shifters */
    for (int i = 0; i <= SHIFTNUM - 1; i++)
    {
        p->SHIFTCFG[i] =
            FLEXIO_SHIFTCFG_INSRC * (1U)                                              /* Shifter input from next shifter's output */
            | FLEXIO_SHIFTCFG_SSTOP(0U)                                               /* Shifter stop bit disabled */
            | FLEXIO_SHIFTCFG_SSTART(0U)                                              /* Shifter start bit disabled and loading data on enabled */
            | FLEXIO_SHIFTCFG_PWIDTH(BUS_WIDTH - 1);                                  /* 8 bit shift width */
    }

    p->SHIFTCTL[0] =
        FLEXIO_SHIFTCTL_TIMSEL(0)                                                     /* Shifter's assigned timer index */
        | FLEXIO_SHIFTCTL_TIMPOL * (0U)                                               /* Shift on posedge of shift clock */
        | FLEXIO_SHIFTCTL_PINCFG(3U)                                                  /* Shifter's pin configured as output */
        | FLEXIO_SHIFTCTL_PINSEL(0)                                                   /* Shifter's pin start index */
        | FLEXIO_SHIFTCTL_PINPOL * (0U)                                               /* Shifter's pin active high */
        | FLEXIO_SHIFTCTL_SMOD(2U);                                                   /* shifter mode transmit */

    for (int i = 1; i <= SHIFTNUM - 1; i++)
    {
        p->SHIFTCTL[i] =
            FLEXIO_SHIFTCTL_TIMSEL(0)                                                 /* Shifter's assigned timer index */
            | FLEXIO_SHIFTCTL_TIMPOL * (0U)                                           /* Shift on posedge of shift clock */
            | FLEXIO_SHIFTCTL_PINCFG(0U)                                              /* Shifter's pin configured as output disabled */
            | FLEXIO_SHIFTCTL_PINSEL(0)                                               /* Shifter's pin start index */
            | FLEXIO_SHIFTCTL_PINPOL * (0U)                                           /* Shifter's pin active high */
            | FLEXIO_SHIFTCTL_SMOD(2U);
    }

    /* Configure the timer for shift clock */
    p->TIMCMP[0] =
        ((beats * 2U - 1) << 8)                                                       /* TIMCMP[15:8] = number of beats x 2 – 1 */
        | (_baud_div / 2U - 1U);                                                      /* TIMCMP[7:0] = shift clock divide ratio / 2 - 1 */

    p->TIMCFG[0] =   FLEXIO_TIMCFG_TIMOUT(0U)                                         /* Timer output logic one when enabled and not affected by reset */
                     | FLEXIO_TIMCFG_TIMDEC(0U)                                       /* Timer decrement on FlexIO clock, shift clock equals timer output */
                     | FLEXIO_TIMCFG_TIMRST(0U)                                       /* Timer never reset */
                     | FLEXIO_TIMCFG_TIMDIS(2U)                                       /* Timer disabled on timer compare */
                     | FLEXIO_TIMCFG_TIMENA(2U)                                       /* Timer enabled on trigger high */
                     | FLEXIO_TIMCFG_TSTOP(0U)                                        /* Timer stop bit disabled */
                     | FLEXIO_TIMCFG_TSTART * (0U);                                   /* Timer start bit disabled */

    p->TIMCTL[0] =
        FLEXIO_TIMCTL_TRGSEL(((SHIFTNUM - 1) << 2) | 1U)                              /* Timer trigger selected as highest shifter's status flag */
        | FLEXIO_TIMCTL_TRGPOL * (1U)                                                 /* Timer trigger polarity as active low */
        | FLEXIO_TIMCTL_TRGSRC * (1U)                                                 /* Timer trigger source as internal */
        | FLEXIO_TIMCTL_PINCFG(3U)                                                    /* Timer' pin configured as output */
        | FLEXIO_TIMCTL_PINSEL(18)                                                    /* Timer' pin index: WR pin */
        | FLEXIO_TIMCTL_PINPOL * (1U)                                                 /* Timer' pin active low */
        | FLEXIO_TIMCTL_TIMOD(1U);                                                    /* Timer mode 8-bit baud counter */
    /* Enable FlexIO */
   p->CTRL |= FLEXIO_CTRL_FLEXEN;

   // configure interrupts
    attachInterruptVector(hw->flex_irq, ISR);
    NVIC_ENABLE_IRQ(hw->flex_irq);
    NVIC_SET_PRIORITY(hw->flex_irq, FLEXIO_ISR_PRIORITY);

    // disable interrupts until later
    p->SHIFTSIEN &= ~(1 << SHIFTER_IRQ);
    p->TIMIEN &= ~(1 << TIMER_IRQ);
}

FASTRUN void NT35510_t41_p::SglBeatWR_nPrm_8(uint32_t const cmd, const uint8_t *value = NULL, uint32_t const length = 0)
{
  while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
    
  }
  

    FlexIO_Config_SnglBeat();
     uint32_t i;
    /* Assert CS, RS pins */
    
    //delay(1);
    CSLow();
    DCLow();

    /* Write command index */
    p->SHIFTBUF[0] = cmd;

    /*Wait for transfer to be completed */
    while(0 == (p->SHIFTSTAT & (1 << 0)))
    {
    }
    while(0 == (p->TIMSTAT & (1 << 0)))
            {  
            }

    /* De-assert RS pin */
    
    microSecondDelay();
    DCHigh();
    microSecondDelay();
    if(length)
    {
        for(i = 0; i < length; i++)
        {    
            p->SHIFTBUF[0] = *value++;
            while(0 == (p->SHIFTSTAT & (1 << 0)))
            {  
            }
        }
        while(0 == (p->TIMSTAT & (1 << 0)))
            {  
            }
    }
    microSecondDelay();
    CSHigh();
    
    /* De-assert CS pin */
}

FASTRUN void NT35510_t41_p::SglBeatWR_nPrm_16 (uint32_t const cmd, const uint16_t *value, uint32_t const length)
{
 while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
  }
    FlexIO_Config_SnglBeat();
    
    /* Assert CS, RS pins */
    CSLow();
    DCLow();
 
    /* Write command index */
    p->SHIFTBUF[0] = cmd;
 
    /*Wait for transfer to be completed */
    while(0 == (p->TIMSTAT & (1 << 0)))
            {  
            }
    microSecondDelay();
    /* De-assert RS pin */
    DCHigh();
    microSecondDelay();

    if (length){
#if (BUS_WIDTH == 8)
    uint16_t buf;
      for(uint32_t i=0; i<length-1U; i++)
        {
          buf = *value++;
            while(0 == (p->SHIFTSTAT & (1U << 0)))
            { 
            }
            p->SHIFTBUF[0] = buf >> 8;

            while(0 == (p->SHIFTSTAT & (1U << 0)))
            {
            }
            p->SHIFTBUF[0] = buf & 0xFF;
        }
        buf = *value++;
        /* Write the last byte */
        while(0 == (p->SHIFTSTAT & (1U << 0)))
            {
            }
        p->SHIFTBUF[0] = buf >> 8;

        while(0 == (p->SHIFTSTAT & (1U << 0)))
        {
        }
        p->TIMSTAT |= (1U << 0);

        p->SHIFTBUF[0] = buf & 0xFF;

        /*Wait for transfer to be completed */
        while(0 == (p->TIMSTAT |= (1U << 0)))
        {
        }
    

#else
      for(uint32_t i=0; i<length-1U; i++)
      {
      while(0 == (p->SHIFTSTAT & (1U << 0)))
      {
      }
      p->SHIFTBUF[0] = *value++;
      }
      //Write the last byte
      while(0 == (p->SHIFTSTAT & (1U << 0)))
      {
      }
      p->TIMSTAT |= (1U << 0);

          p->SHIFTBUF[0] = *value++;

          /*Wait for transfer to be completed */
          while(0 == (p->TIMSTAT |= (1U << 0)))
          {
          }
#endif
    }
    microSecondDelay();
    CSHigh();
}

FASTRUN uint8_t NT35510_t41_p::readCommand (const uint16_t cmd)
{
  while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
  }

    FlexIO_Config_SnglBeat();
    DCLow();

    /* Write command index */
    p->SHIFTBUF[0] = cmd;

    /*Wait for transfer to be completed */
    while(0 == (p->SHIFTSTAT & (1 << 0)))
    {
    }
    while(0 == (p->TIMSTAT & (1 << 0)))
            {  
            }
    /* De-assert RS pin */
    microSecondDelay();
    DCHigh();
    FlexIO_Config_SnglBeat_Read();

    uint8_t dummy = 0;
    uint8_t data = 0;

    while (0 == (p->SHIFTSTAT & (1 << 3)))
        {
        }
    dummy = p->SHIFTBUFBYS[3];

    while (0 == (p->SHIFTSTAT & (1 << 3)))
        {
        }
    data = p->SHIFTBUFBYS[3];

    Serial.printf("Dummy 0x%x, data 0x%x\n", dummy, data);
    
    //Set FlexIO back to Write mode
    FlexIO_Config_SnglBeat();
  return data;


}


NT35510_t41_p * NT35510_t41_p::IRQcallback = nullptr;


FASTRUN void NT35510_t41_p::MulBeatWR_nPrm_IRQ(uint32_t const cmd,  const void *value, uint32_t const length) 
{
  while(WR_IRQTransferDone == false)
  {
    //Wait for any DMA transfers to complete
  }
    FlexIO_Config_SnglBeat();
    CSLow();
    DCLow();

    /* Write command index */
    p->SHIFTBUF[0] = cmd;

    /*Wait for transfer to be completed */

    while(0 == (p->TIMSTAT & (1 << 0)))
            {  
            }
    microSecondDelay();
    /* De-assert RS pin */
    DCHigh();
    microSecondDelay();


    FlexIO_Config_MultiBeat();
    WR_IRQTransferDone = false;
    uint32_t bytes = length*2U;

    bursts_to_complete = bytes / BYTES_PER_BURST;

    int remainder = bytes % BYTES_PER_BURST;
    if (remainder != 0) {
        memset(finalBurstBuffer, 0, sizeof(finalBurstBuffer));
        memcpy(finalBurstBuffer, (uint8_t*)value + bytes - remainder, remainder);
        bursts_to_complete++;
    }

    bytes_remaining = bytes;
    readPtr = (uint32_t*)value;
    //Serial.printf ("arg addr: %x, readPtr addr: %x \n", value, readPtr);
    //Serial.printf("START::bursts_to_complete: %d bytes_remaining: %d \n", bursts_to_complete, bytes_remaining);
  
    uint8_t beats = SHIFTNUM * BEATS_PER_SHIFTER;
    p->TIMCMP[0] = ((beats * 2U - 1) << 8) | (_baud_div / 2U - 1U);
    p->TIMSTAT = (1 << TIMER_IRQ); // clear timer interrupt signal
    
    asm("dsb");
    
    IRQcallback = this;
    // enable interrupts to trigger bursts
    p->TIMIEN |= (1 << TIMER_IRQ);
    p->SHIFTSIEN |= (1 << SHIFTER_IRQ);
    
}

FASTRUN void NT35510_t41_p::_onCompleteCB()
{
  if (_callback){
        _callback();
      }
      return;
}

FASTRUN void NT35510_t41_p::flexIRQ_Callback(){
  
 if (p->TIMSTAT & (1 << TIMER_IRQ)) { // interrupt from end of burst
        p->TIMSTAT = (1 << TIMER_IRQ); // clear timer interrupt signal
        bursts_to_complete--;
        if (bursts_to_complete == 0) {
            p->TIMIEN &= ~(1 << TIMER_IRQ); // disable timer interrupt
            asm("dsb");
            WR_IRQTransferDone = true;
            microSecondDelay();
            CSHigh();
            _onCompleteCB();
            return;
        }
    }

    if (p->SHIFTSTAT & (1 << SHIFTER_IRQ)) { // interrupt from empty shifter buffer
        // note, the interrupt signal is cleared automatically when writing data to the shifter buffers
        if (bytes_remaining == 0) { // just started final burst, no data to load
            p->SHIFTSIEN &= ~(1 << SHIFTER_IRQ); // disable shifter interrupt signal
        } else if (bytes_remaining < BYTES_PER_BURST) { // just started second-to-last burst, load data for final burst
            uint8_t beats = bytes_remaining / BYTES_PER_BEAT;
            p->TIMCMP[0] = ((beats * 2U - 1) << 8) | (_baud_div / 2U - 1); // takes effect on final burst
            readPtr = finalBurstBuffer;
            bytes_remaining = 0;
            for (int i = 0; i < SHIFTNUM; i++) {
                uint32_t data = *readPtr++;
                p->SHIFTBUFBYS[i] = ((data >> 16) & 0xFFFF) | ((data << 16) & 0xFFFF0000);
            }
        } else {
            bytes_remaining -= BYTES_PER_BURST;
            for (int i = 0; i < SHIFTNUM; i++) {
                uint32_t data = *readPtr++;
                p->SHIFTBUFBYS[i] = ((data >> 16) & 0xFFFF) | ((data << 16) & 0xFFFF0000);
        }
    }
  }
    asm("dsb");
}



FASTRUN void NT35510_t41_p::ISR()
{
  asm("dsb");
  IRQcallback->flexIRQ_Callback();
}

#endif
