// Needs SafeSmart() and CachedCall()
#requires <Modules\DisplayText.c>
#requires <util\CPUSaver.c>
#requires <util\Graphics.c>

#import <Constants.h>

function DriveInfoSub() {
	/* If the autodetection doesn't display what you want,
	 * uncomment the next line and enter the letters of the
	 * drives you want displayed.
	 */

	// return list("C:", "D:", "E:", "F:");

	$drives = DriveList();
	for ($i=size($drives)-1; $i >= 0; $i--) {
		if ($drives[$i] <s "C") {
			pop($drives, $i);
			continue;
		}
		$info = VolumeInformation($drives[$i]);
		if ($info.driveType != DRIVE_FIXED)
			pop($drives, $i);
		else {
			$drives[$i] = substring($drives[$i], 0, 2);
		}
	}
	return $drives;
}

function DriveInfo() {
	$drives = CachedCall("DriveInfoSub", "", 29);
	$out = list();
	for ($i = size($drives)-1; $i >= 0; $i--) {
		$d = $drives[$i];
		$out[$i] = list(
			$d,
			CachedCall("GetDiskSpaceFree", $d, 9 + $i)/1024.0/1024.0/1024.0,
			CachedCall("SafeSmart", $d, 43+2*$i)
		);
		$t = $out[$i][2][SMART_ATTRIBUTE_TEMPERATURE];

		if ($t.value >= 20 && $t.value <= 150)
			$temp = $t.value;
		else if ($t.normalized >= 20 && $t.normalized <= 150)
			$temp = $t.normalized;
		else $temp = 0;
		$out[$i][3] = $temp;
	}
	return $out;
}

// $y2 currently ignored, assumes default font is active
// and has twice its height to work with.
function DisplayDriveInfo ($x1, $y1, $x2, $y2, $delta) {
	$drives = DriveInfo();
	$y1--;
	$fh = GetFontHeight() + $delta;
	$width1 = TextSize("C: ")[0];
	$width2 = TextSize("00.0")[0];
	$width3 = $width1+$width2;

	if (size($drives) <= 2 && $fh <= 7) {
		// This branch currently only supports 2 drives and low res screens.
		$d = $drives[0];
		$temp = $d[3];
		if (size($drives) == 2) {
			DisplayText($d[0], $x1, $y1);
			DisplayTextRight(FormatValue($d[1],0,1), $x1 + 40, $y1);
			if ($temp > 0 && $temp < 200) {
				if ($x2 - $x1 >= 63) {
					DisplayTextRight($temp, $x1 + 60, $y1);
					DoubleBox($x1 + 61, $y1+1, $x1 + 63, $y1+3);
				}
			}
			$d = $drives[1];
			$temp = $d[3];
		}
		$y1 = $y2-5;
		DisplayText($d[0], $x1, $y1);
		DisplayTextRight(FormatValue($d[1],0,1), $x1 + 40, $y1);
		if ($temp > 0 && $temp < 200) {
			if ($x2 - $x1 >= 63) {
				DisplayTextRight($temp, $x1 + 60, $y1);
				DoubleBox($x1 + 61, $y1+1, $x1 + 63, $y1+3);
			}
		}
	}else{
		/*if (size($drives == 3)) {
			$dx = ($x2 - $x1)/3.0;
			$x1 += $dx/2; 
			for ($i=0; $i<3 && $i<size($drives); $i++) {
				$x = $x1 + ($i*$dx);
				$d = $drives[$i];
				$string = ($d[1] | 0) +s "";
				if (size($string) != 3) {
					if (size($string) > 3) {
						$string = FormatValue($d[1]/1024,0,1) +s "T";
					}
					else {
						$string = FormatValue($d[1],0,1);
					}
				}
				$s2 = $d[0];
				$temp = $d[3];
				$temp = 40;
				if ($temp > 0 && $temp < 200) {
					$s2 +=s $temp;
					DisplayTextCentered($s2, $x, $y1);
				}
				else {
					DisplayTextCentered($s2, $x, $y1);
				}
				DisplayTextCentered($string, $x, $y1+6);
			}
		}
		else {//*/
			for ($i=0; $i<size($drives); $i++) {
				$x = $x1;
				$y = $y1 + $fh*($i/2);
				if ($y + $fh > $y2+2) break;
				$d = $drives[$i];
				if ($i & 1) {
					$x = $x2 - $width3;
				}
				$string = ($d[1] | 0) +s "";
				if (size($string) != 3) {
					if (size($string) > 3) {
						$string = FormatValue($d[1]/1024,0,1) +s "T";
					}
					else {
						$string = FormatValue($d[1],0,1);
					}
				}
				DisplayTextWithBack($d[0] +s " " +s $string, $x, $y);
			}
		//}
	}
}