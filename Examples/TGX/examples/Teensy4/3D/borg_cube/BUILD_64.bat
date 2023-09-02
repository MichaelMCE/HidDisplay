@echo off

g++ -m64 -static -Ofast -I../../../../src borg_cube.cpp ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c ../../libs/libusb64.a -lwinmm -o borg_cube64.exe
strip borg_cube64.exe