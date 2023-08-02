@echo off

rem call gccpath.bat
del *.o

gcc -m32 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c RawHid.c ../../libTeensyRawHid/libTeensyRawHid.c
gcc -shared -Wl,--dll -m32 RawHid.o libteensyRawHid.o ../../libTeensyRawHid/libusb32.a -o HidDisplay_32.dll -Wl,--output-def=HidDisplay.def -Wl,--add-stdcall-alias
strip HidDisplay_32.dll






