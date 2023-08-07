@echo off

g++ -Ofast -I../../../../src CrazyClock.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o CrazyClock.exe
strip CrazyClock.exe


