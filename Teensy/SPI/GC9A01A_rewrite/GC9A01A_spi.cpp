

#if USE_SPI_GC9A01A2

#include "GC9A01A_SPI.h"

static GC9A01A_SPI STORAGETYPE lcd = GC9A01A_SPI(SPI_CS, SPI_DC, SPI_RST, SPI_MOSI, SPI_SCK);
#include "../common.h"




void GC9A01A_SPI::GC9A01_init (void)
{
    
    GC9A01_set_chip_select(1);
    GC9A01_delay(5);
    GC9A01_set_reset(0);
    GC9A01_delay(10);
    GC9A01_set_reset(1);
    GC9A01_delay(120);
    

    GC9A01_write_command(0xEF);
    
    GC9A01_write_command(0xEB);
    GC9A01_write_byte(0x14);
    
    GC9A01_write_command(0xFE);
    GC9A01_write_command(0xEF);
    
    GC9A01_write_command(0xEB);
    GC9A01_write_byte(0x14);
    
    GC9A01_write_command(0x84);
    GC9A01_write_byte(0x40);
    
    GC9A01_write_command(0x85);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x86);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x87);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x88);
    GC9A01_write_byte(0x0A);
    
    GC9A01_write_command(0x89);
    GC9A01_write_byte(0x21);
    
    GC9A01_write_command(0x8A);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x8B);
    GC9A01_write_byte(0x80);
    
    GC9A01_write_command(0x8C);
    GC9A01_write_byte(0x01);
    
    GC9A01_write_command(0x8D);
    GC9A01_write_byte(0x01);
    
    GC9A01_write_command(0x8E);
    GC9A01_write_byte(0xFF);
    
    GC9A01_write_command(0x8F);
    GC9A01_write_byte(0xFF);
    
    
    GC9A01_write_command(0xB6);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(GC9A01_MADCTL);
#if   (GC9A01_ROTATION == 0)
    GC9A01_write_byte(0x18);
#elif (GC9A01_ROTATION == 1)
    GC9A01_write_byte(0x28);
#elif (GC9A01_ROTATION == 2)
    GC9A01_write_byte(0x48);
#elif (GC9A01_ROTATION == 3)
    GC9A01_write_byte(0x88);
#endif
    
    GC9A01_write_command(GC9A01_COLMOD);
    GC9A01_write_byte(COLOUR_MODE_16_BIT);
    
    GC9A01_write_command(0x90);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    
    GC9A01_write_command(0xBD);
    GC9A01_write_byte(0x06);
    
    GC9A01_write_command(0xBC);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0xFF);
    GC9A01_write_byte(0x60);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0x04);
    
    GC9A01_write_command(0xC3);
    GC9A01_write_byte(0x13);
    GC9A01_write_command(0xC4);
    GC9A01_write_byte(0x13);
    
    GC9A01_write_command(0xC9);
    GC9A01_write_byte(0x22);
    
    GC9A01_write_command(0xBE);
    GC9A01_write_byte(0x11);
    
    GC9A01_write_command(0xE1);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x0E);
    
    GC9A01_write_command(0xDF);
    GC9A01_write_byte(0x21);
    GC9A01_write_byte(0x0c);
    GC9A01_write_byte(0x02);
    
    GC9A01_write_command(0xF0);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x26);
    GC9A01_write_byte(0x2A);
    
    GC9A01_write_command(0xF1);
    GC9A01_write_byte(0x43);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x72);
    GC9A01_write_byte(0x36);
    GC9A01_write_byte(0x37);
    GC9A01_write_byte(0x6F);
    
    GC9A01_write_command(0xF2);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x26);
    GC9A01_write_byte(0x2A);
    
    GC9A01_write_command(0xF3);
    GC9A01_write_byte(0x43);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x72);
    GC9A01_write_byte(0x36);
    GC9A01_write_byte(0x37);
    GC9A01_write_byte(0x6F);
    
    GC9A01_write_command(0xED);
    GC9A01_write_byte(0x1B);
    GC9A01_write_byte(0x0B);
    
    GC9A01_write_command(0xAE);
    GC9A01_write_byte(0x77);
    
    GC9A01_write_command(0xCD);
    GC9A01_write_byte(0x63);
    
    GC9A01_write_command(0x70);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x04);
    GC9A01_write_byte(0x0E);
    GC9A01_write_byte(0x0F);
    GC9A01_write_byte(0x09);
    GC9A01_write_byte(0x07);
    GC9A01_write_byte(0x08);
    GC9A01_write_byte(0x03);
    
    GC9A01_write_command(0xE8);
    GC9A01_write_byte(0x34);
    
    GC9A01_write_command(0x62);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x0D);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xED);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x0F);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xEF);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    
    GC9A01_write_command(0x63);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x11);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x18);
    GC9A01_write_byte(0x13);
    GC9A01_write_byte(0x71);
    GC9A01_write_byte(0xF3);
    GC9A01_write_byte(0x70);
    GC9A01_write_byte(0x70);
    
    GC9A01_write_command(0x64);
    GC9A01_write_byte(0x28);
    GC9A01_write_byte(0x29);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0xF1);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x07);
    
    GC9A01_write_command(0x66);
    GC9A01_write_byte(0x3C);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0xCD);
    GC9A01_write_byte(0x67);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x45);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x67);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x3C);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x01);
    GC9A01_write_byte(0x54);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x32);
    GC9A01_write_byte(0x98);
    
    GC9A01_write_command(0x74);
    GC9A01_write_byte(0x10);
    GC9A01_write_byte(0x85);
    GC9A01_write_byte(0x80);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x00);
    GC9A01_write_byte(0x4E);
    GC9A01_write_byte(0x00);
    
    GC9A01_write_command(0x98);
    GC9A01_write_byte(0x3e);
    GC9A01_write_byte(0x07);
    
    GC9A01_write_command(0x35);
    GC9A01_write_command(0x21);
    
    GC9A01_write_command(0x11);
    GC9A01_delay(120);
    GC9A01_write_command(0x29);
    GC9A01_delay(20);

}

void GC9A01A_SPI::GC9A01_set_address (const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2)
{
    uint8_t data[4];
    
    GC9A01_write_command(COL_ADDR_SET);
    data[0] = (x1 >> 8) & 0xFF;
    data[1] =  x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] =  x2 & 0xFF;
    GC9A01_write_data(data, sizeof(data));

    GC9A01_write_command(ROW_ADDR_SET);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] =  y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] =  y2 & 0xFF;
    GC9A01_write_data(data, sizeof(data));
}

void GC9A01A_SPI::GC9A01_set_frame (GC9A01_frame *frame)
{
    uint8_t data[4];
    
    GC9A01_write_command(COL_ADDR_SET);
    data[0] = (frame->start.X >> 8) & 0xFF;
    data[1] = frame->start.X & 0xFF;
    data[2] = (frame->end.X >> 8) & 0xFF;
    data[3] = frame->end.X & 0xFF;
    GC9A01_write_data(data, sizeof(data));

    GC9A01_write_command(ROW_ADDR_SET);
    data[0] = (frame->start.Y >> 8) & 0xFF;
    data[1] = frame->start.Y & 0xFF;
    data[2] = (frame->end.Y >> 8) & 0xFF;
    data[3] = frame->end.Y & 0xFF;
    GC9A01_write_data(data, sizeof(data));
}
/*
// 18/24bit
void GC9A01A_SPI::spiAdd3Start (const uint8_t a, const uint8_t b, const uint8_t c)
{
	spiPos = 0;
	spiBuffer[spiPos++] = a;
	spiBuffer[spiPos++] = b;
	spiBuffer[spiPos++] = c;

	SPI_BEGIN_TRANSACTION();
	GC9A01_write(spiBuffer, spiPos);
}

// 18/24bit
void GC9A01A_SPI::spiAdd3 (const uint8_t a, const uint8_t b, const uint8_t c)
{
	spiBuffer[spiPos++] = a;
	spiBuffer[spiPos++] = b;
	spiBuffer[spiPos++] = c;
	
	if (spiPos >= SPI_BUFFER_LEN){
		GC9A01_write_continue(spiBuffer, spiPos);
		spiPos = 0;
	}
}

// 16bit
void GC9A01A_SPI::spiAdd16Start (const uint16_t a)
{
	spiPos = 0;
	spiBuffer[spiPos++] = (a>>8)&0xFF;
	spiBuffer[spiPos++] = a&0xFF;
	
	SPI_BEGIN_TRANSACTION();
	GC9A01_write(spiBuffer, spiPos);
}*/

// 16bit
void GC9A01A_SPI::spiAdd16 (const uint16_t a)
{
	uint16_t *pixels = (uint16_t*)&spiBuffer[(++spiPos)++];
	*pixels = a;

	if (!spiBeginWrite){
		spiBeginWrite = 0xFF;
		GC9A01_write(spiBuffer, spiPos);
		
	}else if (spiPos >= SPI_BUFFER_LEN){
		GC9A01_write_continue(spiBuffer, spiPos);
		spiPos = 0;
	}
}

// full frame 16bit update
void GC9A01A_SPI::GC9A01_update (const uint16_t *pixels)
{
	GC9A01_set_address(0, 0, GC9A01_WIDTH-1, GC9A01_HEIGHT-1);
	
	spiAddStart();

	int tPixels = (GC9A01_WIDTH * GC9A01_HEIGHT);
	while (tPixels--)
		spiAdd16(*pixels++);

	spiAddEnd();
}

void GC9A01A_SPI::GC9A01_updateArea (const uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	GC9A01_set_address(x1, y1, x2, y2);
	
	spiAddStart();
	
	const int w = (x2 - x1) + 1;
	const int h = (y2 - y1) + 1;
	int tPixels = (w * h);

	if (tPixels < 1)
		return;
	else if (tPixels > TFT_WIDTH * TFT_HEIGHT)
		tPixels = TFT_WIDTH * TFT_HEIGHT;

	while (tPixels--)
		spiAdd16(*pixels++);

	spiAddEnd();
}

void GC9A01A_SPI::pushPixels16bit (const uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	GC9A01_updateArea(pixels, x1, y1, x2, y2);
}

void GC9A01A_SPI::setBacklight (const uint8_t value)
{
	analogWrite(_bl, value&0x7F);
}

void GC9A01A_SPI::setRotation (const uint8_t value)
{
    GC9A01_write_command(GC9A01_MADCTL);
    
    switch (value){
    case 0:
		GC9A01_write_byte(0x10);
		break;
	case 1:
    	GC9A01_write_byte(0x28);
    	break;
	case 2:
    	GC9A01_write_byte(0x40);
    	break;
	default:
    	GC9A01_write_byte(0x80);
	};
}

void GC9A01A_SPI::GC9A01A_start ()
{
    pinMode(_rst, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    pinMode(_bl, OUTPUT);

 	setBacklight(TFT_INTENSITY);
	
    spi->begin();
    spi->setCS(_cs);
    spi->setMOSI(_mosi);
    spi->setSCK(_sclk);
    spi->setClockDivider(1);
    spi->setDataMode(SPI_MODE0);
	
    GC9A01_init();
    GC9A01_set_address(0, 0, GC9A01_WIDTH-1, GC9A01_HEIGHT-1);
    setRotation(GC9A01_ROTATION);    
}

void GC9A01A_SPI::begin (uint32_t spi_clock, uint32_t spi_clock_read)
{
	spi_clock *= 1000 * 1000;
	
	_SPI_CLOCK = spi_clock;
	_SPI_CLOCK_READ = spi_clock_read;
	
	GC9A01A_start();
}

GC9A01A_SPI::GC9A01A_SPI (uint8_t cs, uint8_t dc, uint8_t rst, uint8_t mosi, uint8_t sclk)
{
  _cs = cs;
  _dc = dc;
  _rst = rst;
  _mosi = mosi;
  _sclk = sclk;
  _bl = TFT_BL;
}


#endif

