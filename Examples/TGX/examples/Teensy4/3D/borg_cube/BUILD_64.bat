@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I%HDLIBDIR% %HDLIB% -I../../../../src borg_cube.cpp ../../libs/libusb64.a -lwinmm -o borg_cube64.exe
strip borg_cube64.exe