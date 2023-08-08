@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Wall -O3 -c touchtest.c ../../libTeensyRawHid/libTeensyRawHid.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m64 touchtest.o libTeensyRawHid.o ../../libTeensyRawHid/libusb64.a -o touchtest64.exe 

strip touchtest64.exe 



