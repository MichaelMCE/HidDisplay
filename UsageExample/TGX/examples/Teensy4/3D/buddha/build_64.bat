@echo off

g++ -Ofast -I../../../../src buddha.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o buddha.exe
strip buddha.exe