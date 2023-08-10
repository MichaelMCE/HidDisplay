#pragma once
#ifndef _TEENSY_H_
#define _TEENSY_H_


typedef struct _touchCtx {
	rawhid_header_t header;
	touch_t touch;
	
	uint8_t enabled;	// send reports. does not reflect current FT5216 comm state
	uint8_t pressed;	// is being pressed
	uint8_t rotate;		// touch rotation direction
	uint8_t unused;
	
	elapsedMillis t0;
}touchCtx_t;

#define TOUCH_REPORTS_HALT		0
#define TOUCH_REPORTS_OFF		1
#define TOUCH_REPORTS_ON		2


typedef struct _recvData {
	uint8_t *readIn;
	int inCt;
}recvDataCtx_t;




#endif

