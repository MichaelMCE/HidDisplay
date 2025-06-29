8bit and 16bit Parallel display driver for Teensy 4.1 microcontroller

Supported controllers:
NT35516		960x540
ILI9806		854x480
RM68120		800x480
R61408 		800x480
ILI9486		480x320
R61529		480x320
S6D04D1		432x240
GC9A01A		240x240
ST7735		160x128

WIP:
NT35510		800x480
LG4572B		800x480


Wiring:
Panel 	Teensy 4.1 Pin

Configurable pins:
RS/DC	10
RST	11
CS	12
BL	33

Non configurable:
WR	36
RD	37

D0	19
D1	18
D2	14
D3	15
D4	40
D5	41
D6	17
D7	16
	
D8	22
D9	23
D10	20
D11	21
D12	38
D13	39
D14	26
D15	27


SPI displays:
RST	8
DC	9
CS	10
MOSI	11
SCK	13
BL	33


Capacitive touch using FT5Xxx series with i2c for communication:
SCL	24
SDA	25
INT	34

Displays which support dual touch panels may share data lines with both i2c and SPI (PT2046) controllers.
Usually only the SPI is labelled.


         AP040IPS15DP    TK050F5590    AC3585    AC1590
SDA:         SDA            T_CS        T_CS      MOSI
SCL:         SCK            SCK         SCL       SCK
INT:         INT            INT         INT       INT


Display boards with selectable 8bit or 16bit interface (via resistors), select 16bit.
As for voltage - Always select 3.3v, including for the backlight.



