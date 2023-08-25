#import <Views\View.c>
#requires <Modules\DisplayText.c>
#requires <util\time.c>
#requires <util\Text.c>

function DrawSpectrum2($cache, $specHist, $left, $top, $right, $bottom) {
	// Significant speed optimization.
	while (!Equals($cache[5][0], $specHist[5]) && size($cache)) {
		pop($cache, 0);
	}

	$colors = list(0,0,0,0,0,RGB(0,255,0), RGB(0,220,0), RGB(0,170,0), RGB(0,120,0), RGB(255,0,0));
	for ($i=5; $i <= 9; $i++) {
		// Draws at offset instead of above/below.
		//$top2 = $top - 20*(9-$i);
		//$bottom2 = $bottom - 20*(9-$i);
		//$left2 = $left + 6*(9-$i);
		$top2 = $top;
		$bottom2 = $bottom;
		$left2 = $left;

		$h = ($bottom2 - $top)/768.0;
		$mul2 = (size($specHist[0])-1) / (1.0*($right-$left2));

		$waveform = $specHist[$i];

		if (!size($cache[$i][1])) {
			$points = list();

			for ($x=$right-$left2; $x>=0; $x--) {
				$val = $bottom2 - $h*$waveform[$mul2*$x];
				if ($val < $top2) $val = $top2;
				$points[2*$x] = $x + $left2;
				$points[2*$x+1] = $val;
			}
			$cache[$i] = list($waveform, $points);
		}
		ColorLine($colors[$i], @$cache[$i][1]);
	}
}

function DrawSpectrum($waveform, $left, $top, $right, $bottom) {
	$h = ($top - $bottom)/255.0;
	$mul2 = (size($waveform)-1) / (1.0*($right-$left));
	for ($x=$left; $x<=$right; $x++) {
		$val = $waveform[$mul2*($x-$left)];
		if ($val > 255) $val = 255;
		DrawRect($x, $bottom, $x, $bottom + $h*$val);
	}
}

function DrawWave($waveform, $left, $top, $right, $bottom) {
	$mul = ($bottom - $top)/255.0;
	$mid = ($top+$bottom)/2.0 + $mul * 0.5;
	$mul2 = (size($waveform)-1) / (1.0*($right-$left));
	for ($x=$left; $x<=$right; $x++) {
		DrawRect($x, $mid, $x, $mid + $mul*$waveform[$mul2*($x-$left)]);
	}
}

struct MediaView extends View {
	// %selected is a control index, %player is a player index.
	var %font, %titleFont, %bigFont, %bigTitleFont, %players, %player, %selected;
	var %titleScroller, %artistScroller;
	var %bigTitleScroller, %bigArtistScroller;
	var %scrollTimer;
	var %bump;
	var %specCache;
	var %background;
	var %icons, %mpIcons;
	var %toolbarVisible;
	var %mapping, %view;
	var %updateTimerId;
	var %SVMemFrameCt;
	
	function MediaView () {
		%specCache = list();
		%players = list();
		%mpIcons = Icons();

		for ($i=0; $i<size($); $i++) {
			if (!IsNull($[$i])){
				%players[size(%players)] = $[$i];
				
				$alist = list();
				$alist[0] = "NextPlayerChange";
				%mpIcons.addIcon(1, 700, 100, $[$i].bigMediaImage, "MediaControl", $alist);
			}
		}
		
		%toolbarImage = ImageLoad("mediaplayer1.png");
		%toolbarImageL = ImageLoad("mediaplayer2.png");
		%background = ImageLoad("mediaplayerBackground.png");
		
		%bigFont = Font("Lucida Sans Unicode", 30,0,0,0,CLEARTYPE_QUALITY);
		%bigTitleFont = Font("Lucida Sans Unicode", 32,0,0,0,CLEARTYPE_QUALITY);
		%titleFont = Font("Lucida Sans Unicode", 13);
		%font = Font("6px2bus", 6);
		%selected = -1;

		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
		%toolbarVisible = 0;

		%icons = Icons();
		%AddButton(111,260,  "media\prev.png", "MediaControl", VK_MEDIA_PREV_TRACK);
		%AddButton(261,260, "media\play.png", "MediaControl", VK_MEDIA_PLAY_PAUSE);
		%AddButton(411,260, "media\next.png", "MediaControl", VK_MEDIA_NEXT_TRACK);
		%AddButton(561,260, "media\stop.png", "MediaControl", VK_MEDIA_STOP);
		%AddButton(336,110,  "media\sound.png","MediaControl", VK_VOLUME_MUTE);

		%updateTimerId = CreateFastTimer("doSVMEMUpdate", 1050, 1, $this);
		%SVMemFrameCt = 0;
	}
	
	function doSVMEMUpdate (){
		if (!IsNull(%mapping)){
			$headerSize = %view.ReadInt(0, 4);
			if ($headerSize == 64){
				$count = %view.ReadInt(24, 4);
				if ($count > 1){ // don't display if VLC is just starting or shutting down
					if (%SVMemFrameCt != $count) // don't display if there isn't an update
						NeedRedraw();
					%SVMemFrameCt = $count;
					return;
				}
			}
		}else{
			%mapping = OpenFileMapping("VLC_SVIDEOPIPE");
			if (!IsNull(%mapping)){
				%view = %mapping.MapViewOfFile(0, 64+(960*540*4));
				if (%updateTimerId)
					ModifyFastTimer(%updateTimerId, 40); // we only want 25fps
			}
		}
		%SVMemFrameCt = 0;
	}
	
	function MediaControl ($event, $param, $argList) {
		$key = $argList[0];

		if ($key ==s "NextPlayerChange"){
			%player++;
			if (%player >= size(%players))
				%player = 0;
			NeedRedraw();
			return;
		}else if ($key == VK_MEDIA_NEXT_TRACK)
			%players[%player].Next();
		else if ($key == VK_MEDIA_PLAY_PAUSE)
			%players[%player].PlayPause();
		else if ($key == VK_MEDIA_STOP)
			%players[%player].Stop();
		else if ($key == VK_MEDIA_PREV_TRACK)
			%players[%player].Prev();
		else if ($key == VK_VOLUME_MUTE)
			%players[%player].ToggleMute();
		
		$alist = list();
		$alist[0] = "";
		PostEvent("ShowToolbar", 1, $alist);

	}
	
	function AddButton ($x, $y, $image, $event, $arg1, $arg2){
		$alist = list();
		$alist[0] = $arg1;
		$alist[1] = $arg2;
		%icons.addIcon(1, $x, $y, $image, $event, $alist);
	}

	function touchDownFn ($event, $param, $tpos) {
		$tpoint = strsplit($tpos, ",", 0, 0, 0); // "x,y,dt,time"
		$x = $tpoint[0];
		$y = $tpoint[1];
		
		if (!%mpIcons.runTouchedIcon($x, $y)){
			if (%toolbarVisible && !%hasFocus)
				return %icons.runTouchedIcon($x, $y);
		}
	}
		
	function Focus() {
		if (!%hasFocus) {
			//%selected = -1;
			%hasFocus = 0;
		}
	}

	function Toolbar ($state){
		%toolbarVisible = $state;
	}
	
	function ToolbarOff (){
		%toolbarVisible = 0;
	}
	
	function Bump() {
		%bump++;
		if (!(%bump%2)) {
			%titleScroller.Bump();
			%bigTitleScroller.Bump();
			NeedRedraw();
		}
		if (!(%bump%3)) {
			%artistScroller.Bump();
			%bigArtistScroller.Bump();
			NeedRedraw();
		}
	}

	function Show() {
		/*DynamicKeySetImage(SBUI_DK_8, SBUI_DK_UP, "dk+.png");
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_DOWN, "dk+Alt.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_UP, "dkSync.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_DOWN, "dkSyncAlt.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "dk-.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "dk-Alt.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "dkPlayer.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "dkPlayerAlt.png");*/
		
		if (!%scrollTimer) {
			%titleScroller = ManualScrollingText(,800,1,1,18);
			%artistScroller = ManualScrollingText(,800,1,1,12);
			%bigTitleScroller = ManualScrollingText(,800,1,1,28);
			%bigArtistScroller = ManualScrollingText(,800,1,1,22);
			%bump = 0;
			RegisterKeyEvent(0, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3);
			%players[%player].Update();
			// Can update $player twice, at times.  Not a huge deal.
			for ($i=0; $i < size(%players); $i++) {
				if ($i == %player) continue;
				%players[$i].Update();
				if (%players[$i].state > %players[%player].state)
					%player = $i;
			}
			%scrollTimer = CreateFastTimer("Bump", 60,,$this);
		}
	}

	function Hide() {
		/*DynamicKeySetImage(SBUI_DK_8, SBUI_DK_UP, "blank.png");
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_DOWN, "blank.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_UP, "blank.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_DOWN, "blank.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "blank.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "blank.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "blank.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "blank.png");*/
		
		if (%scrollTimer) {
			KillTimer(%scrollTimer);
			%scrollTimer = 0;
			// Not really needed... save a few bytes ram.
			%titleScroller = 0;
			%artistScroller = 0;
			%bigTitleScroller = 0;
			%bigArtistScroller = 0;
			UnregisterKeyEvent(0, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3);
		}

		if (%hasFocus){
			$alist = list();
			$alist[0] = G15_CANCEL;
			PostEvent("G15ButtonDown", 1, $alist);
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
		}
	}

	function G15ButtonDown ($event, $param, $buttons) {
		return 0;
		
		if (%players[%player].state < 0)
			%selected = -1;
				
		if (%selected == 0){
			$button = FilterButton($buttons);
			if ($button == G15_UP){
				%players[%player].Prev();
				return 1;
			}else if ($button == G15_DOWN){
				%players[%player].Next();
				return 1;
			}
		}
		if ($buttons & 0xF) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
				if (!($state & ($buttons^3))) {
					$p = %players[%player];
					$delta = 2*$buttons-3;
					if (%selected == -1) {
						%player = (%player + size(%players)+$delta) % size(%players);
					}else{
						%selected = (%selected + $delta + 3)%3;
						if ($p.noBalance && %selected == 2)
							%selected = (%selected + $delta + 3)%3;
					}
				}
			}else{
				if (!($state & ($buttons^12))) {
					if ($buttons & 8) {
						if (%selected >= 0) %selected = -1;
						else %Unfocus();
					}else {
						if (%selected < 0) %selected = 0;
					}
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function VolumeWheel ($change) {
		$p = %players[%player];
		if ($p.state < 0) %selected = -1;
		if (%selected == 0) {
			if ($change < 0) {
				$p.Prev();
			}else {
				$p.Next();
			}
		}else if (%selected == 1) {
			$p.ChangeMode($change);
		}else if (%selected == 2) {
			$p.ChangeBalance($change);
		}else {
			$p.ChangeVolume($change);
		}
	}
	
	function drawBargraph ($posx, $posy, $vwidth, $vheight, $pos){
		DrawRectangle($posx, $posy, $posx+$vwidth, $posy+$vheight, RGB(210,210,210));
		DrawRectangleFilled($posx+1, $posy+1, $posx+$vwidth-1, $posy+$vheight-1, RGBA(80,80,80,120));
		DrawRectangleFilled($posx+2, $posy+2, $posx+2+((($vwidth-4)/100.0)*$pos), $posy+$vheight-2, RGB(160,160,160));
	}

	function HandleEvent ($event, $param, $key) {
		if ($event ==s "touchDown2"){
			return %touchDownFn(@$);
		}
		if ($event ==s "sbuiEventDKDown"){
			//WriteLogLn($event +s " @ " +s $key, 1);

			if ($key == SBUI_DK_3){			// volume down
				%Focus();
				%KeyDown(0,0,0, 0xAE);
				//%Update();
				NeedRedraw();
			}else if ($key == SBUI_DK_8){	// volume up
				%Focus();
				%KeyDown(0,0,0, 0xAF);
				//%Update();
				NeedRedraw();
			}else if ($key == SBUI_DK_10){	// select
				%Focus();
				if (++%selected > 1)
				%selected = -1;
				//%Update();
				NeedRedraw();
			}else if ($key == SBUI_DK_6){	// player rotate
				%Focus();
				%player++;
				if (%player >= size(%players))
				%player = 0;
				//%Update();
				NeedRedraw();
	
			}

			return 1;

		}
		return 0;
	}


	
	function KeyDown ($event, $param, $modifiers, $vk) {
		if (!$modifiers) {
			if ($vk == 0xAE) {
				%VolumeWheel(-6);
			}else if ($vk == 0xAF) {
				%VolumeWheel(6);
			}else if ($vk == 0xB2) {
				%players[%player].Stop();
			}else if ($vk == 0xB3) {
				%players[%player].PlayPause();
			}else if ($vk == 0xB0) {
				%players[%player].Next();
			}else if ($vk == 0xB1) {
				%players[%player].Prev();
			}else if ($vk == 0xAD) {
				%players[%player].ToggleMute();
			}
			return 1;
		}
	}

	function DrawG15() {
		$p = %players[%player];
		$pos = 80 - 13 * size(%players)/2;
		for (; $i<size(%players); $i++) {
			DrawImage(%players[$i].mediaImage, $pos, 35);
			if (%player == $i) {
				InvertRect($pos, 35, $pos+11, 42);
			}
			$pos+=13;
		}

		UseFont(%font);
		DisplayText("Volume:", 1, 28);
		//DisplayText("Balance:", 69, 28);

		if ($p.state >= 0) {
			if (size($p.artist)) {
				%artistScroller.SetText($p.artist);
				%artistScroller.DisplayText(1, 0);

				UseFont(%titleFont);
				%titleScroller.SetText($p.title);
				%titleScroller.DisplayText(1, 6);
			}
			else {
				UseFont(%titleFont);
				%titleScroller.SetText($p.title);
				%titleScroller.DisplayText(1, 3);

			}
			UseFont(%font);

			ClearRect(119,0,159,20);
			DisplayText("of", 80, 21);
			DisplayTextRight(FormatDuration($p.position), 65, 21);
			if ($p.duration <= 0 && $p.position) {
				$duration = "N/A";
			}
			else {
				$duration = FormatDuration($p.duration);
			}
			DisplayTextRight($duration, 119, 21);

			if ($p.muted) {
				DisplayText("M", 37, 28);
			}

			DisplayTextRight($p.volume +s "%", 62, 28);
			DisplayTextRight($p.balance, 119, 28);

			DisplayTextCentered($p.mode, 140,8);

			if ($p.tracks) {
				if ($p.tracks < 10000)
					DisplayTextCentered($p.track +s "/" +s $p.tracks, 140, 15);
				else
					DisplayTextCentered($p.track, 140, 15);
			}
			if (%selected >= 0) {
				if (!%selected) {
					if (size($p.artist))
						InvertRect(0,6,118,18);
					else
						InvertRect(0,3,118,15);
				}
				else if (%selected == 1) {
					$s = TextSize($p.mode)[0];
					InvertRect(139 - $s/2,7,141 + ($s+1)/2,13);
				}
				else if (%selected == 2) {
					InvertRect(68,27,119,33);
				}
			}
		}else {
			%selected = -1;
		}

		if ($p.state < 0) {
			DisplayText($p.playerName +s " Not Running", 1, 14);
		}else {
			DisplayText(list("Stopped:", "Paused:", "Playing:")[$p.state], 1, 21);
		}

		DrawRect(120, 0, 120, 34);
		DrawRect(0, 34, 159, 34);

		DisplayTextCentered("Playmode", 140, 1);
		InvertRect(121, 6, 159);
		InvertRect(121, 14, 159, 20);
		InvertRect(0,20,119,26);

		if (size($p.spectrum)) {
			DrawSpectrum($p.spectrum, 121, 21, 159, 34);
		}
		else if (size($p.waveform)) {
			DrawWave($p.waveform, 121, 21, 159, 33);
		}
	}

	function DrawG19 () {
		$p = %players[%player];

		if (size($p.specHist)) {
			// Flash lights to music.  Could probably do much better.
			// G15SetBacklightColor(RGB($p.spectrum[20], $p.spectrum[160], $p.spectrum[300]));
			DrawSpectrum2(%specCache, $p.specHist, 0, 110, 800-1, 480-1);
		}else if (size($p.spectrum)) {
			DrawSpectrum($p.spectrum, 0, 110, 800-1, 480-1);
		}else if (size($p.waveform)) {
			DrawWave($p.waveform, 0, 110, 800-1, 480-1);
		}

		if (%hasFocus == 1) 
			ColorRect(0, 50, 800-1, 70, RGBA(80,80,80,160));
		else	
			ColorRect(0, 50, 800-1, 70, RGBA(40,40,40,120));

		%mpIcons.drawIcon(%player);

		UseFont(%bigFont);
		SetDrawColor(colorHighlightText);

		if ($p.state < 0) {
			DisplayText($p.playerName +s " Not Running", 1, 44);
		}else{
			//DisplayText(list("Stopped:", "Paused:", "Playing:")[$p.state], 1, 44);
		}

		SetDrawColor(colorText);
		if ($p.state >= 0) {
			if (!%selected) {
				ColorRect(0, 0, DWIDTH-1, 49, RGBA(80,80,80,160));
				SetDrawColor(colorHighlightText);
			}
			if (size($p.artist)) {
				%bigArtistScroller.SetText($p.artist);
				%bigArtistScroller.DisplayText(1, 0);

				UseFont(%bigTitleFont);
				%bigTitleScroller.SetText($p.title);
				%bigTitleScroller.DisplayText(1, 20);
			}else {
				UseFont(%bigTitleFont);
				%bigTitleScroller.SetText($p.title);
				%bigTitleScroller.DisplayText(1, 10);
			}
		
			UseFont(%bigFont);
		
			if ($p.state != 0) {
				SetDrawColor(colorHighlightText);
				
				DisplayTextCentered(FormatDuration($p.position), 45, 46);
				%drawBargraph(100, 52, DWIDTH-200, 16, (100.0/$p.duration)*$p.position);
				if ($p.duration <= 0 && $p.position) {
					$duration = "N/A";
				}else {
					$duration = FormatDuration($p.duration);
				}
				DisplayTextCentered($duration, DWIDTH-50, 46);
			}
		
			//ColorRect(1, 75, 390, 69+28, RGBA(20,100,20,80));
			if (%hasFocus == 1) 
				ColorRect(0, 75, 800-1, 98, RGBA(80,80,80,160));
			else	
				ColorRect(0, 75, 800-1, 98, RGBA(40,40,40,120));
				
			if ($p.tracks)
				DisplayTextRight($p.track +s "/" +s $p.tracks, 790, 72);
			SetDrawColor(colorText);

			$v = "Volume: " +s $p.volume +s "%";
			DisplayText($v, 4, 72);
			if ($p.muted) {
				DrawLine(4, 88, TextSize($v)[0], 88);
			}

			//if (%selected == 2) {
			//	SetDrawColor(colorHighlightText);
			//	//ColorRect(187, 68, 222, 84, RGBA(80,80,80,160));
			//	DisplayText("Balance: " +s $p.balance, 120, 66);
			//	SetDrawColor(colorText);
			//}else {
			//	DisplayText("Balance: " +s $p.balance, 120, 66);
			//}

			if (%selected == 1) {
				$s = TextSize($p.mode)[0];
				ColorRect(398 - $s/2, 75, 402 + ($s+1)/2, 98, RGBA(80,80,80,160));
				SetDrawColor(colorHighlightText);
			}
			DisplayTextCentered($p.mode, 400,72);
		}else {
			%selected = -1;
		}

		if (%SVMemFrameCt > 1){ // don't display if VLC is just starting or shutting down
			$headerSize = %view.ReadInt(0, 4);
			$res = %view.ReadInts(28, 2, 1, 2);
			$bpp = %view.ReadInt(32, 1);
			$image = %view.LoadImage($res[0], $res[1], $bpp, $headerSize);
			$x = (DWIDTH - $res[0]) / 2;
			$y = (DHEIGHT - $res[1]) - 4;
			DrawImage($image, $x, $y);
		}
		if (%toolbarVisible)
			%icons.drawIcons();
	}

	function Draw ($event, $param, $name, $res) {
		/* Too fast, but fits in with my scrolling rate. ~2 per second
		 * gives reasonable second update rate.
		 */
		$p = %players[%player];
		// Tell it that it's been drawn, so knows when it can kill itself if it has
		// an open socket and is idle.
		$p.Drawn();

		if (%bump >= 6) {
			$p.Update();
			%bump %= 6;
		}

		if ($res[0] >= 320){
			DrawImage(%background, 0, 0);
			%DrawG19();
		}else{
			ClearScreen();
			%DrawG15();
		}
	}
};
