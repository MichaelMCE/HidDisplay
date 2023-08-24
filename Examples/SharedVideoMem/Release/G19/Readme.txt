This is intended for and only tested on VLC 32bit version 3.0.18.

Copy the plugins folder to 32bit VLC's folder. Windows should ask you if you wish to overwrite an existing file. If it doesn't then you fucked up - Startover.

Now set VLC's location:
Before starting, edit both the VLC_CaptureServer batch files, setting the location to 32bit VLC.

First we need to start the frame server - VLC.
To start Video playback, Drag'n'drop a video file on to 'VLC_CaptureServer_3.bat'
To start Visualiser only playback, drop a file on to 'VLC_CaptureServer_3_AudioVisualiser.bat'

Next start the frame client - scmem32/64.exe.
A virtual window should appear with current playback video shown, along with something on the G19.

If all goes well then disable the virtual display.
Open config.cfg with Notepad.
Look for [display 2]. Under this change active=1 to active=0.
Save.


Use Taskmanager to remove or stop scmem32/64.exe, when required, once the Virtual display is disabled.
