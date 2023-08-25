
svmem is a video output module for VLC 1.0.x using a shared memory IPC interface.
A module for VLC 1.1.x will be produced eventually.


Install:
copy libsvmem_plugin.dll to vlc/plugins/

Enable:
In the VLC preferences dialog (Ctrl+P) set 'Show settings' to 'All'
Select 'Video' -> 'Output modules' then select 'Shared video memory output' from the drop list on the right hand side.

Video Configure:
Select 'Video' -> 'Output modules' -> 'Shared Video memory'
From here set your desired width, height and Chroma (BPP).

Visualization Configure:
Goom: Select 'Audio' -> 'Visualizations' -> 'Goom'. Set width and height to match the video Configuration.
Visualizer: Select 'Audio' -> 'Visualizations' -> 'Visualizer'. Set width and height to match the video Configuration.


