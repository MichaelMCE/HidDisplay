
/*

	To improve USB throughput, increase RX_NUM (usb_rawhid.c) to max, which is currently 512

	Tab @ 4 spaces
*/


#if USE_FLEXTFT_NT35516

#include "NT35516_t41_p.h"

static NT35516_t41_p STORAGETYPE lcd = NT35516_t41_p(TFT_RS, TFT_CS, TFT_RST, TFT_BL);


#define FIXUP_COLOUR_JITTER		0
#define REGFLAG_DELAY			0xFE
#define REGFLAG_END_OF_TABLE	0x00   // end of list marker

PROGMEM static const cmdTable_t cmdRegistertable[] = {
	
	{0x34,  1,  {0x00}},//turn off te - signal trace isn't brought out (available) on my display
	{0xFF,	4,	{0xAA, 0x55, 0x25,0x01}},
	{0xF2,	35,	{0x00, 0x00, 0x4A,0x0A,0xA8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x01,0x51,0x00,0x01,0x00,0x01}},
	{0xF3,	7,	{0x02, 0x03, 0x07,0x45,0x88,0xD4,0x0D}},
	{0xF0,	5,	{0x55, 0xAA, 0x52,0x08,0x00}},
	{0xB1,	3,	{0x7C, 0x00, 0x00}},
	{0xB8,	4,	{0x01, 0x02, 0x02,0x02}},
	{0xBB,	3,	{0x63, 0x03,0x63}},
	{0xBC,	3,	{0x00, 0x00,0x00}},
	//{0xBD,	5,	{0x01, 0x41, 0x10,0x38, 0x01}},
	{0xC9,	6,	{0x63, 0x06, 0x0D,0x1A,0x17,0x00}},
	{0xF0,	5,	{0x55, 0xAA, 0x52,0x08,0x01}},
	{0xB0,	3,	{0x05, 0x05, 0x05}},
	{0xB1,	3,	{0x05, 0x05, 0x05}},
	{0xB2,	3,	{0x01, 0x01, 0x01}},
	{0xB3,	3,	{0x0E, 0x0E, 0x0E}},
	{0xB4,	3,	{0x0A, 0x0A, 0x0A}},
	{0xB6,	3,	{0x44, 0x44, 0x44}},
	{0xB7,	3,	{0x34, 0x34, 0x34}},
	{0xB8,	3,	{0x20, 0x20, 0x20}},
	{0xB9,	3,	{0x26, 0x26, 0x26}},
	{0xBA,	3,	{0x24, 0x24, 0x24}},
	{0xBC,	3,	{0x00, 0xC8, 0x00}},
	{0xBD,	3,	{0x00, 0xC8, 0x00}},
	
	{0xBE,	1,	{0x80}},   //96
	{0xC0,	2,	{0x00,0x08}},
	{0xCA,	1,	{0x00}},
	{0xD0,	4,	{0x0A,0x10,0x0D,0x0F}},
	
	{0xD1,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xD2,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xD3,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xD4,	4,	{0x03,0xFe,0x03,0xFF}},	
	
	{0xD5,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xD6,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xD7,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xD8,	4,	{0x03,0xFd,0x03,0xFF}},
	
	{0xD9,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xDd,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xDe,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xDf,	4,	{0x03,0xFd,0x03,0xFF}},	 
	
	{0xe0,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xe1,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xe2,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xe3,	4,	{0x03,0xFd,0x03,0xFF}},	
	
	{0xe4,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xe5,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xe6,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xe7,	4,	{0x03,0xFd,0x03,0xFF}},
	
	{0xe8,	16,	{0x00,0x70,0x01,0x16,0x01,0x2A,0x01,0x48,0x01,0x61,0x01,0x7D,0x01,0x94,0x01,0xb7}},
	{0xe9,	16,	{0x01,0xd1,0x01,0xfe,0x02,0x1f,0x02,0x55,0x02,0x80,0x02,0x81,0x02,0xad,0x02,0xdf}},
	{0xea,	16,	{0x02,0xf7,0x03,0x1f,0x03,0x3a,0x03,0x59,0x03,0x70,0x03,0x8b,0x03,0x99,0x03,0xae}},
	{0xeb,	4,	{0x03,0xFd,0x03,0xFF}},	  

	{0x3A, 1, {0x05}},
	{0x2c, 1, {0x00}},
	
	{0x36, 1, {NT35516_IXFORMAT}},
	{REGFLAG_DELAY, 2, {}},
   
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 5, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


FLASHMEM void NT35516_t41_p::setDisplayRegisters (const cmdTable_t *table)
{
	gpioWrite();
	
	for (int c = 0; table[c].cmd != REGFLAG_END_OF_TABLE; c++){
		const cmdTable_t * const cmd = &table[c];
		
		if (cmd->cmd == REGFLAG_DELAY){
			delay(cmd->count);
		}else{
			for (int i = 0; i < cmd->count; i++)
				sendCmd16Arg8((cmd->cmd<<8) + i, cmd->args[i]);
		}
	}
}

FASTRUN void NT35516_t41_p::sendCmd16Arg8 (const uint16_t cmd, const uint8_t arg)
{
	uint16_t arg16[2] = {arg, 0};

   	sglBeatWR_nPrm_16(cmd, arg16, 1);
}

FASTRUN void NT35516_t41_p::sendCmd16 (const uint16_t cmd)
{
	uint16_t arg16[2] = {0, 0};

   	sglBeatWR_nPrm_16(cmd, arg16, 1);
}

FLASHMEM NT35516_t41_p::NT35516_t41_p (int8_t dc, int8_t cs, int8_t rst, int8_t bl) 
{
	_dc = dc;
	_cs = cs;
	_rst = rst;
	_bl = bl;
}

FLASHMEM void NT35516_t41_p::setRotation (const uint8_t r) 
{ 
	_rotation = r&0x03;

	switch (_rotation){
	  case 0:	
	  case 2:_width  = NT35516_TFTWIDTH;
			 _height = NT35516_TFTHEIGHT;
			 break;
	  case 1: 
	  case 3:_width  = NT35516_TFTHEIGHT;
			 _height = NT35516_TFTWIDTH;
			 break;
	}

	gpioWrite();
	sendCmd16Arg8(NT35516_MADCTL, MADCTL[_rotation]);
	delay(5);
}

FLASHMEM void NT35516_t41_p::display_init ()
{		
	setBacklight(TFT_INTENSITY);
	
	digitalWriteFast(_rst, LOW);
	delay(125);
	digitalWriteFast(_rst, HIGH);
	delay(10);
	
	/*
	gpioWrite();
	sendCmd16(NT35516_SLPOUT); // Sleep Out
	delay(125);
	sendCmd16(NT35516_DISPON); // Display On
	delay(10);*/
	
	CSLow();
	setDisplayRegisters(cmdRegistertable);
	CSHigh();
	
	setRotation(3);
	
	//sendCmd16Arg8(0x2100, 0);		// display inversion
	sendCmd16Arg8(0x3800, 0);		// idle mode off
	sendCmd16Arg8(0x1300, 0);		// normal mode one
	sendCmd16Arg8(0x3400, 0);		// turn off TE signal

}

FLASHMEM void NT35516_t41_p::setBacklight (const uint8_t value)
{
	analogWrite(_bl, value&0x7F);
}

FLASHMEM void NT35516_t41_p::begin (const uint8_t baud_div) 
{

	pinMode(37, OUTPUT);		// read pin
	pinMode(36, OUTPUT);		// write pin
	pinMode(_cs, OUTPUT);		// CS
	pinMode(_dc, OUTPUT);		// DC
	pinMode(_rst, OUTPUT);		// RST
	pinMode(_bl, OUTPUT);

	*(portControlRegister(_cs))  = 0xFF;
	*(portControlRegister(_dc))  = 0xFF;
	*(portControlRegister(_rst)) = 0xFF;
  
	digitalWriteFast(36, HIGH);
	digitalWriteFast(37, HIGH);
	digitalWriteFast(_cs, HIGH);
	digitalWriteFast(_dc, HIGH);
	digitalWriteFast(_rst, HIGH);
	
	_width  = NT35516_TFTWIDTH;
	_height = NT35516_TFTHEIGHT;
		
	switch (baud_div){		// calculated referencing a base freq of 240mhz. 480 / 2 / _baud_div = freq
	case   2: _baud_div =120; break;
	case   4: _baud_div = 60; break;
	case   8: _baud_div = 30; break;
	case  12: _baud_div = 20; break;
	case  13: _baud_div = 18; break;
	case  14: _baud_div = 17; break;
	case  15: _baud_div = 16; break;
	case  16: _baud_div = 15; break;
	case  17: _baud_div = 14; break;
	case  18: _baud_div = 13; break;
	case  20: _baud_div = 12; break;
	case  22: _baud_div = 11; break;
	case  24: _baud_div = 10; break;
	case  27: _baud_div =  9; break;
	case  30: _baud_div =  8; break;
	case  34: _baud_div =  7; break;
	case  40: _baud_div =  6; break;
	case  48: _baud_div =  5; break;
	case  60: _baud_div =  4; break;
	case  80: _baud_div =  3; break;
	case 120: _baud_div =  2; break;
	case 240: _baud_div =  1; break;
	default:  _baud_div = 20; break;
	}

	flexIO_init();
	flexIO_Config_snglBeat();
	display_init();

}

FASTRUN void NT35516_t41_p::setAddrWindow (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
	gpioWrite();
		
	sendCmd16Arg8(NT35516_CASET + 0, x1 >> 8);
	sendCmd16Arg8(NT35516_CASET + 1, x1 & 0xFF);
	sendCmd16Arg8(NT35516_CASET + 2, x2 >> 8);
	sendCmd16Arg8(NT35516_CASET + 3, x2 & 0xFF);

	sendCmd16Arg8(NT35516_RASET + 0, y1 >> 8);
	sendCmd16Arg8(NT35516_RASET + 1, y1 & 0xFF);
	sendCmd16Arg8(NT35516_RASET + 2, y2 >> 8);
	sendCmd16Arg8(NT35516_RASET + 3, y2 & 0xFF);
	
	//sendCmd16(NT35516_RAMWR);
}
	
FASTRUN void NT35516_t41_p::pushPixels16bit (uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{

#if 0
	digitalWriteFast(36, HIGH);
	setAddrWindow(x1, y1, x2, y2);
	digitalWriteFast(36, HIGH);

	const int area = ((x2-x1)+1) * ((y2-y1)+1);
	
	// hack to fixup jittering that occurs when any colour component is at full brightness.
	// try disabling this.
#if FIXUP_COLOUR_JITTER
	for (int i = 0; i < area; i++)
		pixels[i] &= 0xF7DE;
#endif

	digitalWriteFast(36, HIGH);
	sglBeatWR_nPrm_16(NT35516_RAMWR, (uint16_t*)pixels, area);
	digitalWriteFast(36, HIGH);
	//sglBeatWR_nPrm_16(0x3c00, (uint16_t*)pixels, area);

#else

	// Using TE is necessary with this controller, but isn't brought out on my display (IPS1P1399).
	// This below, through trial and error, I've found to bring the most consistantly stable results.
	// To help remove jittering/smearing, clamp each colour component to 246, or less, before sending to the microcontroller
	
	const int width = (x2 - x1) + 1;
	const int height = (y2 - y1) + 1;
	
	// refresh line by line
	for (int i = 0; i < height; i++){
		setAddrWindow(x1, y1+i, x2, y1+i);
		digitalWriteFast(36, HIGH);
		
		sglBeatWR_nPrm_16(NT35516_RAMWR, &pixels[i * width], width);

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

FASTRUN void NT35516_t41_p::CSLow () 
{
	digitalWriteFast(_cs, LOW);	  // Select TFT
}

FASTRUN void NT35516_t41_p::CSHigh () 
{
	digitalWriteFast(_cs, HIGH);	 // Deselect TFT
}

FASTRUN void NT35516_t41_p::DCLow () 
{
	digitalWriteFast(_dc, LOW);	  // Writing command to TFT
}

FASTRUN void NT35516_t41_p::DCHigh () 
{
	digitalWriteFast(_dc, HIGH);	 // Writing data to TFT
}

FASTRUN void NT35516_t41_p::gpioWrite ()
{
	pinMode(37, OUTPUT);			// disable rd
	digitalWriteFast(37, HIGH);
	pFlex->setIOPinToFlexMode(36);	// enable write
}

FASTRUN void NT35516_t41_p::flexIO_init ()
{
	/* Get a FlexIO channel */
	pFlex = FlexIOHandler::flexIOHandler_list[2]; // use FlexIO3
	/* Pointer to the port structure in the FlexIO channel */
	p = &pFlex->port();
	/* Pointer to the hardware structure in the FlexIO channel */
	hw = &pFlex->hardware();

	/* Pins setup */
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

	digitalWrite(36, HIGH);
	digitalWrite(37, HIGH);


	/* High speed and drive strength configuration */
	uint32_t value = 0xFF;
	*(portControlRegister(36)) = value;
	*(portControlRegister(37)) = value; 

	*(portControlRegister(19)) = value;
	*(portControlRegister(18)) = value;
	*(portControlRegister(14)) = value;
	*(portControlRegister(15)) = value;
	*(portControlRegister(40)) = value;
	*(portControlRegister(41)) = value;
	*(portControlRegister(17)) = value;
	*(portControlRegister(16)) = value;

#if (BUS_WIDTH == 16)
	*(portControlRegister(22)) = value;
	*(portControlRegister(23)) = value;
	*(portControlRegister(20)) = value;
	*(portControlRegister(21)) = value;
	*(portControlRegister(38)) = value;
	*(portControlRegister(39)) = value;
	*(portControlRegister(26)) = value;
	*(portControlRegister(27)) = value;
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

FASTRUN void NT35516_t41_p::flexIO_Config_snglBeat ()
{
	gpioWrite();

	p->CTRL &= ~FLEXIO_CTRL_FLEXEN;
	p->CTRL |=  FLEXIO_CTRL_SWRST;
	p->CTRL &= ~FLEXIO_CTRL_SWRST;

	/* Configure the shifters */
	p->SHIFTCFG[0] = 
		 FLEXIO_SHIFTCFG_INSRC*(1)								/* Shifter input */
	   | FLEXIO_SHIFTCFG_SSTOP(0)								/* Shifter stop bit disabled */
	   | FLEXIO_SHIFTCFG_SSTART(0)								/* Shifter start bit disabled and loading data on enabled */
	   | FLEXIO_SHIFTCFG_PWIDTH(BUS_WIDTH-1);					/* Bus width */
	 
	p->SHIFTCTL[0] = 
		FLEXIO_SHIFTCTL_TIMSEL(0)								/* Shifter's assigned timer index */
	  | FLEXIO_SHIFTCTL_TIMPOL*(0)								/* Shift on posedge of shift clock */
	  | FLEXIO_SHIFTCTL_PINCFG(3)								/* Shifter's pin configured as output */
	  | FLEXIO_SHIFTCTL_PINSEL(0)								/* Shifter's pin start index */
	  | FLEXIO_SHIFTCTL_PINPOL*(0)								/* Shifter's pin active high */
	  | FLEXIO_SHIFTCTL_SMOD(2);								/* Shifter mode as transmit */

	/* Configure the timer for shift clock */
	p->TIMCMP[0] = 
		(((1 * 2) - 1) << 8)									/* TIMCMP[15:8] = number of beats x 2 “ 1 */
	  | ((_baud_div/2) - 1);									/* TIMCMP[7:0] = baud rate divider / 2 “ 1 */
	
	p->TIMCFG[0] = 
		FLEXIO_TIMCFG_TIMOUT(0)									/* Timer output logic one when enabled and not affected by reset */
	  | FLEXIO_TIMCFG_TIMDEC(0)									/* Timer decrement on FlexIO clock, shift clock equals timer output */
	  | FLEXIO_TIMCFG_TIMRST(0)									/* Timer never reset */
	  | FLEXIO_TIMCFG_TIMDIS(2)									/* Timer disabled on timer compare */
	  | FLEXIO_TIMCFG_TIMENA(2)									/* Timer enabled on trigger high */
	  | FLEXIO_TIMCFG_TSTOP(0)									/* Timer stop bit disabled */
	  | FLEXIO_TIMCFG_TSTART*(0);								/* Timer start bit disabled */
  
	p->TIMCTL[0] = 
		FLEXIO_TIMCTL_TRGSEL((((0) << 2) | 1))					/* Timer trigger selected as shifter's status flag */
	  | FLEXIO_TIMCTL_TRGPOL*(1)								/* Timer trigger polarity as active low */
	  | FLEXIO_TIMCTL_TRGSRC*(1)								/* Timer trigger source as internal */
	  | FLEXIO_TIMCTL_PINCFG(3)									/* Timer' pin configured as output */
	  | FLEXIO_TIMCTL_PINSEL(18)								/* Timer' pin index: WR pin */
	  | FLEXIO_TIMCTL_PINPOL*(1)								/* Timer' pin active low */
	  | FLEXIO_TIMCTL_TIMOD(1);									/* Timer mode as dual 8-bit counters baud/bit */

	/* Enable FlexIO */
	p->CTRL |= FLEXIO_CTRL_FLEXEN;

}
#include <USBHost_t36.h>
FASTRUN void NT35516_t41_p::sglBeatWR_nPrm_16 (uint32_t const cmd, uint16_t *value, const uint32_t length)
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
