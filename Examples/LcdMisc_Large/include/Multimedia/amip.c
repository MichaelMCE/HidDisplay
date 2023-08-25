#import <Multimedia\MediaPlayerController.c>
#import <constants.h>

struct AMIPController extends MediaPlayerController {
	var %oldMode;
	function AMIPController($_url) {
		%mediaImage = LoadImage("Images\amip.png");
		%state = -1;
		%noBalance = 1;
		%Update();
		%execBuffer = list();
	}

	function Stop() {
		%ExecAndRefresh("control", "stop");
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
		%ExecAndRefresh("control", "pause");
	}

	function Play() {
		%ExecAndRefresh("control", "play");
	}

	function Next() {
		%ExecAndRefresh("control", ">");
	}

	function Prev() {
		%ExecAndRefresh("control", "<");
	}

	function ToggleMute() {
		//%UpdateAndRefresh(909);
	}

	var %execBuffer;

	function ExecAndRefresh($param1, $param2) {
		%execBuffer[size(%execBuffer)] = ($param1, $param2);
		if (!%updating) %Update();
		NeedRedraw();
	}

	function ChangeMode($v) {
		%oldMode = (%oldMode + 3 + $v)%3;
		if (%oldMode == 0) {
			%execBuffer[size(%execBuffer)] =("setrepeat", "off");
			%ExecAndRefresh("setshuffle", "off");
		}
		else if (%oldMode == 1) {
			%execBuffer[size(%execBuffer)] =("setrepeat", "on");
			%ExecAndRefresh("setshuffle", "off");
		}
		else {
			%execBuffer[size(%execBuffer)] =("setrepeat", "off");
			%ExecAndRefresh("setshuffle", "on");
		}
		NeedRedraw();
	}


	var %updating;
	function UpdateWait() {
		while ($i < size(%execBuffer)) {
			$forceUpdate = 1;
			DDEWait(XTYP_POKE, CF_TEXT, "mplug", %execBuffer[$i][0], %execBuffer[$i][1]);
			$i++;
		}
		%updating = 1;
		%execBuffer = list();
		%state = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_stat");
		if (!IsString(%state)) {
			%updating = 0;
			%state = -1;
			%title = null;
			return;
		}
		if (%state ==S "1") %state = 2;
		else if (%state ==S "3") %state = 1;
		else %state = 0;

		%volume = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_vol");
		if (!IsString(%volume)) {
			%volume = 0;
			%title = null;
			%updating = 0;
			return;
		}
		%volume = %volume * 100 / 255;

		$t = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_2");
		if (!IsString($t)) {
			%title = null;
			%updating = 0;
			return;
		}
		if (%title !=S $t) {
			%title = $t;
			%year = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_5");
			%artist = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_1");
			%duration = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_sl");
			if (IsNull(%duration)) %duration = -1;
			%duration += 0;
		}
		%position = 0 + DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_ps");
		%oldMode = 0;
		%mode = "Once";
		if (DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_shuffle") ==s "on") {
			%oldMode = 2;
			%mode = "Shuffle";
		}
		else if (DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_repeat") ==s "on") {
			%oldMode = 1;
			%mode = "Repeat";
		}
		%track = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_pos");
		%tracks = DDEWait(XTYP_REQUEST, CF_TEXT, "mplug", "var_ll");
		%updating = 0;
		if ($forceUpdate) NeedRedraw();
		if (size(%execBuffer)) %Update();
	}

	function Update() {
		// Can be called in response to a draw event.
		if (!%updating)
			SpawnThread("UpdateWait", $this);
	}

	function ChangeVolume($value) {
		if ($value > 0)
			%ExecAndRefresh("control", "vup");
		else
			%ExecAndRefresh("control", "vdwn");
		NeedRedraw();
		/*
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
		//*/
	}
};
