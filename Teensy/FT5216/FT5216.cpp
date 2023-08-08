
/*
	FT5216 i2c controller
	Michael McE
	
	Does not handle press release. That's up to you.
	06/08/23
*/


#include <Wire.h>
#include "../librawhiddesc.h"
#include "FT5216.h"

#define FT5216_obj		Wire2	// using i2c pins 24 & 25 on T4.1
#define FT5216_ADDR		0x38	// i2c address
#define FT5216_INT		34		// using pin 33 for touch ready interrupt signal
#define FT5216_SPEED	1000000	// set speed in hz




static volatile int touchInSignal = 0;
static const uint8_t FT5216_RegAddrLUT[10] = {0x03, 0x09, 0x0F, 0x15, 0x1B, 0x21, 0x27, 0x2D, 0x33, 0x39};




void FT5216_start ()
{
	FT5216_obj.beginTransmission(FT5216_ADDR);
}

int FT5216_end ()
{
	return FT5216_obj.endTransmission(false);
}

int FT5216_write (const uint8_t value)
{
	return FT5216_obj.write(value);
}

uint8_t FT5216_readByte ()
{
	return FT5216_obj.read();
}

int FT5216_writeReg (const uint8_t reg, const uint8_t value)
{
	uint8_t data[] = {reg, value};
	return FT5216_obj.write(data, sizeof(data));
}

int FT5216_isAvailable ()
{
	return FT5216_obj.available();
}

void FT5216_begin ()
{
	FT5216_obj.begin();
	FT5216_obj.setClock(FT5216_SPEED);
	
	FT5216_writeReg(FT5216_DEVICE_MODE, 0);
	
	// Delay to enter 'Monitor' status (s)
    FT5216_writeReg(FT5216_REG_TIMEENTERMONITOR, 0x0A);

    // Period of 'Active' status (ms)
    FT5216_writeReg(FT5216_REG_PERIODACTIVE, 0x06);

    // Timer to enter 'idle' when in 'Monitor' (ms)
    FT5216_writeReg(FT5216_REG_PERIODMONITOR, 0x28);
}

uint8_t FT5216_request (const uint8_t length)
{
	return FT5216_obj.requestFrom((uint8_t)FT5216_ADDR, (uint8_t)length, (bool)true);
}	

int touch_getTotal (touch_t *touch)
{
	uint8_t count = FT5216_request(1);
	if (count){
		touch->tPoints = FT5216_readByte()&0x0F;
		//if (touch->tPoints > (int)sizeof(FT5216_RegAddrLUT))
		//	touch->tPoints = sizeof(FT5216_RegAddrLUT);
		if (touch->tPoints > 5)
			touch->tPoints = 5;
		return touch->tPoints;
	}else{
		return 0;
	}
}

int touch_read (touch_t *touch)
{
	if (touch->idx >= touch->tPoints)
		return 0;
	
	FT5216_start();
	FT5216_write(FT5216_RegAddrLUT[touch->idx]);
	if (FT5216_end() != 0){
		printf("i2c write error\r\n");
		return 0;
	}
			
	uint8_t count = FT5216_request(4);
	if (!count) return 0;

	int avail = FT5216_isAvailable();
	if (avail <= 0 || avail != count)
		return 0;

	touch->xh = FT5216_readByte();
	touch->xl = FT5216_readByte();
	touch->yh = FT5216_readByte();
	touch->yl = FT5216_readByte();
	touch->x  = (((uint16_t)touch->xh&0x0F)<<8) | touch->xl;
   	touch->y  = (((uint16_t)touch->yh&0x0F)<<8) | touch->yl;

	if (touch->x == 0xFFF || touch->y == 0xFFF){
		touch->idx = 0;
		touch->tPoints = 0;
		return 0;
	}
   	touch->points[touch->idx].x = touch->x;
   	touch->points[touch->idx].y = touch->y;

   	return ++touch->idx;
}
#if 0
static void (*callb)();
static int interPin;

void reattach ()
{
	attachInterrupt(interPin, callb, FALLING);
}

void dettach ()
{
	detachInterrupt(interPin);
}
#endif
void touch_begin (const int intPin, void(*cb)())
{
	//interPin = intPin;
	//callb = cb;
	
	pinMode(intPin, INPUT_PULLUP);
	attachInterrupt(intPin, cb, FALLING);
	FT5216_begin();
}

void touch_ISR ()
{
	touchInSignal = 0xFF;
}

void touch_start (const int intPin)
{
	touch_begin(intPin, touch_ISR);
}

int touch_process (touch_t *touch)
{
	if (!touchInSignal) return 0;

	FT5216_start();
	FT5216_write(FT5216_TOUCH_POINTS);
	if (FT5216_end() != 0)
		printf("touch_pts Wire2 write error\r\n");

	if (touch_getTotal(touch)){
		for (int i = 0; i < touch->tPoints; i++){
			if (!touch_read(touch)) break;
		}
	}
	touchInSignal = 0;

	return touch->tPoints;
}

int touch_isPressed ()
{
	return touchInSignal;
}

#if 0
void setup ()
{
	Serial.begin(115200);
	while (!Serial);

	printf("FT5216 Capacitive touchscreen (i2c)\r\n");
	touch_begin(FT5216_INT, touch_ISR);
}

void loop ()
{
	if (touchInSignal){
		touchInSignal = 0;

		FT5216_start();
		FT5216_write(FT5216_TOUCH_POINTS); // Set point to TD_STATUS 
		if (FT5216_end() != 0)
			printf("Wire2 write error\r\n");
		
		touch_t touch = {0};
		touch_process(&touch);
	}
	//delay(8);
}
#endif
