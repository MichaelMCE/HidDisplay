This is a server - client package for streaming video to a Teensy 4.1 connected display from a desktop computer over USB.


Instructions:
1) Compile & Upload the teensy/teensy.ino firmware
2) Reconnect the Teensy
3) Install the Desktop libusb0 based driver by using the included Inf-Wizard.
   Open Inf-Wizard.exe then select the Teensy RawHid interface 0 device.
   Select "Install now". No need to save the driver. You can always 'Roll back' afterwards.
4) Reconnect the Teensy

Teensy is now ready to receive video frames.
Checkout UsageExample/LCDMisc_Plugin/TeensyRawHid.c as a starting point.
  


----
Tgx Mars demo rendered on a S6D04D1 432x240, streamed over USB:
[![Tgx Mars demo rendered on a S6D04D1, streamed over USB](https://img.youtube.com/vi/iDyoVsCKpxk/0.jpg)](https://www.youtube.com/watch?v=iDyoVsCKpxk)
