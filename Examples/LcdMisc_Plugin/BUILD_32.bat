@echo off

rem call gccpath.bat
del *.o

gcc -m32 -std=gnu11 -Wall -O2 -c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c RawHid.c -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function 
gcc -shared -Wl,--dll -m32 libHidDisplayAsync.o RawHid.o libHidDisplay.o ../../libHidDisplay/libusb32.a -o release\HidDisplay_32.dll -Wl,--output-def=HidDisplay.def -Wl,--add-stdcall-alias
strip release\HidDisplay_32.dll






