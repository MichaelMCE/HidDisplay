
/*
	FT5216 i2c controller
	Michael McE
	
	Does not handle press release. That's up to you.
	06/08/23
*/


#include <Wire.h>
#include "../libHidDisplayDesc.h"
#include "../config.h"
#include "FT5216.h"

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
	
    FT5216_writeReg(FT5216_REG_THGROUP, 0x16);

    // valid touching peak detect threshold
    FT5216_writeReg(FT5216_REG_THPEAK, 0x3C);

    // Touch focus threshold
    FT5216_writeReg(FT5216_REG_THCAL, 0xE9);

    // threshold when there is surface water
    FT5216_writeReg(FT5216_REG_THWATER, 0x01);

    // threshold of temperature compensation
    FT5216_writeReg(FT5216_REG_THTEMP, 0x01);

    // Touch difference threshold
    FT5216_writeReg(FT5216_REG_THDIFF, 0xA0);
	
	
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

static void FT5216_applyRotation (const int direction, uint16_t *x, uint16_t *y)
{
    uint16_t _x = *x;
    uint16_t _y = *y;

    switch (direction){
    case TOUCH_DIR_LRTB:
        *x = _x;
        *y = _y;
        break;
    case TOUCH_DIR_LRBT:
        *x = _x;
        *y = TFT_HEIGHT-1 - _y;
        break;
    case TOUCH_DIR_RLTB:
        *x = TFT_WIDTH-1  - _x;
        *y = _y;
        break;
    case TOUCH_DIR_RLBT:
        *x = TFT_WIDTH-1  - _x;
        *y = TFT_HEIGHT-1 - _y;
        break;
    case TOUCH_DIR_TBLR:
        *x = _y;
        *y = _x;
        break;
    case TOUCH_DIR_BTLR:
        *x = _y;
        *y = TFT_WIDTH-1  - _x;
        break;
    case TOUCH_DIR_TBRL:
        *x = TFT_HEIGHT-1 - _y;
        *y = _x;
        break;
    case TOUCH_DIR_BTRL:
        *x = TFT_HEIGHT-1 - _y;
        *y = TFT_WIDTH-1  - _x;
        break;

	case TOUCH_DIR_SWAP_A_INVERT_V:
		*x = _y;
		*y = TFT_HEIGHT-1 - _x;
		 break;

	case TOUCH_DIR_SWAP_A_INVERT_H:
		*y = _x;
		*x = TFT_WIDTH-1 - _y;
		 break;
		 
    default:
        break;
    }
}

int touch_getTotal (touch_t *touch)
{
	uint8_t count = FT5216_request(1);
	if (count){
		touch->tPoints = FT5216_readByte()&0x0F;
		if (touch->tPoints > (int)sizeof(FT5216_RegAddrLUT))
			touch->tPoints = sizeof(FT5216_RegAddrLUT);
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
	
	if (touch->direction != TOUCH_DIR_NONE)
		FT5216_applyRotation(touch->direction, &touch->x, &touch->y);

   	touch->points[touch->idx].x = touch->x;
   	touch->points[touch->idx].y = touch->y;

   	return ++touch->idx;
}

void touch_begin (const int intPin, void(*cb)())
{
	pinMode(intPin, INPUT_PULLUP);
	attachInterrupt(intPin, cb, FALLING);
	FT5216_begin();
}

void touch_ISR ()
{
	touchInSignal++;
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
