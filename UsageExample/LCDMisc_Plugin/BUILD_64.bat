@echo off

rem call gccpath.bat
del *.o

gcc -m64 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c RawHid.c libTeensyRawHid\libTeensyRawHid.c

rem libs\libusb.a

dllwrap -m64 RawHid.o libteensyRawHid.o -o RawHid_64.dll -lusb --output-def RawHid.def -k --add-stdcall-alias
strip RawHid_64.dll


