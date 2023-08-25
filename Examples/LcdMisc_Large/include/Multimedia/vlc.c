#import <Multimedia\MediaPlayerController.c>
#requires <util\XML.c>
#requires <util\http.c>

function GetVLCXMLName($xml) {
	$s = SimpleXML($xml[1], list("name"));
	return $s;
}

struct VLCController extends MediaPlayerController {
	var %rawVolume, %modes, %rmode, %statusUrl, %playlistUrl, %needTitle;

	function VLCController($url) {
		if (!size($_url)) {
			$url = GetString("URLs", "vlc");
		}
		%statusUrl = $url +s "status.xml";
		%playlistUrl = $url +s "playlist.xml";
		//%mediaImage = ImageLoad("vlc.png");
		//%bigMediaImage = ImageLoad("vlc2.png");
		%bigMediaImage = "vlc2.png";
		%state = -1;
		%noBalance = 1;
		%Update();
		%playerName = "VLC";
	}

	// For the love of god...
	function GetTrack() {
		$playlist = HttpGetWait(%playlistUrl);

		$index = RegExp($playlist, "(current=|"current|")")[0][1];
		%track = %tracks = 0;
		while (1) {
			$pos = RegExp($playlist, "<leaf()", $pos);
			if (!IsList($pos)) break;
			$pos = $pos[0][1];
			%tracks ++;
			// Works, as track is 1-based index.
			if ($pos < $index) %track++;
		}
		if (%needTitle) {
			$temp = RegExp($playlist, "name=|"(.*?)|"", $index);
			if (IsList($temp)) {
				%title = RegExp($temp[0][0], "([^\\/]*)$")[0][0];
			}
		}
	}

	function Stop() {
		%UpdateAndRefresh("?command=pl_stop");
	}

	// Incredibly evil, horrible, lame VLC interface requirement.
	// VLC has a play button.  I cannot mimick it.  I can only load
	// a needlessly complicated playlist file, find a random id in it,
	// and play.  Why, dear lord, WHY?  Might be away around it with
	// VLC's wonderfully documented language...And I though winamp
	// was bad...
	function GetEvilId() {
		$junk = RegExp(HttpGetWait(%playlistUrl), "id=|"([0-9]*?)|"\W*current", 0);
		return $junk[0][0];
	}

	function PlayPause() {
		if (%state == 0) $id = "&id=" +s %GetEvilId();
		%UpdateAndRefresh("?command=pl_pause&" +s $id);
	}

	function Pause() {
		if (%state != 2) {
			%UpdateAndRefresh("?command=pl_pause&id=0");
		}
	}

	function Play() {
		%UpdateAndRefresh("?command=pl_play&id=" +s %GetEvilId());
	}

	function Next() {
		%UpdateAndRefresh("?command=pl_next");
	}

	function Prev() {
		%UpdateAndRefresh("?command=pl_previous");
	}

	function UpdateAndRefresh($param) {
		// Significantly speeds response.
		SpawnHttpRequest(%statusUrl +s $param);
		%UpdateWait($param);
		NeedRedraw();
	}

	function ChangeMode($v) {
		$nmode = (%rmode + 4 + $v)%4;
		if ($nmode == 0) {
			$nmodes = 0;
			%mode = "Once";
		}
		else if ($nmode == 1) {
			$nmodes = 4;
			%mode = "Repeat";
		}
		else if ($nmode == 2) {
			$nmodes = 2;
			%mode = "Loop";
		}
		else if ($nmode == 3) {
			// Dunno if you need both...Specs, dangit, I want specs!
			$nmodes = 3;
			%mode = "Shuffle";
		}
		$delta = $nmodes ^ %modes;
		if ($delta & 4) {
			SpawnHttpRequest(%statusUrl +s "?command=pl_repeat");
		}
		if ($delta & 2) {
			SpawnHttpRequest(%statusUrl +s "?command=pl_loop");
		}
		if ($delta & 1) {
			SpawnHttpRequest(%statusUrl +s "?command=pl_random");
		}

		%modes = $nmodes;
		%rmode = $nmode;
		NeedRedraw();
	}


	function UpdateWait() {
		$xml = HttpGetWait(%statusUrl);
		if (IsNull($xml)) {
			%title = null;
			%artist = null;
			%year = null;
			%state = -1;
			return;
		}
		// RegExps aren't fast, but they're faster than my XML parser, because of
		// memory allocation overhead.  Simpler, too.
		%balance = 0;
		%rawVolume = RegExp($xml, "<volume>(#*)</volume>")[0][0];
		%volume = %rawVolume*100/511;

		if (%muted) {
			if (%volume) %muted = 0;
			else %volume = %muted;
		}

		%position = RegExp($xml, "<time>(#*)</time>")[0][0];
		$oldDuration = %duration;
		%duration = RegExp($xml, "<length>(#*)</length>")[0][0];
		if (%duration !=S $oldDuration) {
			%needTitle = 1;
			$needUpdate = 1;
		}
		%state = RegExp($xml, "<state>([^<]*)</state>")[0][0];
		%modes = 0;
		%mode = "Once";
		%rmode = 0;
		if (RegExp($xml, "<loop>(#)</loop>")[0][0] == 1) {
			%mode = "Loop";
			%modes |= 2;
			%rmode = 2;
		}
		if (RegExp($xml, "<random>(#)</random>")[0][0] == 1) {
			%mode = "Shuffle";
			%modes |= 1;
			%rmode = 3;
		}
		if (RegExp($xml, "<repeat>(#)</repeat>")[0][0] == 1) {
			%mode = "Repeat";
			%modes |= 4;
			%rmode = 1;
		}

		if (%state ==s "stop") {
			%state = 0;
		}
		else if (%state ==s "paused") {
			%state = 1;
		}
		else %state = 2;

		//Don't need XML parser as XML is escaped in CDATA (Maybe new to VLC v1.0)
		%title = RegExp($xml, "<title><!\[CDATA\[(.*?)\]\]></title>")[0][0];
		%artist = RegExp($xml, "<artist><!\[CDATA\[(.*?)\]\]></artist>")[0][0];
		%year = RegExp($xml, "<date><!\[CDATA\[(.*?)\]\]></date>")[0][0];


		if ($needUpdate) {
			NeedRedraw();
			SpawnThread("GetTrack", $this);
		}
	}

	function Update() {
		// Can be called in response to a draw event.
		SpawnThread("UpdateWait", $this);
	}

	function ChangeVolume($value) {
		// Kinda convoluted, but it works and is very responsive.
		$old = %rawVolume;
		%rawVolume += $value * 5;
		$dir = $value/abs($value);
		while (%rawVolume * 100/511 != %volume + $value) {
			%rawVolume += $dir;
		}
		if (%rawVolume < 0) {
			%rawVolume = 0;
		}
		else if (%rawVolume > 511) {
			%rawVolume = 511;
		}
		%volume = %rawVolume*100/511;
		$delta = %rawVolume - $old;
		if ($delta) {
			if ($delta > 0) {
				$delta = "+" +s  $delta;
			}
			SpawnHttpRequest(%statusUrl +s "?command=volume&val=" +s $delta);
			NeedRedraw();
		}
	}
};