@echo off

rem call gccpath.bat
del *.o



set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m64 -Wall -static -I%HDLIBDIR% -std=gnu18 -Ofast -c reset.c -DLIBUSB_BLOCKS -Wno-unused-function
gcc -m64 %HDLIB% reset.o ../../libHidDisplay/libusb64.a -o reset64.exe 

strip reset64.exe 



