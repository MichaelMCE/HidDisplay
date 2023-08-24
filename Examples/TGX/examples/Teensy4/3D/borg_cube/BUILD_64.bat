@echo off

g++ -Ofast -I../../../../src borg_cube.cpp -m64 ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c ../../libs/libusb64.a -lwinmm -o borg_cube64.exe
strip borg_cube64.exe