@echo off

g++ -m64 -static -Ofast -I../../../../src ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c test-shading.cpp ../../libs/libusb64.a -lwinmm -o test-shading64.exe
strip test-shading64.exe
