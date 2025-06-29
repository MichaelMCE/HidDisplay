

#include <Arduino.h>
#include "encoder/encoder.cpp"
#include "encoder/encoder.h"
#include "libHidDisplayDesc.h"
#include "touch.h"



typedef struct {
	Encoder *enc;
	volatile int posNew;
	volatile int swChange;
	int pos;
}dial_t;



#define IN_ROTARY_1				1001
#define IN_ROTARY_2				1002
#define IN_ROTARY_3				1003

#define IN_SWITCH_1				1011
#define IN_SWITCH_2				1012
#define IN_SWITCH_3				1013

#define ENCODER1_PIN_CLK		30
#define ENCODER1_PIN_DT			31
#define ENCODER1_PIN_SW			32

#define ENCODER2_PIN_CLK		2
#define ENCODER2_PIN_DT			3
#define ENCODER2_PIN_SW			4

#define ENCODER3_PIN_CLK		5
#define ENCODER3_PIN_DT			6
#define ENCODER3_PIN_SW			7

#define ENCODER_SW_DEBOUNCE		120




static dial_t dial1;
static dial_t dial2;
static dial_t dial3;
//static timerSS_t timerSS;	// to resignal a render update if for whatever reason current failed



void enc1Update (const int value)
{
	if (!(value&0x03)){
		if (value != dial1.pos){
			dial1.posNew = dial1.pos - value;
			dial1.pos = value;
		}
	}
}

void enc2Update (const int value)
{
	if (!(value&0x03)){
		if (value != dial2.pos){
			dial2.posNew = dial2.pos - value;
			dial2.pos = value;
		}
	}
}

void enc3Update (const int value)
{
	if (!(value&0x03)){
		if (value != dial3.pos){
			dial3.posNew = dial3.pos - value;
			dial3.pos = value;
		}
	}
}

static void enc1Reset ()
{
	dial1.posNew = 0;
	dial1.swChange = 0;
}

static void enc2Reset ()
{
	dial2.posNew = 0;
	dial2.swChange = 0;
}

static void enc3Reset ()
{
	dial3.posNew = 0;
	dial3.swChange = 0;
}

void enc1SwCB ()
{
	static int lastPressTime;
	
	int currentPressTime = millis();
	if (currentPressTime - lastPressTime > ENCODER_SW_DEBOUNCE){
		lastPressTime = currentPressTime;
		dial1.swChange++;
	}
}

void enc2SwCB ()
{
	static int lastPressTime;
	
	int currentPressTime = millis();
	if (currentPressTime - lastPressTime > ENCODER_SW_DEBOUNCE){
		lastPressTime = currentPressTime;
		dial2.swChange++;
	}
}

void enc3SwCB ()
{
	static int lastPressTime;
	
	int currentPressTime = millis();
	if (currentPressTime - lastPressTime > ENCODER_SW_DEBOUNCE){
		lastPressTime = currentPressTime;
		dial3.swChange++;
	}
}


int encoder_isReady (encodersrd_t *encoders)
{
	encoders->encoder[0].buttonPress    = dial1.swChange;
	encoders->encoder[0].positionChange = dial1.posNew;
	enc1Reset();
	
	encoders->encoder[1].buttonPress    = dial2.swChange;
	encoders->encoder[1].positionChange = dial2.posNew;
	enc2Reset();
	
	encoders->encoder[2].buttonPress    = dial3.swChange;
	encoders->encoder[2].positionChange = dial3.posNew;
	enc3Reset();

	uint16_t somethingHappenedSW = 0;
	uint16_t somethingHappenedPS = 0;

	for (int i = 0; i < ENCODER_TOTAL; i++){
		somethingHappenedSW +=  encoders->encoder[i].buttonPress;
		somethingHappenedPS += (encoders->encoder[i].positionChange != 0);
	}

	encoders->changed = ((somethingHappenedSW<<16) | (somethingHappenedPS&0xFFFF));
	return encoders->changed;

	//return somethingHappenedSW | somethingHappenedPS;
}

void dials_init ()
{
	dial1.pos = -1;
	dial1.posNew = 0;
	dial1.swChange = 0;
	dial1.enc = new Encoder(ENCODER1_PIN_CLK, ENCODER1_PIN_DT, enc1Update);
	
	dial2.pos = -1;
	dial2.posNew = 0;
	dial2.swChange = 0;
	dial2.enc = new Encoder(ENCODER2_PIN_CLK, ENCODER2_PIN_DT, enc2Update);
	
	dial3.pos = -1;
	dial3.posNew = 0;
	dial3.swChange = 0;
	dial3.enc = new Encoder(ENCODER3_PIN_CLK, ENCODER3_PIN_DT, enc3Update);
}

void pins_init ()
{
	pinMode(LED_BUILTIN, OUTPUT);
	
	pinMode(ENCODER1_PIN_SW, INPUT_PULLDOWN);
	attachInterrupt(ENCODER1_PIN_SW, enc1SwCB, RISING);

	pinMode(ENCODER2_PIN_SW, INPUT_PULLDOWN);
	attachInterrupt(ENCODER2_PIN_SW, enc2SwCB, RISING);
	
	pinMode(ENCODER3_PIN_SW, INPUT_PULLDOWN);
	attachInterrupt(ENCODER3_PIN_SW, enc3SwCB, RISING);
}



