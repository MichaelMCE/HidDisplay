@echo off

rem call gccpath.bat
del *.o

gcc -m64 -DLIBUSB_BLOCKS -DUSE_MMX -DHAVE_MMX -DHAVE_MMX1 -mmmx -Wno-unused-function -std=gnu11 -Wall -O2 -c TeensyRawHid.c ..\..\Desktop\libTeensyRawHid.c

rem libs\libusb.a

dllwrap -m64 TeensyRawHid.o libteensyRawHid.o -o TeensyRawHid_64.dll -lusb --output-def TeensyRawHid.def -k --add-stdcall-alias
strip TeensyRawHid_64.dll


