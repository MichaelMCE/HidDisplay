@echo off

rem call gccpath.bat
del *.o

gcc -m32 -Wall -O3 -c screencap.c ../../libTeensyRawHid/libTeensyRawHid.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m32 screencap.o libTeensyRawHid.o ../../libTeensyRawHid/libusb32.a -ldxguid -lole32 -lcomctl32 -luuid -ld3d9 -o screencap32.exe 

strip screencap32.exe 



