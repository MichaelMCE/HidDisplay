@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Wall -O3 -c screencap.c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m64 screencap.o libHidDisplay.o libHidDisplayAsync.o ../../libHidDisplay/libusb64.a -ldxguid -lgdi32 -lole32 -lcomctl32 -luuid -ld3d9 -o screencap64.exe 

strip screencap64.exe 



