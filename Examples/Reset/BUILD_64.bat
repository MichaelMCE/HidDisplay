@echo off

rem call gccpath.bat
del *.o



SET HIDDISPLAY=../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c

gcc -m64 -Wall -static -O2 -c reset.c %HIDDISPLAY% -DLIBUSB_BLOCKS -Wno-unused-function -std=gnu11 
gcc -m64 reset.o libHidDisplay.o libHidDisplayAsync.o ../../libHidDisplay/libusb64.a -o reset64.exe 

strip reset64.exe 



