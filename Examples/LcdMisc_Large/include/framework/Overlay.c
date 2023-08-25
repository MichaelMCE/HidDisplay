#import <constants.h>
#requires <util\graphics.c>
#requires <util\text.c>

struct MessageBoxOverlay {
	// Note:  Option 2 is cancel, and is automatically selected when a new message box is openned.
	var %font, %prompt, %option1, %option2, %fxn1, %fxn2, %obj1, %obj2, %arg1, %arg2, %selected, %handler;
	
	function MessageBoxOverlay ($_handler, $_prompt, $_option1, $_option2, $_fxn1, $_fxn2, $_obj1, $_obj2, $_arg1, $_arg2) {

		%font = Font("04b03", 8);
		%handler = $_handler;
		UseFont(%font);

		%prompt = Elipsisify($_prompt, 150);
		%option1 = $_option1;
		%option2 = $_option2;
		%fxn1 = $_fxn1;
		%fxn2 = $_fxn2;
		%obj1 = $_obj1;
		%obj2 = $_obj2;
		%arg1 = $_arg1;
		%arg2 = $_arg2;
		%selected = (size(%option2) != 0);
		$_handler.SetOverlay($this);
	}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 3) {
			if (size(%option2) && 3 != (G15GetButtonsState() & 3)) {
				if ($buttons & 1) {
					%selected ^= 1;
				}else {
					%selected ^= 1;
				}
				NeedRedraw();
			}
		}else if ($buttons & 8) {
			%Option2();
		}else if ($buttons & 4) {
			if (%selected == 1)
				%Option2();
			else
				%Option1();
		}
		return 1;
	}

	function KeyDown($event, $param, $modifiers, $vk, $string) {
		if ($vk == VK_ESCAPE || $vk == VK_DELETE || $vk == VK_BACK) {
			%G15ButtonDown(,,8);
		}
		if ($vk == VK_RETURN) {
			%G15ButtonDown(,,4);
		}
		if ($vk == VK_RIGHT || $vk == VK_DOWN) {
			%G15ButtonDown(,,2);
		}
		if ($vk == VK_LEFT || $vk == VK_UP) {
			%G15ButtonDown(,,1);
		}
		return 1;
	}

	function Option1() {
		%handler.RemoveOverlay($this);
		if (IsObject(%obj1))
			%obj1.call(%fxn1, %arg1);
		else
			call(%fxn1, %arg1);
		NeedRedraw();
	}

	function Option2() {
		// Not needed when called from Close(), but can't hurt.
		%handler.RemoveOverlay($this);
		if (IsObject(%obj2))
			%obj2.call(%fxn2, %arg2);
		else
			call(%fxn2, %arg2);
		NeedRedraw();
	}

	// Aciton when forced to close by the event handler.
	function Close() {
		NeedRedraw();
		%Option2();
	}

	function Draw ($event, $param, $name, $res) {
		//if ($res[0] <= 160)
		//	%drawMono(@$); // g15 
		//else
			%drawColour(@$);
	}
	
	function drawColour ($event, $param, $name, $res) {
		UseFont(bigHeaderFont);
		SetDrawColor(RGB(240,240,240));
		$tw = TextSize(%prompt)[0];
		$th = TextSize(%prompt)[1];
						
		$w = $res[0];
		$h = $res[1];
		$boxHeight = $h/2;
		$boxWidth = $w/2;
		if ($boxWidth <= $tw)
			$boxWidth = $tw + 20;
		$x = ($w/2) - ($boxWidth/2);
		$y = ($h/2) - ($boxHeight/2);

		DrawRectangle($x, $y, $x+$boxWidth, $y+$boxHeight, RGBA(255,255,255,160));
		DrawRectangleFilled($x+1, $y+1, $x+$boxWidth-1, $y+$boxHeight-1, RGBA(40,40,40,160));
		DisplayText(%prompt, $x+($boxWidth/2)-($tw/2), $y+($boxHeight/4)-($th/2));

		$tw = TextSize(%option1)[0];
		$th = TextSize(%option1)[1];
		$tx = $x+($boxWidth/4)-($tw/2);
		$ty = $y+($boxHeight-($boxHeight/4))-($th/2);
		DrawRectangle($tx-4, $ty-1, $tx+$tw+3, $ty+$th, RGBA(255,255,255,160));
		if (%selected == 0)
			DrawRectangleFilled($tx-3, $ty, $tx+$tw+2, $ty+$th-1, RGBA(180,180,180,120));
		DisplayText(%option1, $tx, $ty);

		$tw = TextSize(%option2)[0];
		$th = TextSize(%option2)[1];
		$tx = $x+($boxWidth-($boxWidth/4))-($tw/2);
		$ty = $y+($boxHeight-($boxHeight/4))-($th/2);
		DrawRectangle($tx-4, $ty-1, $tx+$tw+3, $ty+$th, RGBA(255,255,255,160));
		if (%selected == 1)
			DrawRectangleFilled($tx-3, $ty, $tx+$tw+2, $ty+$th-1, RGBA(180,180,180,120));
		DisplayText(%option2, $tx, $ty);
	}
	
	/*function drawMono ($event, $param, $name, $res) {
		
		UseFont(%font);
		$box2 = TextSize(%option1);
		$maxHeight = $height2 = $box2[1];
		$width2 = $box2[0];
		$box = TextSize(%prompt);
		$fullWidth = $width = $box[0];
		
		if (size(%option2)) {
			$box3 = TextSize(%option2);
			$height3 = $box3[1];
			if ($height3 > $maxHeight) $maxHeight = $height3;
			$width3 = $box3[0];
			$width4 = $width3;
			if ($width4 < $width2) $width4 = $width2;
			$width4 += 6;
			$temp = $width4 * 2;
			if ($temp > $fullWidth) $fullWidth = $temp;
		}else{
			if ($width < $width2 + 2) {
				$width = $width2 + 4;
			}
		}
		$height = $maxHeight + 4 + $box[1];
		$left = 80-$fullWidth/2-4;
		$top = 21-$height/2-4;
		$right = 80+$fullWidth/2+4;
		$bottom = 21+$height/2+4;
		DoubleBox($left, $top, $right, $bottom);
		DisplayText(%prompt, 80-($box[0]-1)/2, $top+2);

		if (size(%option2)) {
			DoubleBox(80-($width4 + $width2)/2-2, $bottom - $box2[1]-7, 80-($width4 - $width2)/2+2, $bottom - 3);
			DoubleBox(80+($width4 - $width3)/2-2, $bottom - $box3[1]-7, 80+($width4 + $width3)/2+2, $bottom - 3);
			DisplayText(%option1, 80-($width4 + $width2)/2+1, $bottom - $box2[1]-5);
			DisplayText(%option2, 80+($width4 - $width3)/2+1, $bottom - $box3[1]-5);
			if (%selected == 0) {
				InvertRect(80-($width4 + $width2)/2-1, $bottom - $box2[1]-6, 80-($width4 - $width2)/2+1, $bottom - 4);
			}
			else {
				InvertRect(80+($width4 - $width3)/2-1, $bottom - $box3[1]-6, 80+($width4 + $width3)/2+1, $bottom - 4);
			}
		}
	}*/
};


struct TimedOverlay {
	var %font, %prompt, %handler, %time, %timer;
	function TimedOverlay($_handler, $_prompt, $_time) {
		%time = $_time;
		%font = Font("04b03", 8);
		%handler = $_handler;
		$_handler.SetOverlay($this);
		%prompt = $_prompt;
		%timer = CreateFastTimer("Close", $_time, 0, $this);
	}

	function Close() {
		return %G15ButtonDown();
	}

	function G15ButtonDown() {
		if (%timer) {
			KillTimer(%timer);
			%timer = 0;
			%handler.RemoveOverlay($this);
			return 1;
		}
	}


	function Draw() {
		UseFont(%font);
		
		$box = TextSize(%prompt);
		$width = $box[0];
		$height = $box[1];
		$left = 80-$width/2-4;
		$top = 21-$height/2-1;
		$right = 80+$width/2+4;
		$bottom = 21+$height/2+1;
		
		DoubleBox($left, $top, $right, $bottom);
		DisplayText(%prompt, 80-($width-1)/2, $top+2);
	}
};

// Note: Depends on eventHandler global variable.
function MakeTimedOverlay($prompt, $time) {
	return TimedOverlay(eventHandler, $prompt, $time);
}

// Note: Depends on eventHandler global variable.
function MakeMessageBoxOverlay() {
	return TimedOverlay(eventHandler, @$);
}
