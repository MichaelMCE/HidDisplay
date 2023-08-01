@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Wall -O3 -c screencap.c ../../libTeensyRawHid/libTeensyRawHid.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m64 screencap.o libTeensyRawHid.o ../../libTeensyRawHid/libusb64.a -ldxguid -lole32 -lcomctl32 -luuid -ld3d9 -o screencap64.exe 

strip screencap64.exe 



