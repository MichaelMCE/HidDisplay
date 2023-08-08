@echo off

rem call gccpath.bat
del *.o

gcc -m32 -Wall -O3 -c touchtest.c ../../libTeensyRawHid/libTeensyRawHid.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m32 touchtest.o libTeensyRawHid.o ../../libTeensyRawHid/libusb32.a -o touchtest32.exe 

strip touchtest32.exe 



