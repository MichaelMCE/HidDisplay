@echo off

rem call gccpath.bat
del *.o

gcc -m32 -Wall -O3 -c screencap.c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m32 screencap.o libHidDisplay.o libHidDisplayAsync.o ../../libHidDisplay/libusb32.a -ldxguid -lole32 -lgdi32 -lcomctl32 -luuid -ld3d9 -o screencap32.exe 

strip screencap32.exe 


