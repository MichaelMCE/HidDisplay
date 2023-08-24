@echo off

g++ -I../../../../src -Ofast scream.cpp -m64 ../../../../../../libHidDisplay/libHidDisplay.c ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../libs/libusb64.a -lwinmm -o scream64.exe
strip scream64.exe
