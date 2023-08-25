#import <Views\View.c>
#import <framework\framework.c>
#requires <framework\header.c>
#requires <util\Graphics.c>
#requires <util\button.c>



struct Stopwatch extends View {
	var %background1;
	var %digits1;
	var %aface;
	var %secFace;
	var %hrFace;
	var %minFace;
	var %dotFace;
	var %base;
	var %t0;
	var %updateTimerId;
	
	function Stopwatch () {
		%hasFocus = 0;
		%noDrawOnCounterUpdate = 0;
		%noDrawOnAudioChange = 1;

		%toolbarImage = LoadImage32("images/dclock1.png");
		%toolbarImageL = LoadImage32("images/dclock2.png");
		//%background1 = LoadImage32("images/clock/backgroundr.png");
		%base = LoadImage32("images/clock/digits9/base.png");

		
		%digits1 = list();	// time
		$i = 10;
		while ($i--)
			%digits1[$i] = LoadImage32("images/clock/digits12/" +s $i +s ".png");
			
		%t0 = GetTickCount()-(55*1000);
		//%updateTimerId = CreateFastTimer("timeUpdate", 200, 1, $this);
	}

	function timeUpdate (){
		NeedRedraw();
	}

	function DrawBackground (){
		//DrawImage(%background1, 0, 0);
	}
	
	function DrawForegroundD (){
		$posX = 10;
		$posY = 60;

		$t1 = GetTickCount()-%t0;

		$time = (($t1 / 1000 / 60)%60);
		$tm1 = $time/10;
		$tm2 = $time%10;
		
		$time = (($t1 / 1000)%60);
		$ts1 = $time/10;
		$ts2 = $time%10;

		$time = ($t1 % 1000)/10;
		$tms1 = $time/10;
		$tms2 = $time%10;
		
		DrawImage(%base, 0, 0);

		$gap = 90;
		// min
		DrawImage(%digits1[$tm1], $posX, $posY); DrawImage(%digits1[$tm2], $posX+$gap, $posY);

		// sec
		$posX += 195;
		DrawImage(%digits1[$ts1], $posX, $posY); DrawImage(%digits1[$ts2], $posX+$gap, $posY);

		// ms
		$posX += 195;
		DrawRotatedScaledImage(%digits1[$tms1], $posX, $posY+(%digits1[$tms1].Size()[1]/4), 0, 0.75, 0.75);
		//DrawRotatedScaledImage(%digits1[$tms2], $posX+$gap, $posY+(%digits1[$tms2].Size()[1]/4), 0, 0.75, 0.75);
	}

	// this is also called from framework.c
	function DrawForeground (){
		%DrawForegroundD();
	}

	function Draw ($event, $param, $name, $res) {
		%DrawBackground();
		%DrawForeground();
	}
};
