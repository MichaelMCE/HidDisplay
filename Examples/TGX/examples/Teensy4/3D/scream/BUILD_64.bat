@echo off

g++ -m64 -static -I../../../../src -Ofast scream.cpp ../../../../../../libHidDisplay/libHidDisplay.c ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../libs/libusb64.a -lwinmm -o scream64.exe
strip scream64.exe
