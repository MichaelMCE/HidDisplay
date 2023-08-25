
function FormatDuration ($duration) {

	$hours = $duration/3600.0;
	$mins = ($duration/60.0)%60.0;
	$secs = $duration%60.0;	
	
	if ($secs < 10) $secs = ":0" +s $secs;
	else  $secs = ":" +s $secs;
	
	if (!$mins) $secs = "00" +s $secs;
	else if ($mins < 10) $secs = "0" +s $mins +s $secs;
	else $secs = $mins +s $secs;
	
	if ($hours) $secs = FormatValue(($hours|0), 1, 0) +s ":" +s $secs;
	return $secs;
}
