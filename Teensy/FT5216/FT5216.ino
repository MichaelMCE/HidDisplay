
/*
	FT5216 i2c controller
	Michael McE
*/


#include <Wire.h>
#include "FT5216.h"

#define FT5216_ADDR		0x38	// i2c address
#define FT5216_INT		33		// using pin 33 for interrupt
	

typedef struct _touch {
	int idx;
	int points;		// number of points (fingers) measured on panel 
	int x;
	int y;
	
	uint8_t xh;
	uint8_t xl;
	uint8_t yh;
	uint8_t yl;
}touch_t;

static volatile int touchInSignal = 0;
static const uint8_t FT5216_RegAddrLUT[10] = {0x03, 0x09, 0x0F, 0x15, 0x1B, 0x21, 0x27, 0x2D, 0x33, 0x39};




void FT5216_start ()
{
	Wire2.beginTransmission(FT5216_ADDR);
}

int FT5216_end ()
{
	return Wire2.endTransmission(false);
}

int FT5216_write (const uint8_t value)
{
	return Wire2.write(value);
}

uint8_t FT5216_readByte ()
{
	return Wire2.read();
}

int FT5216_writeReg (const uint8_t reg, const uint8_t value)
{
	uint8_t data[] = {reg, value};
	return Wire2.write(data, sizeof(data));
}

int FT5216_isAvailable ()
{
	return Wire2.available();
}

void FT5216_begin ()
{
	Wire2.begin();
	Wire2.setClock(200000);
	
	// Delay to enter 'Monitor' status (s)
    FT5216_writeReg(FT5216_REG_TIMEENTERMONITOR, 0x0A);

    // Period of 'Active' status (ms)
    FT5216_writeReg(FT5216_REG_PERIODACTIVE, 0x06);

    // Timer to enter 'idle' when in 'Monitor' (ms)
    FT5216_writeReg(FT5216_REG_PERIODMONITOR, 0x28);
	
}

uint8_t FT5216_request (const uint8_t length)
{
	return Wire2.requestFrom((uint8_t)FT5216_ADDR, (uint8_t)length, (bool)true);
}	

void touch_ISR ()
{
	touchInSignal++;

	FT5216_start();
	FT5216_write(FT5216_TOUCH_POINTS); // Set point to TD_STATUS 
	if (FT5216_end() != 0)
		printf("Wire2 write error\r\n");
}

int touch_getTotal (touch_t *touch)
{
	uint8_t count = FT5216_request(1);
	if (count){
		touch->points = FT5216_readByte()&0x0F;
		if (touch->points > (int)sizeof(FT5216_RegAddrLUT))
			touch->points = sizeof(FT5216_RegAddrLUT);
		return touch->points;
	}else{
		return 0;
	}
}

int touch_read (touch_t *touch)
{
	if (touch->idx >= touch->points)
		return 0;
	
	FT5216_start();
	FT5216_write(FT5216_RegAddrLUT[touch->idx++]);
	if (FT5216_end() != 0){
		printf("Wire2 write error\r\n");
		return 0;
	}
			
	uint8_t count = FT5216_request(4);
	if (!count) return 0;

	int avail = FT5216_isAvailable();
	if (avail <= 0 || avail != count){
		printf("Wire2 read error: %i, %i\r\n", avail, count);
		return 0;
	}
	
	touch->xh = FT5216_readByte();
	touch->xl = FT5216_readByte();
	touch->yh = FT5216_readByte();
	touch->yl = FT5216_readByte();
	touch->x = (((uint16_t)touch->xh&0x0F)<<8) | touch->xl;
   	touch->y = (((uint16_t)touch->yh&0x0F)<<8) | touch->yl;
   	return 1;
}

void touch_begin (const int intPin, void(*cb)())
{
	pinMode(intPin, INPUT_PULLDOWN);
	attachInterrupt(intPin, cb, RISING);
	FT5216_begin();
}

void setup ()
{
	Serial.begin(115200);
	while (!Serial);

	printf("FT5216 Capacitive touchscreen test\r\n");

	touch_begin(FT5216_INT, touch_ISR);

	delay(100);
}

void loop ()
{
	if (!touchInSignal){
		return;
	}else{
		touchInSignal = 0;
	}

	touch_t touch = {0};
	int tPoints = touch_getTotal(&touch);
	if (tPoints){
		printf("\r\n");
		
		for (int i = 0; i < tPoints; i++){
			if (touch_read(&touch))
				printf("Touch %i: %i %i\r\n", i+1, touch.x, touch.y);
		}
	}
}
