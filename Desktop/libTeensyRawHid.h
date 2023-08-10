
#ifndef _LIBTEENSYRAWHID_H_
#define _LIBTEENSYRAWHID_H_



#include "lusb0_usb.h"
#include "librawhiddesc.h"

#define USE_WRITE_ASYNC			1

#define RAWHID_VID				0x16C0
#define RAWHID_PID				0x0486
#define RAWHID_INTERFACE		0
#define RAWHID_EP_WRITE			0x04
#define RAWHID_EP_READ			0x83


#define LIBUSB_VENDERID			RAWHID_VID
#define LIBUSB_PRODUCTID		RAWHID_PID
#define LIBUSB_ENDPOINT_WRITE	RAWHID_EP_WRITE
#define LIBUSB_ENDPOINT_READ	RAWHID_EP_READ




typedef struct _rgb16_t{
	uint16_t b:5;
	uint16_t g:6;
	uint16_t r:5;
}__attribute__ ((packed))rgb16_t;

typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
}__attribute__ ((packed))TRGBA;		// 8888

typedef struct _TeensyRawHidcxt_t{
	uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t PixelFormat;
	uint32_t DeviceVersion;

	uint8_t rgbClamp;
	uint8_t unused[3];

	char path[512+2];

	struct usb_device *usbdev; 
	usb_dev_handle *usb_handle;
	uint32_t interface;
	uint32_t wMaxPacketSize;
	
	uint8_t *frame;
	HANDLE hDev;
	
	uint8_t *buffer;
	
	struct {
		void *cxt;
		int state;
	}async;
}teensyRawHidcxt_t;




int libTeensyRawHid_Open (teensyRawHidcxt_t *ctx, const uint32_t interface, uint32_t deviceIndex);
int libTeensyRawHid_Close (teensyRawHidcxt_t *ctx);

int libTeensyRawHid_OpenDisplay (teensyRawHidcxt_t *ctx);
int libTeensyRawHid_CloseDisplay (teensyRawHidcxt_t *ctx);

int libTeensyRawHid_GetConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc);
int libTeensyRawHid_WriteImage (teensyRawHidcxt_t *ctx, void *pixelData);
int libTeensyRawHid_WriteImageEx (teensyRawHidcxt_t *ctx, uint16_t *pixelData, const uint8_t maxComponentValue);
int libTeensyRawHid_WriteArea (teensyRawHidcxt_t *ctx, void *pixelData, const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2);

int libTeensyRawHid_WriteTiles (teensyRawHidcxt_t *ctx, void *pixelData, const uint32_t size, const uint16_t x, const uint16_t y, const uint16_t tTiles);
int libTeensyRawHid_SetTileConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc);

int libTeensyRawHid_GetDisplayTotal ();

int libTeensyRawHid_TouchReportEnable (teensyRawHidcxt_t *ctx, const int state, const int applyRotation);
int libTeensyRawHid_GetReportWait (teensyRawHidcxt_t *ctx, touch_t *touch);

#endif

