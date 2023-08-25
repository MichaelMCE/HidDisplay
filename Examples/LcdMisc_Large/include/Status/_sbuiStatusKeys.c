
#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <Modules\DisplayText.c>
#import <constants.h>





struct sbuiStatusKeys {
	var %backgroundKeys;
	var %KEY_WIDTH, %KEY_HEIGHT;
	var %switchView;
	var %timeFont;
	var %alist;
	
	function sbuiStatusKeys (){
		%timeFont = Font("76London", 152, 0,0,0, CLEARTYPE_QUALITY);
		%backgroundKeys = ImageLoad("itop.png");

		%KEY_WIDTH = 120;
		%KEY_HEIGHT = 120;
	}

	/*function Hide (){
		//%driveDelayShow = GetTickCount;
	}
		
	function Show (){
		//%driveDelayShow = GetTickCount();
	}*/
	
	
	function DisplayTextShadowedCentered ($str, $x, $y, $regionwidth, $colBg, $colFor){
		$width = TextSize($str)[0];
		$height = TextSize($str)[1]-3;

		$x += ($regionwidth - $width) / 2;
	
		SetDrawColor($colBg);
		DisplayText($str, $x-1, $y-1);	// nw
		DisplayText($str, $x-1, $y+1);	// sw
		DisplayText($str, $x+1, $y+1);	// ne
		DisplayText($str, $x+1, $y-1);	// se
	
		SetDrawColor($colFor);
		DisplayText($str, $x, $y);
	}

	function Draw ($event, $param, $name, $res) {
		SetDrawColor(RGBA(2,3,1,4));					// used by razersb.c to select target display
		DrawPixel(0,0);
		DrawImage(%backgroundKeys, 0, 128);			// Keys display is actually 800x480 with top 128 rows covered
		UseFont(%timeFont);

		$x = 0;
		$y = (DHEIGHT/2) - 106;
		$xpitch = %KEY_WIDTH + 48;
		$ypitch = %KEY_HEIGHT + 48;
		
		$time = FormatTime("HH:NN");
		$date = FormatTime("DDD");
		$day = FormatTime("DD");
		
		for ($i = 0; $i < 5; $i++){
			%DisplayTextShadowedCentered($time[$i], $x, $y, %KEY_WIDTH, 0xB328F612, RGBA(255,255,255,240));
			$x += $xpitch;
		}

		$x = 0;
		$y += $ypitch;
		
		for ($i = 0; $i < 3; $i++){
			%DisplayTextShadowedCentered($date[$i], $x, $y, %KEY_WIDTH, 0xB328C672, RGBA(255,255,255,240));
			$x += $xpitch;
		}

		for ($i = 0; $i < 2; $i++){
			%DisplayTextShadowedCentered($day[$i], $x, $y, %KEY_WIDTH, 0xB328C672, RGBA(255,255,255,240));
			$x += $xpitch;
		}
	}
};

