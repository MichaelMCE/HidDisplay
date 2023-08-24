@echo off

rem call gccpath.bat
del *.o

gcc -m64 -Wall -O3 -c touchtest.c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c -DLIBUSB_BLOCKS -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wno-maybe-uninitialized
gcc -m64 touchtest.o libHidDisplay.o libHidDisplayAsync.o ../../libHidDisplay/libusb64.a -o touchtest64.exe 

strip touchtest64.exe 



