
/*
	Tab @ 4 spaces
*/


#if USE_FLEXTFT_R61529

#include "R61529_t41_p.h"


static R61529_t41_p STORAGETYPE lcd = R61529_t41_p(TFT_RS, TFT_CS, TFT_RST, TFT_BL);


FLASHMEM R61529_t41_p::R61529_t41_p (int8_t dc, int8_t cs, int8_t rst, int8_t bl) 
{
	_dc = dc;
	_cs = cs;
	_rst = rst;
	_bl = bl;
}


PROGMEM static const setting_table_t power_on_setting_table[] = {
	{R61529_RESET,     0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 100},
	{R61529_SLPOUT,    0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 20},
	{R61529_DISLYON,   0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 120},
	{R61529_MCAP,      1, {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	{R61529_BLCTRL1,  20, {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_BLCTRL2,   4, {0x01, 0x00, 0xFF, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_FMAIS,     5, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 2},
	{R61529_DISMODE,   1, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_PANDSET,   8, {0x04, 0xDF, 0x40, 0x10, 0x00, 0x01, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_DTSNM,     5, {0x07, 0x27, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_SGDTSET,   4, {0x57, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_DPIPCTL,   1, {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	
	{R61529_GAMSETA,  24, {0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04, 0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04}, 0},
	{R61529_GAMSETB,  24, {0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04, 0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04}, 0},
	{R61529_GAMSETC,  24, {0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04, 0x03, 0x12, 0x1A, 0x24, 0x32, 0x4B, 0x3B, 0x29, 0x1F, 0x18, 0x12, 0x04}, 0},
	
	{R61529_PSCPSET,  16, {0x99, 0x06, 0x08, 0x20, 0x29, 0x04, 0x01, 0x00, 0x08, 0x01, 0x00, 0x06, 0x01, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_VCOMSET,   4, {0x00, 0x20, 0x20, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_NVMACTL,   4, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_DDBWCTL,   6, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_NVMLCTL,   1, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_SETPIXF,   1, {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_CASET,     4, {0x00, 0x00, 0x01, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0},
	{R61529_PASET,     4, {0x00, 0x00, 0x01, 0xDF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0}
};

FLASHMEM void R61529_t41_p::LCDSettingTableWrite (const setting_table_t *table)
{
	SglBeatWR_nPrm_8(table->command, table->parameter, table->parameters);
	if ((table->wait) > 0)
		delay(table->wait);
}

FLASHMEM void R61529_t41_p::setRotation (const uint8_t r) 
{ 
	_rotation = r&0x03;

	switch (_rotation){
      case 0:	
      case 2: _width  = R61529_TFTWIDTH;
              _height = R61529_TFTHEIGHT;
             break;
      case 1: 
      case 3: _width  = R61529_TFTHEIGHT;
              _height = R61529_TFTWIDTH;
			 break;
	}

	SglBeatWR_nPrm_8(R61529_MADCTL, &MADCTL[_rotation], 1);
	delay(1);
}

FLASHMEM void R61529_t41_p::init_display ()
{		
	// toggle RST low to resetset
	digitalWriteFast(_rst, LOW);
	delay(20);
	digitalWriteFast(_rst, HIGH);
	delay(20);

	CSLow();
	
	for (int i = 0; i < (int)(sizeof(power_on_setting_table) / sizeof(setting_table_t)); i++)
		LCDSettingTableWrite(&power_on_setting_table[i]);

	CSHigh();
	delay(5);
}

FLASHMEM void R61529_t41_p::setBacklight (const uint8_t value)
{
    analogWrite(_bl, value&0x7F);
}

FLASHMEM void R61529_t41_p::begin (const uint8_t baud_div) 
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
  
	pinMode(_cs, OUTPUT);  // CS
	pinMode(_dc, OUTPUT);  // DC
	pinMode(_rst, OUTPUT); // RST
	pinMode(_bl, OUTPUT);
  
	*(portControlRegister(_cs)) = 0xFF;
	*(portControlRegister(_dc)) = 0xFF;
	*(portControlRegister(_rst)) = 0xFF;
  
	digitalWriteFast(_cs, HIGH);
	digitalWriteFast(_dc, HIGH);
	digitalWriteFast(_rst, HIGH);

	FlexIO_Init();
	displayInit();

	_width  = R61529_TFTWIDTH;
	_height = R61529_TFTHEIGHT;

}

FLASHMEM void R61529_t41_p::onCompleteCB (CBF callback)
{
	_callback = callback;
	isCB = true;
}

FASTRUN void R61529_t41_p::setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
	uint8_t CommandValue[4];
		
	uint8_t Command = R61529_CASET;
	CommandValue[0U] = x1 >> 8U;
	CommandValue[1U] = x1 & 0xFF;
	CommandValue[2U] = x2 >> 8U;
	CommandValue[3U] = x2 & 0xFF;
	SglBeatWR_nPrm_8(Command, CommandValue, 4U);
		
	Command = R61529_PASET;
	CommandValue[0U] = y1 >> 8U;
	CommandValue[1U] = y1 & 0xFF;
	CommandValue[2U] = y2 >> 8U;
	CommandValue[3U] = y2 & 0xFF;
	SglBeatWR_nPrm_8(Command, CommandValue, 4U);

}

FASTRUN void R61529_t41_p::pushPixels16bit (const uint16_t *pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}

	uint32_t area = ((x2-x1)+1) * ((y2-y1)+1);
  
	if (!((_lastx1 == x1) && (_lastx2 == x2) && (_lasty1 == y1) && (_lasty2 == y2))){


		setAddrWindow(x1, y1, x2, y2);
		_lastx1 = x1; _lastx2 = x2; _lasty1 = y1; _lasty2 = y2;
	}
  
	SglBeatWR_nPrm_16(R61529_RAMWR, pcolors, area);
	
	while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}

FASTRUN void R61529_t41_p::pushPixels16bitAsync (const uint16_t *pcolors, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{	
	while (WR_IRQTransferDone == false){
    	//Wait for any DMA transfers to complete
	}
	uint32_t area = ((x2-x1)+1) * ((y2-y1)+1);
  
	if (!((_lastx1 == x1) && (_lastx2 == x2) && (_lasty1 == y1) && (_lasty2 == y2))){
  		setAddrWindow(x1, y1, x2, y2);
  		_lastx1 = x1; _lastx2 = x2; _lasty1 = y1; _lasty2 = y2;
	}
  
  	MulBeatWR_nPrm_IRQ(R61529_RAMWR, pcolors, area);
  	
  	while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}

FLASHMEM void R61529_t41_p::displayInit () 
{
	setBacklight(TFT_INTENSITY);
	init_display();
	setRotation(3);
}

FASTRUN void R61529_t41_p::CSLow () 
{
	digitalWriteFast(_cs, LOW);      // Select TFT
}

FASTRUN void R61529_t41_p::CSHigh () 
{
	digitalWriteFast(_cs, HIGH);     // Deselect TFT
}

FASTRUN void R61529_t41_p::DCLow () 
{
	digitalWriteFast(_dc, LOW);      // Writing command to TFT
}

FASTRUN void R61529_t41_p::DCHigh () 
{
	digitalWriteFast(_dc, HIGH);     // Writing data to TFT
}


#pragma GCC push_options
#pragma GCC optimize ("O0")   
FASTRUN void R61529_t41_p::microSecondDelay ()
{
	for (uint32_t volatile  i = 0; i < 5; i++)
		__asm__ volatile ("nop\n\t");
	//delayMicroseconds(1);
}
#pragma GCC pop_options


FASTRUN inline void R61529_t41_p::gpioWrite ()
{
	pFlex->setIOPinToFlexMode(36);
	pinMode(37, OUTPUT);
	digitalWriteFast(37, HIGH);
}

FASTRUN inline void R61529_t41_p::gpioRead ()
{
	pFlex->setIOPinToFlexMode(37);
	pinMode(36, OUTPUT);
	digitalWriteFast(36, HIGH);
}

FASTRUN void R61529_t41_p::FlexIO_Init ()
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

FASTRUN void R61529_t41_p::FlexIO_Config_SnglBeat_Read ()
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


FASTRUN void R61529_t41_p::FlexIO_Config_SnglBeat ()
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

FASTRUN void R61529_t41_p::FlexIO_Config_MultiBeat ()
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

FASTRUN void R61529_t41_p::SglBeatWR_nPrm_8(uint32_t const cmd, const uint8_t *value = NULL, uint32_t const length = 0)
{
	while (WR_IRQTransferDone == false){
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
    
    while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}

FASTRUN void R61529_t41_p::SglBeatWR_nPrm_16 (uint32_t const cmd, const uint16_t *value, uint32_t const _length)
{
	
	volatile uint32_t length = _length;
	
	while (WR_IRQTransferDone == false){
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
		for (volatile uint32_t i = 0; i < length-1U; i++){
      		while(0 == (p->SHIFTSTAT & (1U << 0))){
      		
      		}

      		p->SHIFTBUF[0] = *value++;
      		
		}
    	
		//Write the last byte
		while(0 == (p->SHIFTSTAT & (1U << 0))){
			
		}
		p->TIMSTAT |= (1U << 0);
		p->SHIFTBUF[0] = *value++;

        /*Wait for transfer to be completed */
        while(0 == (p->TIMSTAT |= (1U << 0))){
        }
#endif
    }
    microSecondDelay();
    CSHigh();
    
	while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}

FASTRUN uint8_t R61529_t41_p::readCommand (const uint16_t cmd)
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


R61529_t41_p * R61529_t41_p::IRQcallback = nullptr;


FASTRUN void R61529_t41_p::MulBeatWR_nPrm_IRQ (uint32_t const cmd,  const void *value, uint32_t const length) 
{
	while(WR_IRQTransferDone == false){
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
    
   	while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}

FASTRUN void R61529_t41_p::_onCompleteCB ()
{
	if (_callback)
        _callback();
}

FASTRUN void R61529_t41_p::flexIRQ_Callback ()
{
  
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
    
    while (WR_IRQTransferDone == false){
   		//Wait for any DMA transfers to complete
	}
}



FASTRUN void R61529_t41_p::ISR ()
{
	asm("dsb");
	IRQcallback->flexIRQ_Callback();
}

#endif

