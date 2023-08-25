#import <Multimedia\MediaPlayerController.c>

dll32 Itunes "LCDMisc Itunes.dll" "Init";
dll64 Itunes "LCDMisc Itunes64.dll" "Init";
dll Itunes function GetItunesInfo "GetItunesInfo";
dll Itunes function ItunesNextTrack "ItunesNextTrack";
dll Itunes function ItunesPrevTrack "ItunesPrevTrack";
dll Itunes function ItunesPlay "ItunesPlay";
dll Itunes function ItunesPause "ItunesPause";
dll Itunes function ItunesPlayPause "ItunesPlayPause";
dll Itunes function ItunesMute "ItunesMute";
dll Itunes function ItunesStop "ItunesStop";
dll Itunes function ItunesChangeMode "ItunesChangeMode" (int);
dll Itunes function ItunesChangeVolume "ItunesChangeVolume" (int);


struct ItunesController extends MediaPlayerController {
	function ToggleMute() {
		ItunesMute();
		%UpdateAndDraw();
	}

	function ItunesController() {
		%mediaImage = LoadImage("Images\itunes.png");
		%bigMediaImage = LoadImage32("Images\itunes_big.png");
		%playerName = "iTunes";
		%state = -1;
		%noBalance = 1;
		%Update();
	}

	function Stop() {
		ItunesStop();
		%UpdateAndDraw();
	}

	function PlayPause() {
		ItunesPlayPause();
		%UpdateAndDraw();
	}

	function Pause() {
		ItunesPause();
		%UpdateAndDraw();
	}

	function Play() {
		ItunesPlay();
		%UpdateAndDraw();
	}

	function Next() {
		ItunesNextTrack();
		%UpdateAndDraw();
	}

	function Prev() {
		ItunesPrevTrack();
		%UpdateAndDraw();
	}

	function ChangeMode($v) {
		ItunesChangeMode($v);
		%UpdateAndDraw();
	}


	function Update() {
		$info = GetItunesInfo();
		%state = $info[0];
		%volume = $info[1];
		%muted = $info[2];
		%title = $info[3];
		%artist = $info[4];
		%duration = $info[5];
		%position = $info[6];
		%track = $info[7];
		%tracks = $info[8];
		%mode = $info[9];
	}

	function ChangeVolume($value) {
		ItunesChangeVolume(3*$value);
		%UpdateAndDraw();
	}

	function UpdateAndDraw() {
		%Update();
		NeedRedraw();
	}
};
