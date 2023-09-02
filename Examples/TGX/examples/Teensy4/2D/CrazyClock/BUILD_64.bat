@echo off

g++ -m64 -static -Ofast -I../../../../src ../../../../../../libHidDisplay/libHidDisplay.c ../../../../../../libHidDisplay/libHidDisplayAsync.c CrazyClock.cpp ../../libs/libusb64.a -lwinmm -o CrazyClock64.exe
strip CrazyClock64.exe


