@echo off

rem call gccpath.bat
del *.o


set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c

gcc -m32 -static -Wall -I%HDLIBDIR% -std=gnu18 -Wall -O2 -c draw.c drawop.c -DLIBUSB_BLOCKS -Wno-unused-function -std=gnu18 
gcc -m32 draw.o drawop.o %HDLIB% ../../libHidDisplay/libusb32.a -o draw32.exe 

strip draw32.exe 
