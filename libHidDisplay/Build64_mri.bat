@echo off


gcc -m64 -c -O2 libHidDisplay.c
gcc -m64 -c -O2 libHidDisplayAsync.c

gcc -m64 -shared -Wl,--output-def=libhiddisplay.def -Wl,--dll -Wl,--out-implib=libhiddisplay.dll.a libHidDisplayAsync.o libHidDisplay.o libusb64.a -o hiddisplay.dll
ar rc libhiddisplay_tmp.a libHidDisplayAsync.o libHidDisplay.o

ar -M <pack64.mri
ranlib libhiddisplay.a

del libhiddisplay_tmp.a
del libHidDisplayAsync.o 
del libHidDisplay.o

strip HidDisplay.dll
move HidDisplay.dll release/x64/HidDisplay.dll
move libhiddisplay.dll.a release/x64/
move libhiddisplay.a release/x64/
