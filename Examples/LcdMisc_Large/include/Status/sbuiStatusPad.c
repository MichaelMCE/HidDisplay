
#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <util\Audio.c>
#requires <Modules\DisplayText.c>
#import <constants.h>
#import <Modules\Graph.c>
//#requires <Modules\DriveInfo.c>
#requires <Modules\CounterManager.c>
#requires <Shared Memory.c>
#requires <util\button.c>
//#requires <util\tcx.c>




struct sbuiStatusPad extends MenuHandler {
	var %font, %modeFont, %bpmFont, %freqFont, %counters, %vlcFont, %bwFont;
	//var %driveDelayShow;
	var %background, %origBackground;
	var %vlcTitle, %vlcTitleTimer, %vlcWindowHandle;
	var %vlcTitlePosX, %vlcTitlePosY;
	var %trackTitle, %trackAlbum, %trackNumber, %trackLength, %trackArtist, %trackAddedTime;
	var %trackArtworkMRL;
	var %trackArtworkImage;
	var %hrmIsConnected;
	var %tcxRoute;
	var %serial;
	var %cpuClock;
	var %cpuTotal;  /* total cores as representated in windows task manager */
	var %switchView;
	var %graphDisplay, %colours;

	
	function sbuiStatusPad (){
		%counters = GetCounterManager();
		%font = Font("Arial", 64,0,0,0,CLEARTYPE_QUALITY);
		%vlcFont = Font("Arial", 64,0,0,0,CLEARTYPE_QUALITY);
		%bpmFont = Font("Insane Hours",60,0,0,0,CLEARTYPE_QUALITY);
		%freqFont = Font("Insane Hours",72,0,0,0,CLEARTYPE_QUALITY);
		%bwFont = Font("76London",76,0,0,0,CLEARTYPE_QUALITY);
		//%modeFont = Font("Insane Hours", 32,0,0,0,CLEARTYPE_QUALITY);
		%hrmIsConnected = 0;
		//%driveDelayShow = GetTickCount();
		%origBackground = ImageLoad("statusBackgroundPad.png");
		%background = %origBackground.Copy();
		%cpuTotal = GetNumberOfCPUCores(); // if not available, use GetString("Processor", "Cores", "8");
		//%cpuTotal = 24;
		%cpuClock = list();

		for ($i = 0; $i < %cpuTotal; $i++){
			%cpuClock[$i] = PerformanceCounter("ProcessorPerformance", "PPM_Processor_"+s $i, "frequency", 1);
			%cpuClock[$i].SetAutoUpdate(1);

			/*%cpuClock[$i].Update();
			$clk = %cpuClock[$i].GetValue();
			WriteLogLn(FormatValue($i) +s " - " +s FormatValue($clk),1);*/
		}

		// (const char *Port, const int baud, const int Bitsize, const int StopBits*10, const int Parity)
		//%serial = ComPort("COM1", 9600, 8, 10, 0);

		%vlcTitleTimer = CreateTimer("GetVLCWindowTitle", 10, 1, $this);
		%GetVLCSMeta();

		// check include/util/tcx.c more
		//%tcxRoute = Tcx(/*"filename.tcx"*/, "C:/Users/Administrator/AppData/Roaming/Garmin/Devices/<yourdeviceid>/History/");
		//%tcxRoute.update();
		
		%colours = list(RGB(0,224,0), RGB(255,0,0), RGB(0,255,255), RGB(255,255,0), RGB(255,0,255), RGB(240,240,240), RGB(244,100,50), RGB(0,0,244));
		%graphDisplay = 0;
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
			if ($y > $y1 && $y < $y2){
				if (++%graphDisplay > 7)
					%graphDisplay = 0;
				NeedRedraw();
			}
		}
	}
	
    function HandleEvent ($event, $param, $key) {
		if ($event ==s "touchDown2"){
			return %touchDownFn(@$);
		}else if ($event ==s "sbuiEventDKDown"){
			//WriteLogLn($event +s " @ " +s $key, 1);

			if ($key == SBUI_DK_3){
				if (++%graphDisplay > 7)
					%graphDisplay = 0;
				NeedRedraw();
			}else if ($key == SBUI_DK_9){
				%setClockState(SHOW_CLOCKA);
				
			}else if ($key == SBUI_DK_10){
				%selectClockNext();
			}
			return 1;
			
		}else if ($event ==s "sbuiEventDKUp"){
			//WriteLogLn($event +s " - " +s $key, 1);
			return 1;
			
		}else if ($event ==s "touchDownEvent"){
			//WriteLogLn($event +s " - " +s $key, 1);
			return 1;
		}
		return 0;
	}
	
	function GetVLCSMeta (){
		//%serial.Write("123456teSt");
		//%serial.Write(65);
		/*$alist = list(15);
		$alist[0] = 65;
		$alist[1] = 2.37;
		$alist[2] = 77;
		$alist[3] = "123456teSt";
		%serial.Write($alist);*/

		
		%trackTitle = trackGetMetaStr(MTAG_Title);
		if (IsNull(%trackTitle)) return;
		
		%trackAlbum = trackGetMetaStr(MTAG_Album);
		%trackNumber = trackGetMetaInt(MTAG_TrackNumber);
		%trackArtist = trackGetMetaStr(MTAG_Artist);
		//%trackAddedTime = trackGetMetaInt(MTAG_ADDTIME);
		%trackLength = trackGetMetaStr(MTAG_LENGTH);
		$trackArtwork = trackGetMetaStr(MTAG_ArtworkPath);
		
		if ($trackArtwork !=s %trackArtworkMRL){
			%trackArtworkMRL = $trackArtwork;
			%trackArtworkImage = LoadImage32(%trackArtworkMRL);
			
			//WriteLogLn(%trackArtworkMRL, 1);

			if (!IsNull(%trackArtworkImage)){
				
				$height = %trackArtworkImage.Size()[1];
				%trackArtworkImage = %trackArtworkImage.Zoom(268.0/$height);	// 170 = space available below volume bar
				%trackArtworkImage = %trackArtworkImage.Fade(0.7, 0);
				//%trackArtworkImage = %trackArtworkImage.Flip(3);
				//%trackArtworkImage = %trackArtworkImage.Pixelize(4);
				//%trackArtworkImage = %trackArtworkImage.Blur(4, 0/*blurOp - todo*/);
				
				/*
				image32.Fade(level, mode). mode 0 = rgba, mode 1 = rgb, mode 2 = alpha channel only
				image32.Blur(radius). radius = 1,2,3,etc..
				image32.Flip(v_or_h). v_or_h = 1 for vertical, 2 for horizontal, 3 = flips both (identical to a 180degree rotate)
				image32.Pixelize(pixelizationSize);	// factor
				image32.Copy();	// clone the image
				image32.Clear(colour);	// clear image. colour defaults to 0xFF000000 if not supplied
				image32.Draw(desImage32, desX, desY, srcX, srcY, srcWidth, srcHeight);	//draw one image32 on to another. does not return a copy
				*/

				%background = %origBackground.Copy();
				%trackArtworkImage.Draw(%background, (DWIDTH - %trackArtworkImage.Size()[0]) - 6, (DHEIGHT - %trackArtworkImage.Size()[1])-6);
				//%trackArtworkImage.Draw(%background, 0,0);
				
			}
		}
		
		//%tcxRoute.update();
	}
	
	function GetVLCWindowTitle (){
		%GetVLCSMeta();

		%hrmIsConnected = abs(GetTickCount() - (hrmGetValue64("HRM_bpmTime"))) < 10000; // 10000 = 10 seconds. don't display if hrm isn't connected or updating
		//$hlist = hrmGetInfo();
		//WriteLogLn($hlist[6],1);
		//%hrmIsConnected = (GetTickCount() - $hlist[6]) < 10000; // 10000 = 10 seconds. don't display if hrm isn't connected or updating
		if (%hrmIsConnected){
			if (IsNull(%bpmFont))
				//%bpmFont = Font("Insane Hours",60,0,0,0,CLEARTYPE_QUALITY);
				//%bpmFont = Font("Shuttle-Extended",96,0,0,0,CLEARTYPE_QUALITY);
				%bpmFont = Font("SF Square Head",96,0,0,0,CLEARTYPE_QUALITY);
				
			if (IsNull(%modeFont))
				%modeFont = Font("Insane Hours", 32,0,0,0,CLEARTYPE_QUALITY);
			return;	// HRM display and vlc title share the same display area
		}
		
		// get the title..
		%vlcWindowHandle = vlcGetWindowHandle();
		if (!%vlcWindowHandle) return;

		%vlcTitle = GetWindowText(%vlcWindowHandle);
		if (%vlcTitle ==s "VLC media player"){		// VLC is running but not playing anything
			%vlcWindowHandle = 0;
			return;
		}

		// remove file extension if possible
		$breakdown = strsplit(%vlcTitle, ".", 0, 200, );
		if (size($breakdown) == 2){		// if there is only a before and after, then this must be an extension
			%vlcTitle = $breakdown[0];
		}else{
			$breakdown = strsplit(%vlcTitle, " - ", 0, 200, ); // remove the vlc program name which follows the hyphen
			if (size($breakdown) > 2){
				$total = size($breakdown);
				%vlcTitle = "";
								
				for ($i = 0; $i < $total-1; $i++){
					%vlcTitle = %vlcTitle +s $breakdown[$i];
					if ($i < $total-2) %vlcTitle = %vlcTitle +s " - ";
				}
			}else{
				%vlcTitle = $breakdown[0];
			}
		}
		
		%vlcTitlePosY = 88;
		UseFont(%font);
		$twidth = TextSize(%vlcTitle)[0];
		$minX = 280;
		
		if ($twidth >= DWIDTH - $minX)
			%vlcTitlePosX = $minX;
		else
			%vlcTitlePosX = DWIDTH-2 - $twidth;
	}
		
	function drawVolume ($posx, $posy, $vwidth, $vheight, $vol){
		DrawRectangle($posx, $posy, $posx+$vwidth, $posy+$vheight, RGB(20,255,20));
		DrawRectangleFilled($posx+1, $posy+1, $posx+$vwidth-1, $posy+$vheight-1, RGBA(200,60,0,80));
		DrawRectangleFilled($posx+2, $posy+2, $posx+2+((($vwidth-4)/100.0)*$vol), $posy+$vheight-2, RGB(2.55*$vol,255-(2.00*$vol),255-(2.55*$vol)));
	}

	function Hide (){
		//%driveDelayShow = GetTickCount;
		StopTimer(%vlcTitleTimer);
		//razersb_ClearDisplayKeys();
	}
		
	function Show (){
		//%driveDelayShow = GetTickCount();
		StartTimer(%vlcTitleTimer);
	}
	
	function drawCPUFreq ($width){
		$maxClk = 0;
		
		for ($i = 0; $i < %cpuTotal; $i++){
			$clk = %cpuClock[$i].GetValue();
			if ($clk > $maxClk) $maxClk = $clk;
		}

		UseFont(%freqFont);
		DisplayTextRightShadowed($maxClk, $width-8, 88, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));
		UseFont(%font);	
	}
	
	function drawBandwidthTitle (){
		$titleX = (DWIDTH/2) - 88;
		$titleY = 96;
		
		SetDrawColor(RGB(0,255,0));
		DisplayTextShadowed("D:"+s FormatSize(%counters.down, 1,0,1), $titleX, $titleY, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,255));
		SetDrawColor(RGB(255,0,0));
		DisplayTextRightShadowed("U:"+s FormatSize(%counters.up, 1,0,1), DWIDTH-3, $titleY, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,255));
		SetDrawColor(RGB(0,0,0));
	}
	
	function drawGraph ($graph, $width, $height){
		if ($graph == 0){
			$graphY1 = 204;
					
			DrawMultiGraph(%counters.cpuGraph, 1, $height-4, ($width/2)-2, $graphY1, RGBA(0,0,0,0),list(0,2));
			DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), ($width/2)+2, $height-4, $width-2, $graphY1, RGBA(0,0,0,0),list(0,2));
			
		}else if ($graph == 1){
			%drawCPUFreq($width);
			
			DrawMultiGraph(%counters.cpuGraph, 1, $height-1, $width-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if ($graph == 2){
			UseFont(%bwFont);
			%drawBandwidthTitle();
			
			DrawMultiGraph(%counters.cpuGraph, 1, $height-1, ($width/2)-2, 164, RGBA(20,20,20,120),list(0,2));
			DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), ($width/2)+2, $height, $width-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if ($graph == 3){
			UseFont(%bwFont);
			%drawBandwidthTitle();
			
			DrawMultiGraph(list(%counters.downGraph, %counters.upGraph), 1, $height-1, $width-2, 164, RGBA(20,20,20,120),list(0,2));
			
		}else if ($graph == 4){
			//$cpuTotal = size(%counters.cpuGraph);
			%drawCPUFreq($width);

			$gap = 2;
			$width = ($width- $gap * %cpuTotal) / %cpuTotal;
			$x = 4;
			
			for ($i = 0; $i < %cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawGraph($graph, $x, $height-1, $x+$width-1, 164, RGBA(20,20,20,120), %colours[$i&0x07], list(0,2));
				$x += $width + $gap;
			}
		}else if ($graph == 5){
			%drawCPUFreq($width);
			DrawMultiGraph(%counters.cpuGraph, 4, $height-1, ($width/2)-2, 164, RGBA(20,20,20,120),list(0,2));
			
			//$cpuTotal = size(%counters.cpuGraph);
			$gap = 3;
			$x = ($width/2)+10;
			$width = (($width-$x)- $gap * %cpuTotal) / %cpuTotal;
			
			for ($i = 0; $i < %cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawBarGraph($graph, $x, $height-1, $x+$width-1, 164, RGBA(20,20,20,120), %colours[$i&0x07], list(1,0,1));
				$x += $width + $gap;
			}
		}else if ($graph == 6){
			%drawCPUFreq($width);

			$gap = 2;
			$width = ($width - $gap * (%cpuTotal/2)) / (%cpuTotal/2);
			$x = 5;
			$y = $height - 4;
			$height = 154;
			
			for ($i = 0; $i < %cpuTotal/2; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawGraph($graph, $x, $y, $x+$width-1, $y-$height, RGBA(20,20,20,120), %colours[$i&0x07], list(0,2));
				$x += $width + $gap;
			}
			
			$x = 5;
			$y -= $height + 5;
			
			for ($i = %cpuTotal/2; $i < %cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawGraph($graph, $x, $y, $x+$width-1, $y-$height, RGBA(20,20,20,120), %colours[$i&0x07], list(0,2));
				$x += $width + $gap;
			}
		}else if ($graph == 7){
			%drawCPUFreq($width);

			$gap = 2;
			$width = ($width - $gap * (%cpuTotal/2)) / (%cpuTotal/2);
			$x = 5;
			$y = $height - 4;
			$height = 154;
						
			for ($i = 0; $i < %cpuTotal/2; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawBarGraph($graph, $x, $y, $x+$width-1, $y-$height, RGBA(20,20,20,120), %colours[$i&0x07], list(1,0,1));
				$x += $width + $gap;
			}
			
			$x = 5;
			$y -= $height + 5;
			
			for ($i = %cpuTotal/2; $i < %cpuTotal; $i++){
				$graph = %counters.cpuGraph[$i];
				DrawBarGraph($graph, $x, $y, $x+$width-1, $y-$height, RGBA(20,20,20,120), %colours[$i&0x07], list(1,0,1));
				$x += $width + $gap;
			}
		}
	}
	
	
	function Draw ($event, $param, $name, $res) {
		$width  = $res[0];
		$height = $res[1];
		
		DrawImage(%background, 0, 0);
		DisplayHeader($res);
		UseFont(%font);
	
		$memory = GetMemoryStatus();
		$x = 1;
		$y = 62;
		$vpitch = 48;
		
		DisplayTextShadowed("CPU: "+s FormatValue(%counters.cpuData["_Total"], 0, 1)+s "%", $x, $y, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		$y += $vpitch;
		DisplayTextShadowed("Mem: "+s FormatValue($memory[1]/1024.0/1024.0), $x, $y, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
		
		//DisplayTextCenteredShadowed("VM: "+s FormatValue($memory[3]/1024.0/1024.0), 290, 50, RGBA(0x0,0x0,0x0,255), RGBA(0xFF,0x00,0x00,255));

		if (%graphDisplay){
			%drawGraph(%graphDisplay, $width, $height);
			return;
		}

		if (%hrmIsConnected){
			UseFont(%bpmFont);
			DisplayTextCenteredShadowed(FormatValue(hrmGetValue("HRM_bpm")), ($width/2)-16, 0/*20*/, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));
			UseFont(%modeFont);
			DisplayTextShadowed(FormatValue(hrmGetValue("HRM_mode")), ($width/2), 40, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));
			UseFont(%font);

		}else if (%vlcWindowHandle){
			DisplayTextShadowed(%vlcTitle, %vlcTitlePosX, %vlcTitlePosY, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
			
		}else{
			%drawCPUFreq($width);
		}


		%drawVolume(10, 170, $width-20, 32, GetMaxAudioState(0,0)[0]);
		DrawImage(%trackArtworkImage, ($width - %trackArtworkImage.Size()[0])-4, ($height - %trackArtworkImage.Size()[1])-4);

		if (!IsNull(%trackTitle)){
			UseFont(%vlcFont);
			$x = 1;
			$y = 200;
			$vpitch = 56;
			
			DisplayTextShadowed(%trackNumber +s ":" +s %trackTitle, $x, $y, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
			$y += $vpitch;
			DisplayTextShadowed(%trackLength, $x, $y, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
			$y += $vpitch;
			DisplayTextShadowed(%trackArtist, $x, $y, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,255));
			$y += $vpitch;
			DisplayTextShadowed(%trackAlbum, $x, $y, RGBA(0xf6,0x7f,0x0,156), RGBA(255,255,255,240));
		}


		//$ati = GetATTData();
		/*$speed = GetSpeedFanData();
		DisplayTextCenteredWithBack("CPU:" +s $speed["temps"][1]+s "°c", 160, 23);
		DisplayTextCenteredWithBack("Fan:" +s $speed["fans"][0], 250, 23);
		DisplayTextCenteredWithBack("GPU:" +s $speed["temps"][6]+s "°c", 160, 42);
		//DisplayTextCenteredWithBack("Core:" +s $speed["temps"][8]+s "°c", 250, 42);
		DisplayTextCenteredWithBack("HD0:" +s $speed["temps"][0]+s "°c", 160, 60);
		DisplayTextCenteredWithBack("HD1:" +s $speed["temps"][5]+s "°c", 250, 60);
		*/

		// delay showing the drive info at startup as this function is quite slow when first executed.
		//if (GetTickCount() - %driveDelayShow > 2000)
		//	DisplayDriveInfo(315, 23, 460, 84, -4);

		//DisplayTextCentered(GetRemoteIP(), 240, 77);

		/*SetDrawColor(RGB(0,255,0));
		DisplayTextWithBack("Dn:"+s FormatSize(%counters.down, 1,0,1), 250, 95);
		SetDrawColor(RGB(255,0,0));
		DisplayTextWithBack("Up:"+s FormatSize(%counters.up, 1,0,1), 400, 95);
		SetDrawColor(RGB(0,0,0));*/
		
		//DrawMultiGraph(%counters.cpuGraph, 1, DHEIGHT-2, DWIDTH-2, 100, RGBA(0,0,0,0),list(0,2));
		
		
		%drawGraph(%graphDisplay, $width, $height);
		
		//%tcxRoute.render($res[0]+0.0, $res[1]+0.0);
	}
};

