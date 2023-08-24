@echo off

g++ -m64 -I../../../../src -Ofast ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c test-shading.cpp ../../libs/libusb64.a -lwinmm -o test-shading64.exe
strip test-shading64.exe
