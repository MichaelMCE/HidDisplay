@echo off

rem Set VLC location - Is Need for drag'n'drop - No quotes
SET VLCPATH=C:\Program Files (x86)\VLC

rem Display 
SET WIDTH=800
SET HEIGHT=480

rem Goom Width must be divisible by 64
SET GOOM_WIDTH=832
SET GOOM_HEIGHT=480
rem SET GOOM_WIDTH=%WIDTH%
rem SET GOOM_HEIGHT=%HEIGHT%

rem for audio visuals plugin
SET EFFECT_WIDTH=%WIDTH%
SET EFFECT_HEIGHT=%HEIGHT%


Start "" "%VLCPATH%\vlc.exe" --audio-desync=100 --no-video-deco --audio-visual=visual --vout=svmem --svmem-width=%WIDTH% --svmem-height=%HEIGHT% --svmem-chroma=RV16 --goom-width=%GOOM_WIDTH% --goom-height=%GOOM_HEIGHT% --effect-width=%EFFECT_WIDTH% --effect-height=%EFFECT_HEIGHT% %1 --effect-fft-window=hann --no-qt-video-autoresize --qt-system-tray --qt-start-minimized --qt-auto-raise=0

rem cmd /C call 


