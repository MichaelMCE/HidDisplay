@echo off

rem call gccpath.bat
del *.o



SET HIDDISPLAY=../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c

gcc -m32 -Wall -static -O2 -c draw.c drawop.c %HIDDISPLAY% -DLIBUSB_BLOCKS -Wno-unused-function -std=gnu11 
gcc -m32 draw.o drawop.o libHidDisplay.o libHidDisplayAsync.o ../../libHidDisplay/libusb32.a -o draw32.exe 

strip draw32.exe 



