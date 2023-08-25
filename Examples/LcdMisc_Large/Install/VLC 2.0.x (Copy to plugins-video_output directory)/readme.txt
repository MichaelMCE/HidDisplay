
libsvmem_plugin.dll:
svmem is a video output module for VLC 2.0.x using a shared memory IPC interface.


Install:
copy libsvmem_plugin.dll to vlc/plugins/video_output

Enable:
In the VLC preferences dialog (Ctrl+P) set 'Show settings' to 'All'
Select 'Video' -> 'Output modules' then select 'Shared video memory output' from the drop list on the right hand side.

Video Configure:
Select 'Video' -> 'Output modules' -> 'Shared Video memory'
From here set your desired width, height and Chroma (BPP).

Visualization Configure:
Goom: Select 'Audio' -> 'Visualizations' -> 'Goom'. Set width and height to match the video Configuration.
Visualizer: Select 'Audio' -> 'Visualizations' -> 'Visualizer'. Set width and height to match the video Configuration.



libsbui_plugin.dll:
VLC video output module for SBUI with the sole purpose of redirecting all video to the SBUI device.
Might be unstable.

Install:
copy libsbui_plugin.dll to vlc/plugins/video_output

