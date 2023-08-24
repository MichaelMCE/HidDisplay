
VLC_CaptureServer.bat is a server-client application to enable video streaming directly to the display.
To use: Edit VLC_CaptureServer.bat with notepad, set display resolution and vlc.exe's location - Save.


svmem is a video output module for VLC using a shared memory IPC interface.
svmem allows the C/C++ developer to add streaming video to an application, external to VLC, with only a few lines of code.

To use:
Review the example source code and try the included test viewer.

Basically it amounts to:
include "svmem.h"
Create a handle to shared memory map file "VLC_SVIDEOPIPE".
Create a handle to named shared event "VLC_SMEMEVENT". This is used by the plugin to signal when an update has occurred.
Create a handle to named Semaphore "VLC_SMEMLOCK". This is used to synchronize read/writes to the shared memory.

Example method:
do
 wait for the update signal
  got signal, wait for the semaphore lock
  got lock
   copy video frame
   release lock
  process video
loop


Install:
copy libsvmem_plugin.dll to vlc/plugins/video_output/

Enable:
In the VLC preferences dialog (Ctrl+P) set 'Show settings' to 'All'
Select 'Video' -> 'Output modules' then select 'Shared video memory output' from the drop list on the right hand side.

Video Configure:
Select 'Video' -> 'Output modules' -> 'Shared Video memory'
From here set your desired width, height and Chroma (BPP).

Visualization Configure:
Goom: Select 'Audio' -> 'Visualizations' -> 'Goom'. Set width and height to match the video Configuration.
Visualizer: Select 'Audio' -> 'Visualizations' -> 'Visualizer'. Set width and height to match the video Configuration.


