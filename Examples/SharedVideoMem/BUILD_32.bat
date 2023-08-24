@echo off

rem call gccpath.bat
del *.o

gcc -m32 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c vlc_hidclient.c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c
gcc -m32 vlc_hidclient.o libHidDisplayAsync.o libHidDisplay.o libusb32.a -o vlc_hidclient32.exe 

strip vlc_hidclient32.exe 



