@echo off

g++ -I../../../../src -Ofast crazyclock.cpp green.cpp long_hand.cpp small_hand.cpp -m64 ../../../../../../libHidDisplay/libHidDisplay.c ../../../../../../libHidDisplay/libHidDisplayAsync.c ../../libs/libusb64.a -lwinmm -o CrazyClock64.exe
strip CrazyClock64.exe

