@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m64 -static -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c RawHid.c -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function 
dllwrap -m64 %HDLIB% RawHid.o ../../libHidDisplay/libusb64.a -o release\HidDisplay_64.dll --output-def HidDisplay.def -k --add-stdcall-alias
strip release\HidDisplay_64.dll


