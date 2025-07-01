@echo off


gcc -m32 -c -O2 libHidDisplay.c
gcc -m32 -c -O2 libHidDisplayAsync.c

gcc -m32 -shared -Wl,--output-def=libhiddisplay.def -Wl,--dll -Wl,--out-implib=libhiddisplay.dll.a libHidDisplayAsync.o libHidDisplay.o libusb32.a -o hiddisplay.dll
ar rc libhiddisplay_tmp.a libHidDisplayAsync.o libHidDisplay.o

ar -M <pack32.mri
ranlib libhiddisplay.a

del libhiddisplay_tmp.a
del libHidDisplayAsync.o 
del libHidDisplay.o

strip HidDisplay.dll
move HidDisplay.dll release/x32/HidDisplay.dll
move libhiddisplay.dll.a release/x32/
move libhiddisplay.a release/x32/

