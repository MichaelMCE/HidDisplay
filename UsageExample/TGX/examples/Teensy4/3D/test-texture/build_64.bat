@echo off

g++ -I../../../../src -Ofast test-texture.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o test-texture.exe
strip test-texture.exe

