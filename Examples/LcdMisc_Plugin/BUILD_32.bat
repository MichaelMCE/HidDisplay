@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m32 -static -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c RawHid.c -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function 
gcc -m32 -shared -Wl,--dll %HDLIB% RawHid.o ../../libHidDisplay/libusb32.a -o release\HidDisplay_32.dll -Wl,--output-def=HidDisplay.def -Wl,--add-stdcall-alias
strip release\HidDisplay_32.dll






