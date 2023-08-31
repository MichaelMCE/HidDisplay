@echo off

REM set path to MingGW here:
SET CMPPATH=F:\Compilers\MinGW


REM DO NOT MODIFY BELOW

SET PATH=%PATH%;%CMPPATH%\bin
SET DEVCI=%CMPPATH%\include
SET DEVCL=%CMPPATH%\lib
SET DEVCGCC=%CMPPATH%\bin\gcc.exe
SET DEVCDLLW=%CMPPATH%\bin\dllwrap.exe
SET DEVCRM=%CMPPATH%\bin\rm.exe
SET DEVCMAKE=%CMPPATH%\bin\make.exe

gcc -v

