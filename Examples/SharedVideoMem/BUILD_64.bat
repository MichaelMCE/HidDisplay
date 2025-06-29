@echo off

rem call gccpath.bat
del *.o

set HDLIBDIR=../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
gcc -m64 -Ofast -I%HDLIBDIR% -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu18 -Wall -c vlc_hidclient.c 
gcc -m64 -Ofast %HDLIB% vlc_hidclient.o libusb64.a -o vlc_hidclient64.exe 

strip vlc_hidclient64.exe 
