@echo off

set HDLIBDIR=../../../../../../libHidDisplay
set HDLIB=%HDLIBDIR%/libHidDisplay.c %HDLIBDIR%/libHidDisplayAsync.c
g++ -m64 -static -Ofast -I%HDLIBDIR% %HDLIB% -I../../../../src test-texture.cpp ../../libs/libusb64.a -lwinmm -o test-texture64.exe
strip test-texture64.exe

