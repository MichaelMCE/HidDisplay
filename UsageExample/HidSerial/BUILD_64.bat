@echo off

gcc -m64 -Wall hidserial.c hidapi/x64/hidapi.a -s -o hidserial64.exe