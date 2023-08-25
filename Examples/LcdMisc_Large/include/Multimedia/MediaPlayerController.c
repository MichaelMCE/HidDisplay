struct MediaPlayerController {
	var %bigMediaImage,
		%mediaImage,
		%state,
		%lastDrawn,

/* How muted/volume work doesn't matter.  Volume = old volume or volume = 0
 * when muted both work fine.
 */
		%volume,
		%muted,
		%balance,
		%position,
		%duration,
		%mode,
		%year,
		%title,
		%file,
		%artist,
		%track,
		%noBalance,
		%tracks,
		%playerName;
	// Just to prevent a warning.
	function MediaPlayerController() {
	}

	function ToggleMute() {
		if (%muted) {
			%volume = 0;
			%ChangeVolume(%muted);
		}
		else if (%volume) {
			%muted = %volume;
			%ChangeVolume(-500);
			%volume = %muted;
		}
		else {
			%muted = 1;
			NeedRedraw();
		}
	}

	function ChangeVolume() {
	}
	function Drawn() {
		%lastDrawn = Time();
	}
};
