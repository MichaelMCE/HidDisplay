@echo off

g++ -I../../../../src -Ofast mars.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o mars.exe
strip mars.exe