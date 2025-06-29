@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m32 -Wall -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c touchtest.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu18 -Wno-maybe-uninitialized
gcc -m32 %HDLIB% touchtest.o ../../libHidDisplay/libusb32.a -o touchtest32.exe 

strip touchtest32.exe 



