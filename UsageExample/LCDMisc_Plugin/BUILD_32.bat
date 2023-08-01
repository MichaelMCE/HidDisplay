@echo off

rem call gccpath.bat
del *.o

gcc -m32 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c RawHid.c ../../libTeensyRawHid/libTeensyRawHid.c


dllwrap -m32 RawHid.o libteensyRawHid.o ../../libTeensyRawHid/libusb32.a -o RawHid_32.dll --output-def RawHid.def -k --add-stdcall-alias
strip RawHid_32.dll


