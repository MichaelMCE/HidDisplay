@echo off

g++ -m64 -static -Ofast -Wall -I../../../../src ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../../../../../libHidDisplay/libHidDisplay.c buddha.cpp ../../libs/libusb64.a -lwinmm -s -o buddha64.exe
rem strip buddha64.exe