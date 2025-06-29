@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I%HDLIBDIR% %HDLIB% -I../../../../src characters.cpp ../../libs/libusb64.a -lwinmm -fpermissive -o characters64.exe
strip characters64.exe

