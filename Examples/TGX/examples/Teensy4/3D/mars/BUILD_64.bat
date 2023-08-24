@echo off

g++ -m64 -I../../../../src -Ofast ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c mars.cpp ../../libs/libusb64.a -lwinmm -o mars64.exe
strip mars64.exe