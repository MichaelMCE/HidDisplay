#include <Wire.h>

#define FT5216_ADDR 0x38


void setup ()
{
  Serial.begin(115200);
  while (!Serial);
  delay(100);
  
  Wire2.begin();
  Serial.println("Capacitive touchscreen started");
}

void loop ()
{
	
  Wire2.beginTransmission(FT5216_ADDR);
  Wire2.write(0x02); // Set point to TD_STATUS 
  
  if (Wire2.endTransmission(false) != 0) {
    Serial.println("Write error !");
    delay(1000);
    return;
  }
  
  uint8_t count = Wire2.requestFrom(FT5216_ADDR, 5, true);
  if (Wire2.available() <= 0) {
    Serial.println(String("Read error !, ") + Wire2.available());
    delay(1000);
    return;
  }

  // Process Data
  uint8_t TD_STATUS = Wire2.read();
  uint8_t TOUCH1_XH = Wire2.read();
  uint8_t TOUCH1_XL = Wire2.read();
  uint8_t TOUCH1_YH = Wire2.read();
  uint8_t TOUCH1_YL = Wire2.read();

  if ((TD_STATUS&0x0F) > 0) {
    uint16_t cx = (((uint16_t)TOUCH1_XH&0x0F)<<8)|TOUCH1_XL;
    uint16_t cy = (((uint16_t)TOUCH1_YH&0x0F)<<8)|TOUCH1_YL;
    Serial.println(String("Touch: ") + cx + ", " + cy);
  }
  delay(50);
}
