@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m64 -static -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c touchtest.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu18 -Wno-maybe-uninitialized
gcc -m64 %HDLIB% touchtest.o ../../libHidDisplay/libusb64.a -o touchtest64.exe 

strip touchtest64.exe 



