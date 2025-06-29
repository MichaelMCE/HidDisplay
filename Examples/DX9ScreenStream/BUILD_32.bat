@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m32 -static -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c screencap.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu18 -Wno-maybe-uninitialized
gcc -m32 screencap.o %HDLIB% ../../libHidDisplay/libusb32.a -ldxguid -lole32 -lgdi32 -lcomctl32 -luuid -ld3d9 -o screencap32.exe 

strip screencap32.exe 



