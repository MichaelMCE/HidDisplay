

function GetMaxAudioState ($mixer, $src)
{
	// $vol = 0;
	$vistaVol = GetVistaMasterVolume();
	if (size($vistaVol) >= 2) {
		$vistaVol[0] *= 100;
		return $vistaVol;
	}
	
	$mute = 1;
	$emute = 1;
	
	for (;!IsNull($type = GetAudioType($mixer, $i));$i++) {
		if ($type != 4 && $type != 5) continue;
		$vol2 = GetAudioValue($mixer, $i, $src, -0x50030001, 1);
		$mute2 = GetAudioValue($mixer, $i, $src, -0x20010002, 1);
		if ($mute && !$vol2) continue;

		if (($vol2 >= $vol && $mute2 <= $emute) || ($emute && ($vol2 && !$mute2))) {
			$vol = $vol2;
			$emute = $mute = $mute2;
			if (!$vol) $emute = 1;
		}
	}
	
	return list($vol * (100.0/65535.0), $emute);
}
