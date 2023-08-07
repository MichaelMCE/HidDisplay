@echo off

g++ -I../../../../src -Ofast test-shading.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o test-shading.exe
strip test-shading.exe
