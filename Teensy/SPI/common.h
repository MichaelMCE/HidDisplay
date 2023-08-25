
#ifndef _COMMONINTF_H_
#define _COMMONINTF_H_

// this file is an interface between the core C code and CPP display code



void display_begin (const uint32_t speed)
{
	lcd.begin(speed);
}

// x1..y2 = the distination region these pixels represent
void display_update (const uint16_t *pixels, const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2)
{
	lcd.pushPixels16bit(pixels, x1, y1, x2, y2);
}

void display_setRotation (const uint8_t rotation)
{
	lcd.setRotation(rotation);
}

void display_setBacklight (const uint8_t level)
{
	lcd.setBacklight(level);
}

#endif
