
function ImageLoad ($path)
{
	//WriteLogLn($path, 1);
	return LoadImage32(IMAGE_PATH +s $path);
}

function DrawRectangle ($x1, $y1, $x2, $y2, $colour){
	ColorLine($colour, $x1, $y1, $x1, $y2, $x2, $y2, $x2, $y1, $x1, $y1);
}

function DrawRectangleFilled ($x1, $y1, $x2, $y2, $colour){
	ColorRect(@$);
}
	
function DoubleBox ($left, $top, $right, $bottom) {
	DrawRect(@$);
	ClearRect($left+1, $top+1, $right-1, $bottom-1);
}

function DrawLineRotated ($x, $y, $xc, $yc, $a, $colour){
		
		$xr = $x*cos($a) - $y*sin($a);
		$yr = $x*sin($a) + $y*cos($a);
		ColorLine($colour, $xc, $yc, $xr+$xc, $yr+$yc);
}
