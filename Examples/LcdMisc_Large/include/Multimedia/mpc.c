#import <Multimedia\MediaPlayerController.c>
#requires <util\http.c>

struct MPCController extends MediaPlayerController {
	var %statusUrl, %url;

	function MPCController($_url) {
		if (!size($_url)) {
			%url = GetString("URLs", "mpc");
		}
		else
			%url = $_url;
		%playerName = "MPC";
		%statusUrl = %url +s "status.html";
		//%mediaImage = LoadImage("Images\mpc.png");
		//%bigMediaImage = LoadImage32("Images\mpc_big.png");
		%bigMediaImage = "Images\mpc2.png";
		%state = -1;
		%noBalance = 1;
		%balance = 0;
		%Update();
	}

	function Stop() {
		%UpdateAndRefresh("890");
	}

	function PlayPause() {
		if (%state != 2) {
			%Play();
		}
		else {
			%Pause();
		}
	}

	function Pause() {
		%UpdateAndRefresh(888);
	}

	function Play() {
		%UpdateAndRefresh(887);
	}

	function Next() {
		%UpdateAndRefresh(921);
	}

	function Prev() {
		%UpdateAndRefresh(920);
	}

	function ToggleMute() {
		%UpdateAndRefresh(909);
	}

	function UpdateAndRefresh($param) {
		SpawnHttpRequest(%url +s "command.html?wm_command=" +s $param);
		%UpdateWait($param);
		NeedRedraw();
	}

	function ChangeMode($v) {
		// Don't think this can be done.
	}


	function UpdateWait() {
		$status = HttpGetWait(%statusUrl);
		if (IsNull($status) ||
			(!IsList($vals = RegExp($status, "^OnStatus\('(.*?) - Media Player Classic[^']*', '([^',]*)', (#*), '[^']*', (#*), '[^']*', (#*), (#*)\)$")) &&
			 !IsList($vals = RegExp($status, "^OnStatus\('()Media Player Classic[^']*', '([^',]*)', (#*), '[^']*', (#*), '[^']*', (#*), (#*)\)$")))) {
			%title = null;
			%state = -1;
			return;
		}
		%state = 0;


		%title = strreplace($vals[0][0], "\'", "'");
		if ($vals[1][0] ==s "Playing") {
			%state = 2;
		}
		else if ($vals[1][0] ==s "Paused") {
			%state = 1;
		}
		%position = $vals[2][0]/1000;
		%duration = $vals[3][0]/1000;
		%muted = $vals[4][0];
		%volume = $vals[5][0];
	}

	function Update() {
		// Can be called in response to a draw event.
		SpawnThread("UpdateWait", $this);
	}

	function ChangeVolume($value) {
		%volume += $value;
		if (%volume < 0) {
			%volume = 0;
		}
		else if (%volume > 100) {
			%volume = 100;
		}
		SpawnHttpRequest(%statusUrl +s "?command=volume&val=" +s $delta);
		%UpdateAndRefresh("-2&volume=" +s %volume);
		NeedRedraw();
	}
};
