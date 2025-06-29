@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I%HDLIBDIR% %HDLIB% -I../../../../src CrazyClock.cpp ../../libs/libusb64.a -lwinmm -o CrazyClock64.exe
strip CrazyClock64.exe


