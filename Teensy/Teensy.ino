/* 

Intended for compiling with:
https://github.com/MichaelMCE/TeensyFlexIoDisplay

Or alternatively roll your own:
Add your own display wrapper by implementing the below functions around your display driver
	void tft_init (void)
	void tft_clear (uint16_t)
	uint8_t *tft_getBuffer (void)
	void tft_update (void)

*/


#include <Arduino.h>
#include "common.h"


#define PACKET_SIZE			512	/* Dont change - bad things happen */
#define COLOUR_24TO16(c)	((uint16_t)(((((c)>>16)&0xF8)<<8) | ((((c)>>8)&0xFC)<<3) | (((c)&0xF8)>>3)))


/*
	VID:0x16C0
	PID:0x0486
	Inf:0
	Ep:0x04
	
	Use libusb inf-wizard to install a driver for the 'RawHid interface 0' device.
*/



// Synchronize this with libTeensyRawHid.h
typedef struct _header_t {
	uint32_t x1;
	uint32_t y1;
	uint32_t x2;
	uint32_t y2;
	uint32_t len;
	uint32_t crc;		// (x1+y1+x2+y2) ^ len
}header_t;






uint32_t decodeHeader (header_t *header, uint32_t *x1, uint32_t *y1, uint32_t *x2, uint32_t *y2, uint32_t *len)
{
	uint32_t crc = (header->x1 + header->y1 + header->x2 + header->y2) ^ header->len;
	if (crc != header->crc){
		//printf("header crc mismatch\r\n");
		//tft_clear(COLOUR_24TO16(0x0000FF));
		//tft_update();
		return 0;
	}
	
	if ((header->x2 <= header->x1) || (header->y2 <= header->y1)){
		//printf("header area invalid\r\n");
		//tft_clear(COLOUR_24TO16(0x00FF00));
		//tft_update();
		return 0;
	}

	if (((header->x2 - header->x1)+1) * ((header->y2 - header->y1)+1) * sizeof(uint16_t) != header->len){
		//printf("header size mismatch\r\n");
		//tft_clear(COLOUR_24TO16(0xFF0000));
		//tft_update();
		return 0;
	}
	
	*x1  = header->x1;
	*y1  = header->y1;
	*x2  = header->x2;
	*y2  = header->y2;
	*len = header->len;

	return *len;
}

void setup ()
{
	//Serial.begin(9600);
	//printf("RawHID 64 byte, 432x240*2");

	tft_init();
	tft_clear(0x0000);		// 16bit colour
	tft_update();
}

static uint8_t bufferIn[PACKET_SIZE];

void loop ()
{

	header_t *headerIn = (header_t*)bufferIn;
	headerIn->len = 0;
	headerIn->crc = 0;
	
	int bytesIn = RawHID.recv(headerIn, 10);
	if (bytesIn != PACKET_SIZE) return;

	header_t header;
	if (decodeHeader(headerIn, &header.x1, &header.y1, &header.x2, &header.y2, &header.len)){
		uint8_t *dbuffer = (uint8_t*)tft_getBuffer();
		
		int totalReads = (header.len / PACKET_SIZE);
		if (!totalReads) totalReads = 1;
		
		while (totalReads--){
			bytesIn = RawHID.recv(bufferIn, 5);
			if (bytesIn != PACKET_SIZE) break;
			
			memcpy(dbuffer, bufferIn, PACKET_SIZE);
			dbuffer += PACKET_SIZE;
		}

		int finalRead = (header.len%PACKET_SIZE);
		if (finalRead){
			bytesIn = RawHID.recv(bufferIn, 5);
			if (bytesIn > 0)
				memcpy(dbuffer, bufferIn, finalRead);
		}

		tft_update();
	}else{
		//printf("decode failed\r\n");
	}
	
}
