

#include <Wire.h>

#define FT5216_ADDR 0x38
#define FT5216_INT	33


typedef struct _touch {
	int points;		// number of points (fingers) measured on panel
	int x;
	int y;
	
	uint8_t xh;
	uint8_t xl;
	uint8_t yh;
	uint8_t yl;
}touch_t;



static volatile int touchInSignal = 0;

void touchISR ()
{
	touchInSignal++;

	Wire2.beginTransmission(FT5216_ADDR);
	Wire2.write(0x02); // Set point to TD_STATUS 
	if (Wire2.endTransmission(false) != 0)
		printf("Wire2 write error\r\n");
}

void setup ()
{
	Serial.begin(115200);
	while (!Serial);

	printf("Capacitive touchscreen started\r\n");

	pinMode(FT5216_INT, INPUT_PULLDOWN);
	attachInterrupt(FT5216_INT, touchISR, RISING);
	  
	Wire2.begin();
	Wire2.setClock(200000);
	delay(100);
}

int getTouch (touch_t *touch)
{
	uint8_t count = Wire2.requestFrom((uint8_t)FT5216_ADDR, (uint8_t)5, (bool)true);
	if (!count) return 0;

	int avail = Wire2.available();
	if (avail <= 0 || avail != count){
		printf("Wire2 read error: %i, %i, %i\r\n", Wire2.available(), avail, count);
		return 0;
	}
	
	touch->points = Wire2.read();
	touch->xh = Wire2.read();
	touch->xl = Wire2.read();
	touch->yh = Wire2.read();
	touch->yl = Wire2.read();

	if ((touch->points&0x0F) > 0){
		touch->x = (((uint16_t)touch->xh&0x0F)<<8) | touch->xl;
    	touch->y = (((uint16_t)touch->yh&0x0F)<<8) | touch->yl;
    	return 1;
	}
	return 0;
}

void loop ()
{
	if (!touchInSignal){
		return;
	}else{
		touchInSignal = 0;
	}


	touch_t touch;
	if (getTouch(&touch))
		printf("Touch: %i, %i %i\r\n", touch.points, touch.x, touch.y);

}
