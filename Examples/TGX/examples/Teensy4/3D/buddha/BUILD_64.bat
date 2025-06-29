@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I%HDLIBDIR% %HDLIB% -I../../../../src buddha.cpp ../../libs/libusb64.a -lwinmm -s -o buddha64.exe
strip buddha64.exe