This is a server - client package for streaming video to a Teensy 4.1 connected display from a desktop computer over USB.


Instructions:
Before compiling, a few modifications are required to the core Teensys4 source:

1) Apply this patch, or manually modify:
https://github.com/PaulStoffregen/cores/pull/629

2) Change USB endpoint type from Interrupt to Bulk:
Modify Teenhardware\teensy\avr\cores\teensy4\usb_desc.h
After the patch and around line 630, look for the following line:
 "#elif defined(USB_RAWHID) || defined(USB_RAWHID512)"
Within this #define section (~line 620-630) look for:

RAWHID_RX_INTERVAL, set this to 0

And
#define ENDPOINT3_CONFIG    ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_INTERRUPT
#define ENDPOINT4_CONFIG    ENDPOINT_RECEIVE_INTERRUPT + ENDPOINT_TRANSMIT_UNUSED
Replace with:
#define ENDPOINT3_CONFIG    ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_BULK
#define ENDPOINT4_CONFIG    ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_UNUSED


Modify Teenhardware\teensy\avr\cores\teensy4\usb_desc.c
Look for "#ifdef RAWHID_INTERFACE" ~line 1200
This will modify the configuration for USB @ 480 Mbit speed

Change the bmAttribute from Interrupt to Bulk
Look for two instances off (~line 1230):
0x03,                                   // bmAttributes (0x03=intr)
Changed both to:
0x02,                                   // bmAttributes (0x03=intr, 0x02=bulk)


Now we're ready:
1) Open Teensy/config.h - Select controller and preferences.
2) Select USB Type: Arduino Tools menu -> USB Type -> Raw Hid 512
3) Compile & Upload the teensy/teensy.ino firmware.
4) Reconnect the Teensy.
5) Install the Desktop libusb0 based driver by using the included Inf-Wizard.
   Open Inf-Wizard.exe then select the Teensy RawHid interface 0 device.
   Select "Install now". You can always 'Roll back' afterwards. No need to save the driver.
6) Reconnect the Teensy

Teensy is now ready to receive video frames.
Checkout Examples/LCDMisc_Plugin/RawHid.c as a starting point.



----
Tgx's Mars demo rendered on a 432x240 S6D04D1 @ 120fps, streamed over USB from desktop:
https://www.youtube.com/watch?v=iDyoVsCKpxk
  
Topdown view of Teensy 4.1 connected to a 854x480 display.
https://github.com/MichaelMCE/HidDisplay/blob/main/Extra/ILI9806-854x480.jpg
