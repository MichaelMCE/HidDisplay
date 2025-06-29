@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m64 -static -I%HDLIBDIR% -std=gnu18 -Wall -Ofast -c screencap.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu18 -Wno-maybe-uninitialized
gcc -m64 screencap.o %HDLIB% ../../libHidDisplay/libusb64.a -ldxguid -lgdi32 -lole32 -lcomctl32 -luuid -ld3d9 -o screencap64.exe 

strip screencap64.exe 



