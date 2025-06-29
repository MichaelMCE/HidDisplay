
#ifndef _GC9A01A2_SPI_H_
#define _GC9A01A2_SPI_H_

#if USE_SPI_GC9A01A2

#include <Arduino.h>
#include <SPI.h>


#define GC9A01_WIDTH		TFT_WIDTH
#define GC9A01_HEIGHT		TFT_HEIGHT


// Set display rotation		0 to 3
#define GC9A01_ROTATION		1


#define COL_ADDR_SET        0x2A
#define ROW_ADDR_SET        0x2B
#define MEM_WR              0x2C
#define MEM_WR_CONT         0x3C
#define GC9A01_MADCTL		0x36
#define GC9A01_COLMOD       0x3A

#define COLOUR_MODE_12_BIT  0x03
#define COLOUR_MODE_16_BIT  0x05
#define COLOUR_MODE_18_BIT  0x06

#define RGB888To16(r,g,b) ((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | (((b)&0xF8)>>3))
#define SPI_BEGIN_TRANSACTION() spi->beginTransaction(SPISettings(_SPI_CLOCK, MSBFIRST, SPI_MODE0))
#define SPI_END_TRANSACTION() spi->endTransaction()


#ifdef __cplusplus
extern "C" {
#endif


#define SPI_BUFFER_LEN	(2048)


typedef struct _point {
    uint16_t X;
    uint16_t Y;
}GC9A01_point;

typedef struct _frame {
    GC9A01_point start;
    GC9A01_point end;
}GC9A01_frame;




class GC9A01A_SPI {
public:
	GC9A01A_SPI (uint8_t _CS, uint8_t _DC, uint8_t _RST = 255, uint8_t _MOSI = 11, uint8_t _SCLK = 13);

	void begin (uint32_t spi_clock = TFT_SPEED, uint32_t spi_clock_read = TFT_SPEED);
	void setBacklight (const uint8_t value);
	void setRotation (const uint8_t value);
	void pushPixels16bit (const uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

	uint8_t _rst, _bl;
	uint8_t _cs, _dc;
	uint8_t _mosi, _sclk;

	SPIClass *spi = &SPI1;

	uint32_t _SPI_CLOCK;
	uint32_t _SPI_CLOCK_READ;
	
private:

	int spiBeginWrite = 0;
	int spiPos = 0;
	uint8_t spiBuffer[SPI_BUFFER_LEN+4];
	

	void GC9A01A_start ();
	void GC9A01_init (void);
	
	void GC9A01_set_address (const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2);
	void GC9A01_set_frame (GC9A01_frame *frame);

	//void spiAdd16Start (const uint16_t a);
	void spiAdd16 (const uint16_t a);
	//void spiAdd3Start (const uint8_t a, const uint8_t b, const uint8_t c);
	//void spiAdd3 (const uint8_t a, const uint8_t b, const uint8_t c);

	void GC9A01_updateArea (const uint16_t *pixels, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
	void GC9A01_update (const uint16_t *pixels);
	
	
	inline void setPixel16 (uint16_t *pixels, uint16_t x, uint16_t y, uint16_t colour)
	{
		pixels[(y*GC9A01_WIDTH) + x] = colour;
	}

	inline void GC9A01_write_byte (uint8_t val)
	{
	    GC9A01_write_data(&val, sizeof(val));
	}
	
	inline void GC9A01_spi_tx (uint8_t *data, size_t len)
	{
		spi->transfer(data, len);
	}

	inline void GC9A01_set_data_command (uint8_t val)
	{
	    digitalWriteFast(_dc, val);
	}

	inline void GC9A01_set_reset (uint8_t val)
	{
	    digitalWriteFast(_rst, val);
	}

	inline void GC9A01_set_chip_select (uint8_t val)
	{
	    digitalWriteFast(_cs, val);
	}

	inline void GC9A01_delay (uint16_t ms)
	{
    	delay(ms);
	}

	inline void GC9A01_write_command (uint8_t cmd)
	{
	    GC9A01_set_data_command(0);
	    GC9A01_set_chip_select(0);
	    GC9A01_spi_tx(&cmd, sizeof(cmd));
	    GC9A01_set_chip_select(1);
	}

	inline void GC9A01_write_data (uint8_t *data, size_t len)
	{
	    GC9A01_set_data_command(1);
	    GC9A01_set_chip_select(0);
	    GC9A01_spi_tx(data, len);
	    GC9A01_set_chip_select(1);
	}

	inline void GC9A01_write (uint8_t *data, size_t len)
	{
	    GC9A01_write_command(MEM_WR);
	    GC9A01_write_data(data, len);
	}

	inline void GC9A01_write_continue (uint8_t *data, size_t len)
	{
	    GC9A01_write_command(MEM_WR_CONT);
	    GC9A01_write_data(data, len);
	}

	inline void spiAddStart ()
	{
		spiPos = 0;
		spiBeginWrite = 0;
		SPI_BEGIN_TRANSACTION();
	}

	inline void spiAddEnd ()
	{
		if (spiPos){
			GC9A01_write_continue(spiBuffer, spiPos);
			spiPos = 0;
		}
		SPI_END_TRANSACTION();
	}
};



#ifdef __cplusplus
}
#endif

#endif
#endif
