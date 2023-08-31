
Geenral information for compiling the various programs on Windows.
310823



All software tested and built against MingGw GCC 13.2.0  
https://sourceforge.net/projects/gcc-win64/files/13.2.0/gcc-13.2.0-no-debug.7z/download

Install:
Unpack this somewhere, eg; c:\mingGw\<the unpacked files>
Edit gccpath.bat then enter edit CMPPATH to the above folder.
Copy gccpath to c:\windows directory.

Compile:
Open a command prompt (type 'cmd' in explorer).
type gccpath

32bit: gcc file.c
or
64bit: gcc -m64 file1.c file2.c -o file.exe
or
gcc -m64 -c file1.c file2.c
gcc -m64 file1.o file2.o -o file.exe
  
If all goes well, file.exe should be created and ready.


