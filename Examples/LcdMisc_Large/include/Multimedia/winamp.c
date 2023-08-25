#import <Multimedia\MediaPlayerController.c>


struct WinampController extends MediaPlayerController {
	var %handle, %rawVolume, %rawBalance, %changed;
	var %spectrum, %waveform, %specHist, %sock, %lastReceived;

	function WinampController() {
		//%mediaImage = LoadImage("Images\winamp.png");
		
		//%bigMediaImage = LoadImage32("Images\winamp2.png");
		%bigMediaImage = "Images\winamp2.png";
		
		%lastDrawn = Time();
		%state = -1;
		%playerName = "Winamp";
	}

	function Stop() {
		PostMessage(%handle, 0x0111, 40047, 0);
	}

	function PlayPause() {
		if (%state == 2) {
			%Pause();
		}else{
			%Play();
		}
	}

	function Pause() {
		//if (%state != 2) {
			PostMessage(%handle, 0x0111, 40046, 0);
		//}
	}

	function ChangeMode($v) {
		$data = GetSharedMemory("LCDMisc Winamp");
		$m = (ParseBinaryInt($data, 20, 2)+3 + $v)%3;
		// Repeat
		PostMessage(%handle, 0x0400, $m >= 1, 253);
		// Shuffle
		PostMessage(%handle, 0x0400, $m == 2, 252);
	}

	function Play() {
		if (%state == 0) {
			PostMessage(%handle, 0x0111, 40045, 0);
		}
		else if (%state == 1) {
			PostMessage(%handle, 0x0111, 40046, 0);
		}
	}

	function Next() {
		PostMessage(%handle, 0x0111, 40048, 0);
	}

	function Prev() {
		PostMessage(%handle, 0x0111, 40044, 0);
	}

	function Update() {
		if (Time() - %lastReceived >= 2) {
			%spectrum = null;
			%waveform = null;
			%specHist = null;
			if (IsNull(%sock)) {
				%sock = Socket("127.0.0.1", 33495, IPPROTO_UDP, "HandleVisSocketMessage", $this);
			}
		}
		$data = GetSharedMemory("LCDMisc Winamp");
		if (ParseBinaryInt($data, 0, 2) != 1) {
			%state = -1;
			%handle = 0;
			return;
		}

		%state = ParseBinaryInt($data, 2, 2);
		if (!%changed) {
			%rawVolume = ParseBinaryInt($data, 4, 2);
			%rawBalance = ParseBinaryInt($data, 6, 2);
		}
		else %changed--;
		%volume = (%rawVolume/2.55)|0;
		%balance = %rawBalance*100/127;
		if (%muted) {
			if (%volume) %muted = 0;
			else %volume = %muted;
		}
		%handle = ParseBinaryInt($data, 8, 4);

		// Only thing worth the extra communication overhead...
		// Think shared memory is faster than sending a whole bunch of
		// interprocess messages, though I could be wrong.
		%position = SendMessage(%handle, 0x0400, 0, 105); ///1000;
		// On 64-bit systems, get a 32-bit number read as a 64-bit one.  So propogate
		// the sign of the 32-bit number.  The and isn't necessary, but doesn't hurt.
		$mask = ((%position & 0x80000000)<<32)>>32;
		%position |= $mask;
		%position &= $mask |  0x7FFFFFFF;
		%position /= 1000;
		//%position = ParseBinaryInt($data, 12, 4)/1000;

		%duration = ParseBinaryInt($data, 16, 4);
		%mode = list("Once", "Loop", "Shuffle")[ParseBinaryInt($data, 20, 2)];
		%year = ParseBinaryInt($data, 22, 2);
		%track = ParseBinaryInt($data, 24, 4);
		%tracks = ParseBinaryInt($data, 28, 4);

		%title = ParseBinaryStringUTF8($data, 32, 4096);
		%file = ParseBinaryStringUTF8($data, 33+size(%title), 4096);
		%artist = ParseBinaryStringUTF8($data, 34+size(%title)+size(%file), 4096);
	}

	function HandleVisSocketMessage($reserved, $error, $msg, $socket) {
		if (!Equals($socket, %sock)) return;
		if ($msg != FD_READ) {
			if ($msg == FD_WRITE) return;
			%spectrum = null;
			%waveform = null;
			%specHist = null;
			%sock = null;
			return;
		}
		%lastReceived = Time();
		while (size($temp = $socket.ReadFrom()[0])) {
			if (size($temp) < 20) continue;
			if (substring($temp, 0, 20) !=s "WAMP LCDMisc Viz 0.2") {
				continue;
			}
			if (size($temp) < 20+576*2) {
				%spectrum = null;
				%waveform = null;
				%specHist = null;
				continue;
			}
			$data = $temp;
		}
		if (!size($data) || %lastReceived-%lastDrawn > 1) return;


		if (!size(%specHist)) {
			%specHist = list();
			%specHist[9] = null;
		}
		pop(%specHist,0);

		$spec = ParseBinaryInts($data, 20, 1, 1, 425);
		$specOld = %spectrum;
		for ($i=424; $i>=0; $i--) {
			$val = $spec[$i] * log(3+$i);
			$temp = $specOld[$i]*0.9 - 30;
			if ($val < $temp) $val = $temp;
			//if ($val > 255) $val = 255;
			$spec[$i] = $val;
		}
		%specHist[9] = %spectrum = $spec;

		%waveform = ParseBinaryInts($data, 20+576, 1, 0, 576);

		NeedRedraw();
	}

	function ChangeVolume($value) {
		%rawVolume = %rawVolume+($value*2.55)|0;
		$newVolume = (%rawVolume/2.55)|0;
		if ($newVolume != $value+%volume) {
			%rawVolume += $value/abs($value);
		}
		if (%rawVolume < 0) %rawVolume = 0;
		if (%rawVolume >255) %rawVolume = 255;
		PostMessage(%handle, 0x0400, %rawVolume, 122);

		%volume = (%rawVolume/2.55)|0;
		%changed = 3;
		NeedRedraw();
	}
	function ChangeBalance($value) {
		%rawBalance += $value;
		if (%rawBalance < -127) %rawBalance = -127;
		if (%rawBalance >127) %rawBalance = 127;

		PostMessage(%handle, 0x0400, %rawBalance, 123);

		%balance = %rawBalance*100/127;
		%changed = 3;
		NeedRedraw();
	}
};
