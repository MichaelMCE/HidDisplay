@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I../../../../src crazyclock.cpp green.cpp long_hand.cpp small_hand.cpp -I%HDLIBDIR% %HDLIB% ../../libs/libusb64.a -lwinmm -o CrazyClock64.exe
strip CrazyClock64.exe

