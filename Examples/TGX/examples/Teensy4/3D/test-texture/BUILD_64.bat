@echo off

g++ -m64 -I../../../../src -Ofast ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c test-texture.cpp ../../libs/libusb64.a -lwinmm -o test-texture64.exe
strip test-texture64.exe

