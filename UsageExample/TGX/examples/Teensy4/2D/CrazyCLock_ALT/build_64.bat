@echo off

g++ -I../../../../src -Ofast crazyclock.cpp green.cpp long_hand.cpp small_hand.cpp -m64 ../../../../../../libTeensyRawHid/libTeensyRawHid.c ../../libs/libusb64.a -lwinmm -o CrazyClock.exe
strip CrazyClock.exe

