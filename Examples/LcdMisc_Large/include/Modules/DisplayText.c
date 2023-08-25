

function DisplayTextWithBack ($str, $x, $y){
	$width = TextSize($str)[0];
	$height = TextSize($str)[1]-3;

	ColorRect($x-1, $y+2, $x+$width, $y+$height, RGBA(20,20,20,180));
	DisplayText($str, $x, $y);
}

function DisplayTextCenteredWithBack ($str, $x, $y){
	$width = TextSize($str)[0];
	$height = TextSize($str)[1]-3;

	ColorRect($x-($width/2), $y+2, $x+($width/2)+1, $y+$height, RGBA(20,20,20,180));
	DisplayTextCentered($str, $x, $y);
}

function DisplayTextRightWithBack ($str, $x, $y){
	$width = TextSize($str)[0];
	$height = TextSize($str)[1]-3;

	ColorRect($x-$width+1, $y+2, $x+1, $y+$height, RGBA(20,20,20,180));
	DisplayTextRight($str, $x, $y);
}

function DisplayTextShadowed ($str, $x, $y, $colBg, $colFor){

	SetDrawColor($colBg);
	
	DisplayText($str, $x-1, $y-1);	// nw
	DisplayText($str, $x-1, $y+1);	// sw
	DisplayText($str, $x+1, $y+1);	// ne
	DisplayText($str, $x+1, $y-1);	// se
	
	SetDrawColor($colFor);
	DisplayText($str, $x, $y);
}

function DisplayTextCenteredShadowed ($str, $x, $y, $colBg, $colFor){

	SetDrawColor($colBg);
	DisplayTextCentered($str, $x-1, $y-1);
	DisplayTextCentered($str, $x-1, $y+1);
	DisplayTextCentered($str, $x+1, $y+1);
	DisplayTextCentered($str, $x+1, $y-1);
	
	SetDrawColor($colFor);
	DisplayTextCentered($str, $x, $y);
}

function DisplayTextRightShadowed ($str, $x, $y, $colBg, $colFor){

	SetDrawColor($colBg);
	DisplayTextRight($str, $x-1, $y-1);
	DisplayTextRight($str, $x-1, $y+1);
	DisplayTextRight($str, $x+1, $y+1);
	DisplayTextRight($str, $x+1, $y-1);
	
	SetDrawColor($colFor);
	DisplayTextRight($str, $x, $y);
}
