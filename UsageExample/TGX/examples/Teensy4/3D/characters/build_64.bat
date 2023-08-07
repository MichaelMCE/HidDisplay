@echo off

g++ -Ofast -I../../../../src characters.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -fpermissive -o characters.exe
strip characters.exe

