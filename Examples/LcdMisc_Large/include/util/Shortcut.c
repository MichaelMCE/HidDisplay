#requires <framework\Overlay.c>
#import <constants.h>

function GetClipboardText() {
	for ($i=0; $i<3; $i++) {
		$temp = GetClipboardData();
		if (IsList($temp)) {
			if (IsString($temp[1]) && $temp[0] == 13) return $temp[1];
			return $temp[0];
		}
		Sleep(10);
	}
}

function LaunchURLClipboard($prefix, $suffix) {
	if (IsString($text = GetClipboardText())) {
		Run($prefix +s $text +s $suffix,,2);
		return 1;
	}
	else {
		if (IsNull($text))
			MakeTimedOverlay("Can't read Clipboard.", 5000);
		else
			MakeTimedOverlay("Invalid data in Clipboard.|n" +s $text +s ": Not text or unknown format.", 5000);
	}
	return 0;
}

function LaunchGoogleClipboard() {
	return LaunchURLClipboard("http://www.google.com/search?q=");
}

function LaunchWikiClipboard() {
	return LaunchURLClipboard("http://en.wikipedia.org/wiki/");
}

function LaunchDictClipboard() {
	return LaunchURLClipboard("http://dictionary.reference.com/browse/");
}

function LaunchGoogleClipboardPlus() {
	if (IsString($text = GetClipboardText())) {
		$index = strstr($text, ":");

		// Probably a URL.
		if (!IsNull($index) && $index < 8)
			Run($text,,2);
		else {
			$index = strstr($text, ".");

			// Probably url without the http.  Might not be...
			if (!IsNull($index) && $index < size($text)-3)
				Run("http://" +s $text,,2);
			// Just use google.
			else
				Run("http://www.google.com/search?q=" +s $text,,2);
		}
		return 1;
	}
	else {
		if (IsNull($text))
			MakeTimedOverlay("Can't read Clipboard.", 5000);
		else
			MakeTimedOverlay("Invalid data in Clipboard.|n" +s $text +s ": Not text or unknown format.", 5000);
	}
	return 0;
}

function KillCaps() {
	Wait(200);
	if (!capsSet && (GetKeyState(VK_CAPITAL)&0x1) == 1) {
		capsSet = 1;
		KeyDown(VK_CAPITAL);
		Wait(50);
		KeyUp(VK_CAPITAL);
		Wait(250);
		capsSet = 0;
	}
}

// Traps cursor on current monitor.
function ToggleCursorLock() {
	if (gCursorLocked) {
		gCursorLocked = 0;
		ClipCursor();
	}
	else {
		$pos = GetCursorPos();
		$monitors = EnumDisplayMonitors();
		for ($i=0; $i < size($monitors); $i++) {
			if ($monitors[$i][0] <= $pos[0] && $pos[0] <= $monitors[$i][2] &&
				$monitors[$i][1] <= $pos[1] && $pos[1] <= $monitors[$i][3]) {
					gCursorLocked = 1;
					ClipCursor(@$monitors[$i]);
					return;
			}
		}
	}
}

struct VolumeSmoother {
	var %oldValue;
	
	function VolumeSmoother ($_eventHandler){
		$_eventHandler.Insert(100, $this);
		%oldValue = GetAudioValue(0, -4, 0, -0x50030001, 1);
	}
	
	function AudioChange (){
		$newValue = GetAudioValue(0, -4, 0, -0x50030001, 1);
		$delta = 512;
		
		if ($newValue - %oldValue > $delta && $newValue - %oldValue < 4000) {
			SetAudioValue(0, -4, 0, -0x50030001, %oldValue+=$delta);
		}else if (%oldValue-$newValue > $delta && %oldValue-$newValue < 4000) {
			SetAudioValue(0, -4, 0, -0x50030001, %oldValue-=$delta);
		}else {
			%oldValue = $newValue;
		}
	}
};
