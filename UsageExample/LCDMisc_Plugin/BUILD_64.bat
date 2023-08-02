@echo off

rem call gccpath.bat
del *.o

gcc -m64 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c RawHid.c ../../libTeensyRawHid/libTeensyRawHid.c

rem libs\libusb.a

dllwrap -m64 RawHid.o libteensyRawHid.o ../../libTeensyRawHid/libusb64.a -o HidDisplay_64.dll --output-def HidDisplay.def -k --add-stdcall-alias
strip HidDisplay_64.dll


