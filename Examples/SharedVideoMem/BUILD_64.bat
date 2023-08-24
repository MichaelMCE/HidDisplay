@echo off

rem call gccpath.bat
del *.o

gcc -m64 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c vlc_hidclient.c ../../libHidDisplay/libHidDisplayAsync.c ../../libHidDisplay/libHidDisplay.c

rem add -mwindows to enable windowless build
gcc -m64 vlc_hidclient.o libHidDisplayAsync.o libHidDisplay.o libusb64.a -o vlc_hidclient64.exe 

strip vlc_hidclient64.exe 



