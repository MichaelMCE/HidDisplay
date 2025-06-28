
#ifndef _libHidDisplay_H_
#define _libHidDisplay_H_



#include "lusb0_usb.h"
#include "libHidDisplayDesc.h"


#define RAWHID_VID				0x16C0
#define RAWHID_PID				0x0486
#define RAWHID_INTERFACE		0
#define RAWHID_EP_WRITE			0x04
#define RAWHID_EP_READ			0x83


#define LIBUSB_VENDERID			RAWHID_VID
#define LIBUSB_PRODUCTID		RAWHID_PID
#define LIBUSB_ENDPOINT_WRITE	RAWHID_EP_WRITE
#define LIBUSB_ENDPOINT_READ	RAWHID_EP_READ

#define USE_WRITE_ASYNC			1



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

typedef struct _trd {
	void *sourceBuffer;
	volatile int exiting;
	HANDLE hRenderThread;
	HANDLE hUpdateEvent;
	
	uint32_t state;
	uint64_t fcount;
}winthrd_t;

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
		int threadState;
		winthrd_t thrd;
	}async;
}teensyRawHidcxt_t;


int libHidDisplay_OpenDisplay (teensyRawHidcxt_t *ctx, const int displayIdx);
int libHidDisplay_CloseDisplay (teensyRawHidcxt_t *ctx);

int libHidDisplay_Open (teensyRawHidcxt_t *ctx, const uint32_t interface, uint32_t deviceIndex);
int libHidDisplay_Close (teensyRawHidcxt_t *ctx);

int libHidDisplay_GetConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc);
int libHidDisplay_WriteImage (teensyRawHidcxt_t *ctx, void *pixelData);
int libHidDisplay_WriteImageEx (teensyRawHidcxt_t *ctx, uint16_t *pixelData, const uint8_t maxComponentValue);
int libHidDisplay_WriteImageAsync (teensyRawHidcxt_t *ctx, void *buffer);
int libHidDisplay_WriteArea (teensyRawHidcxt_t *ctx, void *pixelData, const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2);

int libHidDisplay_WriteTiles (teensyRawHidcxt_t *ctx, void *pixelData, const uint32_t size, const uint16_t x, const uint16_t y, const uint16_t tTiles);
int libHidDisplay_SetTileConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc);

int libHidDisplay_GetDisplayTotal ();

int libHidDisplay_TouchReportEnable (teensyRawHidcxt_t *ctx, const int state, const int applyRotation);

// returns total number of points (fingers pressed)
// returns 0 on error or timeout. timeout is @ 400ms
// returns -1 on release (all fingers up)
int libHidDisplay_GetReportWait (teensyRawHidcxt_t *ctx, touch_t *touch);
int libHidDisplay_GetReportWaitEx (teensyRawHidcxt_t *ctx, int *reportType, void *report);

// is also called automatically at libHidDisplay_Close[Display]()
void libHidDisplay_WriteImageAsyncStop (teensyRawHidcxt_t *ctx);


// work in progress
int libHidDisplay_DrawOpsCommit (teensyRawHidcxt_t *ctx, void *drawOps, const int drawOpsLen, const int totalOps, const uint32_t flags, const int refId);
int libHidDisplay_DrawOpsExecute (teensyRawHidcxt_t *ctx, const int refId);
int libHidDisplay_DrawClutCommit (teensyRawHidcxt_t *ctx, void *colTable, const int colTotal, const size_t colTableSize, const uint16_t refId);


// intiate device reboot
int libHidDisplay_Reset (teensyRawHidcxt_t *ctx);

#endif

