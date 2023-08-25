
#import <Views\View.c>
//#requires <framework\framework.c>
#requires <framework\header.c>
#requires <util\Graphics.c>
#requires <util\button.c>




struct Clock extends View, MenuHandler {
	var %background1, %background2;
	var %digits1, %digits2;
	var %aface;
	var %secFace;
	var %hrFace;
	var %minFace;
	var %capFace;
	var %dbase;
	var %predImages;
	var %loaded;
	var %predFont;

	
	function Clock () {
		%hasFocus = 0;
		%noDrawOnCounterUpdate = 0;
		%noDrawOnAudioChange = 1;

		%toolbarImage = LoadImage32("images/dclock1.png");
		%toolbarImageL = LoadImage32("images/dclock2.png");
		//%background1 = LoadImage32("images/clockBackground.png");	// sun rise
		//%background1 = LoadImage32("images/clock/backgroundr.png");	// sun rise
		//%background2 = LoadImage32("images/clock/backgroundn.png");	// night
		
		%aface = list();
		%secFace = list();
		%hrFace = list();
		%minFace = list();
		%capFace = list();
		%loaded = list();
		
		/*
		%dbase = LoadImage32("images/clock/base.png");
		%digits1 = list();	// time
		$i = 10;
		while ($i--)
			%digits1[$i] = LoadImage32("images/clock/digits1L/" +s $i +s ".png");

		%digits2 = list();	// date
		$i = 10;
		while ($i--)
			%digits2[$i] = LoadImage32("images/clock/digits2/" +s $i +s ".png");*/

	}

	function loadClock ($idx) {
		$i = $idx;
		
		//WriteLogLn($idx,1);

		if ($idx == 9){
			%aface[$i] = LoadImage32("images/clock/CrazyClock/face.png");
			//%secFace[$i] = LoadImage32("images/clock/CrazyClock/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/CrazyClock/hr.png");
			%minFace[$i] = LoadImage32("images/clock/CrazyClock/min.png");
			//%capFace[$i] = LoadImage32("images/clock/CrazyClock/cap.png");
			
		}else if ($idx == 0){
			%predImages = list();
			%predImages[CLK_IMG_PRED_BASE45] = LoadImage32("images/clock/Predator/base_45.png");
	    	%predImages[CLK_IMG_PRED_BASE90] = LoadImage32("images/clock/Predator/base_90.png");
	    	%predImages[CLK_IMG_PRED_BASE135] = LoadImage32("images/clock/Predator/base_135.png");
	    	%predImages[CLK_IMG_PRED_TOP45] = LoadImage32("images/clock/Predator/top_45.png");
	    	%predImages[CLK_IMG_PRED_TOP90] = LoadImage32("images/clock/Predator/top_90.png");
	    	%predImages[CLK_IMG_PRED_TOP135] = LoadImage32("images/clock/Predator/top_135.png");
	    	%predImages[CLK_IMG_PRED_BASE0] = LoadImage32("images/clock/Predator/base_0.png");		// 6
	    	%predImages[CLK_IMG_PRED_BASE18] = LoadImage32("images/clock/Predator/base_18.png");//(72d)
	    	%predImages[CLK_IMG_PRED_BASE243] = LoadImage32("images/clock/Predator/base_243.png");		// 8
	    	%predImages[CLK_IMG_PRED_TOP0] = LoadImage32("images/clock/Predator/top_0.png");
	    	%predImages[CLK_IMG_PRED_TOP18] = LoadImage32("images/clock/Predator/top_18.png");// 10
	    	%predImages[CLK_IMG_PRED_TOP243] = LoadImage32("images/clock/Predator/top_243.png");
	    	//%predImages[CLK_IMG_PRED_PREDLEFT] = LoadImage32("images/clock/Predator/predator5.png");	// 12
	    	%predImages[CLK_IMG_PRED_SKULL] = LoadImage32("images/clock/Predator/skull.png");
	    	//%predImages[CLK_IMG_PRED_PREDRIGHT] = LoadImage32("images/clock/Predator/predator6.png");
	    	
	    	//%predFont = Font("76London", 162, 0,0,0, CLEARTYPE_QUALITY);
	    	%predFont = Font("Racer", 151, 0,0,0, CLEARTYPE_QUALITY);

		}else if ($idx == 7){
			%aface[$i] = LoadImage32("images/clock/Silver/face.png");
			%secFace[$i] = LoadImage32("images/clock/Silver/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/Silver/hr.png");
			%minFace[$i] = LoadImage32("images/clock/Silver/min.png");
			%capFace[$i] = LoadImage32("images/clock/Silver/cap.png");
			
		}else if ($idx == 6){
		
			%aface[$i] = LoadImage32("images/clock/GreyFace/face.png");
			%secFace[$i] = LoadImage32("images/clock/GreyFace/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/GreyFace/hr.png");
			%minFace[$i] = LoadImage32("images/clock/GreyFace/min.png");
			%capFace[$i] = LoadImage32("images/clock/GreyFace/cap.png");
		
		}else if ($idx == 5){
			%aface[$i] = LoadImage32("images/clock/CyanTex/face.png");
			%secFace[$i] = LoadImage32("images/clock/CyanTex/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/CyanTex/hr.png");
			%minFace[$i] = LoadImage32("images/clock/CyanTex/min.png");
			%capFace[$i] = LoadImage32("images/clock/CyanTex/cap.png");

		}else if ($idx == 4){
			%aface[$i] = LoadImage32("images/clock/CyanGlow/face.png");
			%secFace[$i] = LoadImage32("images/clock/CyanGlow/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/CyanGlow/hr.png");
			%minFace[$i] = LoadImage32("images/clock/CyanGlow/min.png");
			%capFace[$i] = LoadImage32("images/clock/CyanGlow/cap.png");
		
		}else if ($idx == 3){
			%aface[$i] = LoadImage32("images/clock/RedGlow/face.png");
			%secFace[$i] = LoadImage32("images/clock/RedGlow/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/RedGlow/hr.png");
			%minFace[$i] = LoadImage32("images/clock/RedGlow/min.png");
			%capFace[$i] = LoadImage32("images/clock/RedGlow/cap.png");
				
		}else if ($idx == 2){
			%aface[$i] = LoadImage32("images/clock/Glowface/face.png");
			%secFace[$i] = LoadImage32("images/clock/Glowface/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/Glowface/hr.png");
			%minFace[$i] = LoadImage32("images/clock/Glowface/min.png");
			%capFace[$i] = LoadImage32("images/clock/Glowface/cap.png");

		}else if ($idx == 1){
			%aface[$i] = LoadImage32("images/clock/face.png");
			%secFace[$i] = LoadImage32("images/clock/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/hr.png");
			%minFace[$i] = LoadImage32("images/clock/min.png");
			
		}else if ($idx == 8){
			%aface[$i] = LoadImage32("images/clock/Butterfly/face.png");
			%secFace[$i] = LoadImage32("images/clock/Butterfly/sec.png");
			%hrFace[$i] = LoadImage32("images/clock/Butterfly/hr.png");
			%minFace[$i] = LoadImage32("images/clock/Butterfly/min.png");
			%capFace[$i] = LoadImage32("images/clock/Butterfly/cap.png");
		}
	}
	
	function touchDownFn ($event, $param, $tpos) {
		$tpoint = strsplit($tpos, ",", 0, 0, 0); // "x,y,dt,time"
		$x = $tpoint[0];
		$y = $tpoint[1];
		
		$boxW = 128;
		$boxH = 100;
		$x1 = (DWIDTH - $boxW) / 2;
		$y1 = (DHEIGHT - $boxH) / 2;
		$x2 = $x1 + $boxW;
		$y2 = $y1 + $boxH;

		if ($x > $x1 && $x < $x2){
			if ($y > $y1 && $y < $y2)
				%selectClockNext();
		}
	}
	
    function HandleEvent ($event, $param, $key) {
    	if ($event ==s "touchDown2"){
			return %touchDownFn(@$);
		}else  if ($event ==s "sbuiEventDKDown"){
			if ($key == SBUI_DK_10){
				%selectClockNext();
			}
			return 1;
		}
		return 0;
	}

	function width ($image){
		return $image.Size()[0];
	}

	function height ($image){
		return $image.Size()[1];
	}

	function %drawImageCentered ($img, $xc, $yc){
		DrawImage($img, $xc - (%width($img)/2), $yc - (%height($img)/2));
	}

	function renderPredator5Pt ($xc, $yc, $drawFlags, $num) {
		if ($drawFlags == 0) return;
		
		$drawBase = $drawFlags&1;
		$drawTop = $drawFlags&2;
		$draw1 = $num&1;
		$draw2 = $num&2;
		$draw4 = $num&4;
		$draw8 = $num&8;
		$draw16 = $num&16;
		
			
		if ($draw1){
			$offsetX = 69;
			$offsetY = 57;
			
			$base = %predImages[CLK_IMG_PRED_BASE0];
			$top = %predImages[CLK_IMG_PRED_TOP0];

			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc - $offsetY);	
		}
		
		if ($draw2){
			$offsetX = 8;
			$offsetY = 12;
			
			$base = %predImages[CLK_IMG_PRED_BASE18];
			$top = %predImages[CLK_IMG_PRED_TOP18];
			
			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc - $offsetY);	
		}
		
		if ($draw4){
			$offsetX = 32;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE135];
			$top = %predImages[CLK_IMG_PRED_TOP135];

			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc + $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc + $offsetY);	
		}
		
		if ($draw16){
			$offsetX = 130;
			$offsetY = 12;
			
			$base = %predImages[CLK_IMG_PRED_BASE243];
			$top = %predImages[CLK_IMG_PRED_TOP243];
			
			if ($drawBase) %drawImageCentered($base, $xc - $offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc - $offsetX, $yc - $offsetY);
		}
		
		if ($draw8){
			$offsetX = 106;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE45];
			$top = %predImages[CLK_IMG_PRED_TOP45];

			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc + $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc + $offsetY);		
		}		
	}

	function renderPredator6Pt ($xc, $yc, $drawFlags, $num) {
	
		if ($drawFlags == 0) return;
				
		$drawBase = $drawFlags&1;
		$drawTop = $drawFlags&2;
    	
		$draw1 = $num&1;
		$draw2 = $num&2;
		$draw4 = $num&4;
		$draw8 = $num&8;
		$draw16 = $num&16;
		$draw32 = $num&32;

		if ($draw1){
			$offsetX = 32;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE45];
			$top = %predImages[CLK_IMG_PRED_TOP45];
			
			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc - $offsetY);	
		}
    	
		if ($draw2){
			$offsetX = 0;
			$offsetY = 0;
			
			$base = %predImages[CLK_IMG_PRED_BASE90];
			$top = %predImages[CLK_IMG_PRED_TOP90];
			
			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc - $offsetY);	
		}
    	
		if ($draw4){
			$offsetX = 32;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE135];
			$top = %predImages[CLK_IMG_PRED_TOP135];
			
			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc + $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc + $offsetY);	
		}
		
		if ($draw32){
			$offsetX = 106;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE135];
			$top = %predImages[CLK_IMG_PRED_TOP135];
			
			if ($drawBase) %drawImageCentered($base, $xc - $offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc - $offsetX, $yc - $offsetY);	
		}
		
		if ($draw16){
			$offsetX = 138;
			$offsetY = 0;
			
			$base = %predImages[CLK_IMG_PRED_BASE90];
			$top = %predImages[CLK_IMG_PRED_TOP90];
			
			if ($drawBase) %drawImageCentered($base, $xc - $offsetX, $yc - $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc - $offsetX, $yc - $offsetY);
		}
    	
		if ($draw8){
			$offsetX = 106;
			$offsetY = 61;
			
			$base = %predImages[CLK_IMG_PRED_BASE45];
			$top = %predImages[CLK_IMG_PRED_TOP45];

			if ($drawBase) %drawImageCentered($base, $xc-$offsetX, $yc + $offsetY);
			if ($drawTop) %drawImageCentered($top, $xc-$offsetX, $yc + $offsetY);		
		}
	}

	function renderPredatorBase () {
		UseFont(%predFont);
		%drawImageCentered(%predImages[CLK_IMG_PRED_SKULL], DWIDTH/2, DHEIGHT/2);
		DisplayTextCenteredShadowed(FormatTime("HHNN"), DWIDTH/2, -16, RGB(255,0,0), 0x730204);
	}
	
	function renderPredatorClk () {
		
		$hr = FormatTime("H");
		$min = FormatTime("N");
		$sec = FormatTime("S");
		
		$xc = DWIDTH / 2;
		$yc = DHEIGHT / 2;
		
		$left = $xc / 3.0;
		$middle = $xc;
		$right = DWIDTH - $left;
		$y = $yc + 46;


		for ($r = 1; $r < 3; $r++){	// draw base layers first
    		$x = $left + 76;
    		%renderPredator5Pt($x, $y, $r&1, $hr);
    		%renderPredator5Pt($x, $y, $r&2, $hr);

    		$x = $middle + 65;
    		%renderPredator6Pt($x, $y, $r&1, $min);
    		%renderPredator6Pt($x, $y, $r&2, $min);
    
			$x = $right + 54;
    		%renderPredator6Pt($x, $y, $r&1, $sec);
	    	%renderPredator6Pt($x, $y, $r&2, $sec);
    	}
	}


	function DrawBackground (){
		//$hrs = FormatTime("HH");
		//if ($hrs >= 06 && $hrs <= 22)
			DrawImage(%background1, 0, 0);
		//else
			//DrawImage(%background2, 0, 0);
	}
	
	function DrawForegroundD ($_X){
		$baseW = %width(%dbase);
		$baseH = %height(%dbase);
		
		$x = (DWIDTH - $baseW)/2;
		$y = (DHEIGHT - $baseH)/2;
				
		if ($_X){
			$posX = $x;	// clock view
			$posY = $y - 52;
		}else{
			$posX = $x;	// clock overlay view
			$posY = $y;
		}

		$time = FormatTime("HHNN");
		$th1 = $time[0];
		$th2 = $time[1];
		$tm1 = $time[2];
		$tm2 = $time[3];

		DrawImage(%dbase, $posX, $posY);

		$posX += 48;
		$space = 16;
		$dw = %width(%digits1[$th1]);
		$dh = %height(%digits1[$th1]);
		$iy = (($baseH - $dh)/2) + $posY - 2;

		// hours
		DrawImage(%digits1[$th1], $posX, $iy); DrawImage(%digits1[$th2], $posX+$dw+$space, $iy);

		// minutes
		$posX += ($baseW/2) - $space;
		DrawImage(%digits1[$tm1], $posX, $iy); DrawImage(%digits1[$tm2], $posX+$dw+$space, $iy);
	}

	function DrawForegroundA ($clockIdx){
		$hr = FormatTime("H");
		$min = FormatTime("N");
		$sec = FormatTime("S");
		$s =  (6.0 * $sec + 0.5) * 0.0174532925195;	// 6 = (360.0 / 60.0)
		$m = (6.0 * $min) * 0.0174532925195;			// 30 = 360.0 / 12.0
		$h = (30.0 * ($hr +((1.0/60.0)*$min)) + 0.5) * 0.0174532925195;

		$face = %aface[$clockIdx];
		$sface = %secFace[$clockIdx];
		$hface = %hrFace[$clockIdx];
		$mface = %minFace[$clockIdx];
		$cface = %capFace[$clockIdx];

		$xc = DWIDTH/2.0;
		$yc = DHEIGHT/2.0;

		DrawImage($face, $xc - (%width($face) / 2), $yc - (%height($face) / 2));
		
		if ($clockIdx == 8){		// butterfly. recenter hands
			$xc = 204 + 30;// + 80;
			$yc = 252;// + 30;
		}
		
		DrawRotatedScaledImage($sface, $xc - %width($sface)/2, $yc - %height($sface)/2, $s);
		DrawRotatedScaledImage($hface, $xc - %width($hface)/2, $yc - %height($hface)/2, $h);
		DrawRotatedScaledImage($mface, $xc - %width($mface)/2, $yc - %height($mface)/2, $m);
		DrawImage($cface, $xc - %width($cface)/2, $yc - %height($cface)/2);
	}

	// this is also called from framework.c
	function DrawForeground ($clockType, $i){
		//if (razersb_GetSelectedDisplay() == SBUI_DISPLAY_KEYS)
		//	return;
			
		$idx = $clockType-1;
		if (%loaded[$idx] == 0){
			%loaded[$idx] = 1;
			%loadClock($idx);
		}
		
		if (drawClockIdx == 0)
			%renderPredatorClk();
		else
			%DrawForegroundA($idx);
	}
			
	function DrawDate ($x, $y){
		$dy = FormatTime("DD");
		$mn = FormatTime("MM");
		$yr = FormatTime("YY");
		
		// day
		DrawImage(%digits2[$dy[0]], $x, $y); DrawImage(%digits2[$dy[1]], $x+62, $y);

		// month
		$x += 132;
		DrawImage(%digits2[$mn[0]], $x, $y); DrawImage(%digits2[$mn[1]], $x+62, $y);

		// year
		$x += 132;
		DrawImage(%digits2[$yr[0]], $x, $y); DrawImage(%digits2[$yr[1]], $x+62, $y);
	}
	
	function Draw ($event, $param, $name, $res) {
		//if (razersb_GetSelectedDisplay() != SBUI_DISPLAY_KEYS){
			//%DrawBackground();
			ClearScreen();
			
			if (drawClockIdx == 0)
				%renderPredatorBase();
			
			%DrawForeground(1+drawClockIdx, 1);
			//%DrawDate(205, 372);
		//}
	}
};
