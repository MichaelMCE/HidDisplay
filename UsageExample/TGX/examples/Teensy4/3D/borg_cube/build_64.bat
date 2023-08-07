@echo off

g++ -Ofast -I../../../../src borg_cube.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o borg_cube.exe
strip borg_cube.exe