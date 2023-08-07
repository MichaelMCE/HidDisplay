@echo off

g++ -I../../../../src -Ofast scream.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o scream.exe
strip scream.exe
