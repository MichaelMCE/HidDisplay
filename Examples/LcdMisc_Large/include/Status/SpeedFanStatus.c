#requires <Shared Memory.c>

struct SpeedFanStatus {
	var %scroll;
	function SpeedFanStatus() {
		%scroll = -5;
    }

	function Draw() {
		ClearScreen();
		UseFont(0);
		$data = GetSpeedFanData();
		$m = size($data["temps"]);
		if (size($data["fans"])>$m) $m = size($data["fans"]);
		if (size($data["volts"])>$m) $m = size($data["volts"]);
		%scroll++;
		if (%scroll >= $m) %scroll = -4;

		$start = %scroll;
		if ($start > $m - 4) $start = $m-4;
		if ($start < 0) $start = 0;

		DisplayTextRight("#", 10, $y);
		DisplayTextRight("temps", 59, $y);
		DisplayTextRight("fans", 109, $y);
		DisplayTextRight("volts", 159, $y);
		InvertRect(0,0, 159,7);
		$y+=8;
		for ($i=$start; $i<$start+5; $i++) {
			DisplayTextRight($i, 10, $y);
			DisplayTextRight($data["temps"][$i], 59, $y);
			DisplayTextRight($data["fans"][$i], 109, $y);
			DisplayTextRight($data["volts"][$i], 159, $y);
			$y+=7;
		}
	}
};
