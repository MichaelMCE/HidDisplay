#import <Views\View.c>
#requires <util\Text.c>

struct ClipboardView extends View {
	var %data, %data2, %lines, %offset, %happy, %zoom, %x, %y;

	// Despite name, used when fail to read clipboard data to try again
	// every second.
	var %redrawTimer;

	var %bigFont, %smallFont;

	// Don't zoom when can't.
	var %minZoom;

	// formatting dimensions.  Changed for the V2.
	var %w, %h, %g19;

	function ClipboardView($_smallFont, $_bigFont) {
		%smallFont = $_smallFont;
		%bigFont = $_bigFont;
		%toolbarImage  = LoadImage32("Images\clipboard1.png");
		%toolbarImageL = LoadImage32("Images\clipboard2.png");

		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
    }


	function Draw($event, $param, $name, $res) {
		$farRight = $right = $res[0]-1;
		$bottom = $res[1]-1;
		$bpp = $res[2];
		if (!%happy) %Update();
		ClearScreen();
		if (IsString(%data)) {
			if ($bpp == 32)
				UseFont(%bigFont);
			else
				UseFont(%smallFont);
			DisplayText(%data,%w>300,%offset * GetFontHeight());
		}
		else if (type(%data) ==S "Image") {
			DrawImage(%data, %x, %y);
		}
		else if (type(%data) ==S "Image32") {
			if ($bpp > 1)
				DrawImage(%data, %x, %y);
			else {
				$s = %data.Size();
				$x2 = %x/2;//($s[0]-160)/2+80;
				$y2 = %y/2-39;//($s[1]-120)/2+21;
				DrawImage(%data2, $x2, $y2);
			}
		}
	}

	function UpdateDimensions() {
		if (480) {
			%w = 478;
			%h = 272;
			%g19 = 1;
		}
		else {
			%w = 160;
			%h = 43;
			%g19 = 0;
		}

	}

	function KeyDown($event, $param, $modifiers, $vk) {
		$vk = MediaFlip($vk);
		if (%hasFocus || IsNull($event)) {
			if (IsString(%data)) {
				if (%g19)
					UseFont(%bigFont);
				else
					UseFont(%smallFont);
				$height = GetFontHeight();
				// Volume up.
				if ($vk == 0xAE || $vk == VK_UP) {
					if (%offset < 0) {
						%offset ++;
					}
				}
				// Volume down.
				else if ($vk == 0xAF || $vk == VK_DOWN) {
					if (%offset > %h/$height-%lines) {
						%offset --;
					}
				}
			}
			else if (type(%data) ==S "Image" || type(%data) ==S "Image32") {
				// Volume up.
				if ($vk == 0xAE) {
					if (!%minZoom)
						%ReZoom(%zoom / 2.0);
				}
				// Volume down.
				else if ($vk == 0xAF) {
					if (%zoom != 1)
						%ReZoom(%zoom * 2.0);
				}
				else if ($vk == 0x25 || $vk == 0x27) {
					$width = %data.Size()[0];
					if ($width > %w) {
						%x += 0x26 - $vk;
						if (%x > 0) {
							%x = 0;
						}
						else if (%x < %w - $width) {
							%x = %w - $width;
						}
					}
				}
				else if ($vk == 0x26 || $vk == 0x28) {
					$height = %data.Size()[1];
					if ($height > %h) {
						%y += 0x27 - $vk;
						if (%y > 0) {
							%y = 0;
						}
						else if (%y < %h - $height) {
							%y = %h - $height;
						}
					}
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
			RegisterKeyEvent(0, 0xAE, 0xAF, 0x25, 0x26, 0x27, 0x28);
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
			UnregisterKeyEvent(0, 0xAE, 0xAF, 0x25, 0x26, 0x27, 0x28);
		}
	}

	function G15ButtonDown($event, $param, $button) {
		$button = FilterButton($button);
		if ($button & 0x3F) {
			if ($button == G15_LEFT) {
				%KeyDown(,,,VK_LEFT);
			}
			else if ($button == G15_RIGHT) {
				%KeyDown(,,,VK_RIGHT);
			}
			else if ($button == G15_UP) {
				%KeyDown(,,,VK_UP);
			}
			else if ($button == G15_DOWN) {
				%KeyDown(,,,VK_DOWN);
			}
			else if ($button == G15_CANCEL) {
				%Unfocus();
				NeedRedraw();
			}
			return 1;
		}
	}

	function ZoomImage($image, $newZoom) {
		$dim = $image.Size();
		$width = $dim[0];
		$height = $dim[1];

		%minZoom = 0;
		if ($newZoom * $width < %w && $newZoom*$height < %h) {
			$newZoom = %w*1.0/$width;
			if ($newZoom * $height > %h) {
				$newZoom = %h *1.0/$height;
			}
			// Safety margin.
			$newZoom *= 0.9999;
			%minZoom = 1;
		}
		if ($newZoom > 1 || $newZoom <= 0) $newZoom = 1;

		%zoom = $newZoom;
		if (%g19) {
			%data = $image.Zoom(%zoom);
			%data2 = %data.ToImage(0.5, -1);
		}
		else {
			%data2 = null;
			%data = %data2.ToImage(%zoom, -1);
		}
	}

	function ReZoom($newZoom) {
		%UpdateDimensions();
		$dim = %data.Size();
		$width  = $dim[0];
		$height = $dim[1];
		$temp = GetClipboardData();
		if (type($temp[1]) !=S "Image32") {
			return %Update();
		}
		%ZoomImage($temp[1], $newZoom);
		$dim = %data.Size();
		$width2  = $dim[0];
		$height2 = $dim[1];
		if ($width2 < %w) {
			%x = (%w-$width2)/2;
		}
		else {
			$center = %x - %w/2;
			%x = $width2*(1.0*$center)/$width + %w/2;
			if (%x > 0) %x = 0;
			if (%x < %w-1-$width2) %x = %w-1-$width2;
		}
		if ($height2 < %h) {
			%y = (%h-$height2)/2;
		}
		else {
			$center = %y - %h/2;
			%y = $height2*(1.0*$center)/$height + %h/2;
			if (%y > 0) %y = 0;
			if (%y < %h-1-$height2) %y = %h-1-$height2;
		}
	}

	function Update() {
		%offset = 0;
		%happy = 0;
		%data = null;
		%data2 = null;
		%UpdateDimensions();
		if (%g19)
			UseFont(%bigFont);
		else
			UseFont(%smallFont);
		for ($i=0; $i<3; $i++) {
			$temp = GetClipboardData();
			if (IsList($temp)) {
				%happy = 1;
				if (IsString($temp[1])) {
					%data = FormatText($temp[1], %w);
					%lines = TextSize(%data)[1]/GetFontHeight();
				}
				else if (type($temp[1]) ==S "Image32") {
					%ZoomImage($temp[1], 0);

					$dim = %data.Size();
					$width = $dim[0];
					$height = $dim[1];

					%x = (%w-$width)/2;
					%y = (%h-$height)/2;
				}
				break;
			}
			if (!IsNull($temp)) {
				%data = "[No Recognized Format]";
				if ($i > 1) break;
			}
			else {
				// Reduces chance of ending up with an error message.
				%data = "[Error Reading Clipboard]";
			}
			Sleep(10);
		}
		NeedRedraw();
		if (%happy) {
			if (%redrawTimer) {
				KillTimer(%redrawTimer);
				%redrawTimer = 0;
			}
		}
		else if (!%redrawTimer) {
			%redrawTimer = CreateTimer("NeedRedraw", 1);
		}
	}

	function Show() {
		SetEventHandler("ClipboardChange", "Update", $this);
	}

	function Hide() {
		if (%redrawTimer) {
			KillTimer(%redrawTimer);
			%redrawTimer = 0;
		}
		%happy = 0;
		%data = null;
		%data2 = null;
		SetEventHandler(, "ClipboardChange");
	}
};

