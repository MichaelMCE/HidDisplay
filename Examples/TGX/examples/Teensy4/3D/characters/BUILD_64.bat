@echo off

g++ -m64 -I../../../../src -Ofast ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c characters.cpp ../../libs/libusb64.a -lwinmm -fpermissive -o characters64.exe
strip characters64.exe

