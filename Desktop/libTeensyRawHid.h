
#ifndef _LIBTeensyRawHid_H_
#define _LIBTeensyRawHid_H_



#include "lusb0_usb.h"



#define TEENSYRAWHID_VID				0x16C0
#define TEENSYRAWHID_PID				0x0486
#define TEENSYRAWHID_INTERFACE			0
#define TEENSYRAWHID_ENDPOINT			0x04
#define TEENSYRAWHID_PACKETSIZE			512     // select 64 for standard RawHid, or 512 for RawHid512

#if 1
#define TEENSYRAWHID_WIDTH				432
#define TEENSYRAWHID_HEIGHT				240
#else
#define TEENSYRAWHID_WIDTH				480
#define TEENSYRAWHID_HEIGHT				320
#endif

#define LIBUSB_VENDERID					TEENSYRAWHID_VID
#define LIBUSB_PRODUCTID				TEENSYRAWHID_PID
#define LIBUSB_ENDPOINT					TEENSYRAWHID_ENDPOINT



// Synchronize this with Teensy.ino
typedef struct _header_t {
	uint32_t x1;
	uint32_t y1;
	uint32_t x2;
	uint32_t y2;
	uint32_t len;
	uint32_t crc;		// (x1+y1+x2+y2) ^ len
}header_t;



typedef struct _rgb16_t {
	uint16_t b:5;
	uint16_t g:6;
	uint16_t r:5;
}rgb16_t;

typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}__attribute__ ((packed))TRGBA;		// 8888

typedef struct _TeensyRawHidcxt_t{
	uint32_t Width;
    uint32_t Height;
    uint32_t PixelFormat;
	uint32_t DeviceVersion;
	char path[512+2];
	uint32_t displayOp;
	
	struct usb_device *usbdev; 
	usb_dev_handle *usb_handle;
	uint32_t interface;
	
	uint8_t *frame;
	uint8_t *frameAlt;
	HANDLE hDev;
}teensyRawHidcxt_t;


typedef struct _razersb_t{
	teensyRawHidcxt_t  teensyRawHid;
}teensyRawHid_t;




int libTeensyRawHid_Open (teensyRawHidcxt_t *ctx, const uint32_t interface, uint32_t index);
int libTeensyRawHid_Close (teensyRawHidcxt_t *ctx);
int libTeensyRawHid_OpenDisplay (teensyRawHidcxt_t *ctx, uint32_t index);
int libTeensyRawHid_CloseDisplay (teensyRawHidcxt_t *ctx);
int libTeensyRawHid_WriteImage (teensyRawHidcxt_t *ctx, header_t *idesc, void *pixelData);

// legacy
int libTeensyRawHid_GetDisplayTotal ();


#endif

