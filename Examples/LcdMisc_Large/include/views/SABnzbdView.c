// SABnzbdView.c module 0.1.7 (20090419)
// 4wd   major.crap@gmail.com
//
// For LCDMisc by Dwarg for the Logitech G15
//
// http://host/sabnzbd/api?mode=qstatus&output=xml
// http://host/sabnzbd/api?mode=pause
// http://host/sabnzbd/api?mode=resume
// http://host/sabnzbd/api?mode=shutdown
#import <Views\View.c>
#import <constants.h>
#requires <framework\header.c>
#requires <util\XML.c>

struct SABnzbdView extends View {
	var %dlspeed,
    %paused,
    %leftmb,
    %totalmb,
    %numdl,
    %dspace1,
    %dspace2,
    %timeleft,
    %joblist,
	%offset,
	%scrollTimer,
	%url,
	%mode,
	%lastUpdate,
	%miniFont,
    %displayLocation,
    %subview,
    %warnings;

	function SABnzbdView ($_url, $_location, $_sub) {
		%noDrawOnCounterUpdate = 0;

		%lastUpdate = Time()-60*60-5;
		%toolbarImage = LoadImage("Images\SABnzbd.png");

		%miniFont = Font("6px2bus", 6);
		
		if (!size($_url)) {
			%url = GetString("URLs", "SABnzbd");
		}
		else
			%url = $_url;

        if (strstr(%url, "apikey", 0, 20)) {
            %mode = "&";
        }
        else
            %mode = "api?";
		// Draw on counter updates and after 1.5 second intervals for scrolling.
		// Means more redraws than are really needed, but scrolling is too fast at 1 second,
		// too slow at 2 second, and need to update the time display, too.
		%noDrawOnAudioChange = 1;

		%displayLocation = $_location;
		%subview = $_sub;
	}

	function Show() {
		if (!%scrollTimer) {
			%offset = 0;
			%scrollTimer = CreateTimer("Bump", 5,,$this);
		}
		if (%subview) {
      RegisterKeyEvent(0, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
    }
	}

	function Hide() {
		if (%scrollTimer) {
			KillTimer(%scrollTimer);
			%scrollTimer = 0;
    }
		if (%subview) {
			UnregisterKeyEvent(0, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
		}
	}

	function Update() {
      $rawdata = null;
	  $rawdata = HttpGetWait(%url +s %mode +s "mode=qstatus&output=json");
	  if (!IsNull($rawdata)) {
	    $json = JSONdecode($rawdata);
	    %warnings = $json["have_warnings"];
        %numdl = $json["noofslots"];
        %dlspeed = FormatValue($json["kbpersec"], 3, 2);
        %paused = $json["paused"];
        %totalmb = FormatValue($json["mb"]/1024, 3, 1);
        %leftmb = FormatValue($json["mbleft"]/1024, 3, 1);
        %dspace1 = FormatValue($json["diskspace1"], 3, 2);
        %dspace2 = FormatValue($json["diskspace2"], 3, 2);
        %timeleft = $json["timeleft"];
	    $pos = 0;
        %joblist = list();
        $jobs = $json["jobs"];
        if (IsList($jobs))
		  while ($pos < size($jobs)) {
            %joblist[$pos] = ($jobs[$pos]["msgid"],
                            $jobs[$pos]["filename"],
                            $jobs[$pos]["mbleft"],
                            $jobs[$pos]["id"],
                            $jobs[$pos]["mb"]
                           );
//            WriteLogLn(%joblist[0][1], 1);


			$pos++;
          }
		NeedRedraw();
      }
      else %dlspeed = null;
    }

	function PerformActionWait($action) {
	  return HttpGetWait(%url +s %mode +s "mode=" +s $action);
	}

	function KeyDown($event, $param, $modifiers, $vk) {
  	if ($vk == VK_MEDIA_STOP) {
			%PerformActionWait("shutdown");
		}
		else if ($vk == VK_MEDIA_PLAY_PAUSE) {
			if (!%paused) {
				%PerformActionWait("pause");
			}
			else {
				%PerformActionWait("resume");
			}
		}
		SpawnThread("Update", $this);
	}

	function Focus() {
		if (!%hasFocus) {
			RegisterKeyEvent(0, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			UnregisterKeyEvent(0, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE);
			%hasFocus = 0;
		}
	}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 0xF) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
			}
			else {
				if ($buttons & 8) {
					%Unfocus();
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function Draw() {
		ClearScreen();
		UseFont(0);
    ClearRect(0,0,159,6);
    DisplayTextRight(FormatTime("HH:NN:SS"), 57);
		DrawLine(75, 0, 75, 42);
		DrawLine(0, 7, 75, 7);
    if (Time()-%lastUpdate >= 30) {
			%lastUpdate = Time();
			SpawnThread("Update", $this);
		}
		if (IsNull(%dlspeed)) {
      UseFont(0);
      ClearRect(35, 22, 125, 30);
      DrawLine(35, 22, 125, 22);
      DrawLine(35, 30, 125, 30);
      DrawLine(35, 22, 35, 30);
      DrawLine(125, 22, 125, 30);
      DisplayText("No SABnzbd data", 38, 23);
		}
		else {
      UseFont(%miniFont);
      DisplayText("IP:" +s substring(%url, 7, strstr(%url, ":", 8)), 2, 9);
      DisplayTextRight("[" +s %numdl +s "]", 74, 15);
      DisplayText("SPD:" +s %dlspeed +s "K", 2, 15);
      DisplayText("REM:" +s %leftmb +s "/" +s %totalmb +s " GB", 2, 21);
      DisplayTextRight("[" +s %warnings +s "]", 74, 27);
  	  if (%dspace1 == %dspace2) {
        DisplayText("SPC:" +s %dspace1 +s " GB", 2, 27);
      }
      else {
        DisplayText("COMP:" +s %dspace1 +s " GB", 2, 27);
        DisplayText("DNLD:" +s %dspace2 +s " GB", 2, 33);
      }

      $height = 2;
      $jobleft = 0;
		for (;$i<3;$i++) {
		  $job = %joblist[$i];
		  $height += ((7 * $i) - (7 * ($i == 2)));
		  if (TextSize($job[1])[0] > 105) {
			$job[1] = substring($job[1], 0, 19);
		  }
		  DisplayText($job[1], 77, $height);
		  InvertRect(76, $height - 1, 159, $height + 5);
		  DisplayText(FormatValue($job[2], 1, 2), 77, $height + 7);
		  $jobleft += FormatValue(($job[2] / (%dlspeed / 1024)), 6, 0);
		  if (%dlspeed > 2)
	    	DisplayTextRight(FormatTime("DDD HH:NN", Time() + $jobleft), 159, $height + 7);
          else
            DisplayTextRight("Unknown", 159, $height + 7);
  		  $height += 7;
		}
		UseFont(0);

 		if (%paused) {
          ClearRect(61, 22, 99, 30);
          DrawLine(61, 22, 99, 22);
          DrawLine(61, 30, 99, 30);
          DrawLine(61, 22, 61, 30);
          DrawLine(99, 22, 99, 30);
          DisplayText("Pause!", 64, 23);
        }
      }
	}
};
