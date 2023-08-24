@echo off

rem call gccpath.bat
del *.o

gcc -m64 -std=gnu11 -Wall -O2 -c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c RawHid.c -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function 
dllwrap -m64 RawHid.o libHidDisplayAsync.o libHidDisplay.o ../../libHidDisplay/libusb64.a -o release\HidDisplay_64.dll --output-def HidDisplay.def -k --add-stdcall-alias
strip release\HidDisplay_64.dll


