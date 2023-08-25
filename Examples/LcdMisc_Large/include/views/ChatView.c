#import <Views\View.c>
#requires <util\Text.c>

#requires <Modules\Chat.c>

struct ChatView extends View {
	var %font, %boldFont;
	// Only used for viewing channel list.
	var %services, %selected, %search, %searchTime;

	// Currently displayed channel.  0/null for displaying list.
	var %active, %activeChannel;

	function ChatView () {
		%services = list();
		for ($i=0; $i<size($); $i++) {
			if (!IsNull($[$i])) %services[size(%services)] = $[$i];
		}

		%toolbarImage = LoadImage("Images\Chat.png");
		%font = Font("04b03", 8);
		%boldFont = Font("04b03", 8, 0, 1);

		// Could redraw only when I have to, but too much effort to do so in all such situations.
		//%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
	}
// 11100
//100100
// 11100
// 11100
//   100
// 11000
	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
			// Test was chosen so it matches ReplaceUsers().
			RegisterKeyEvent(3<<16, VK_ESCAPE);
			if (!IsNull(%activeChannel)) %activeChannel.chat.Focus();
			else {
				RegisterKeyEvent(3<<16, VK_VOLUME_DOWN, VK_VOLUME_UP, VK_RETURN, VK_PRIOR, VK_NEXT, VK_HOME, VK_END);
				RegisterKeyRange(3<<16, VK_LEFT, VK_DOWN);
				RegisterKeyRange(3<<16, VK_0, VK_9);
				RegisterKeyRange(3<<16, VK_A, VK_Z);
				RegisterKeyRange(3<<16, 0xBA, 0xC0);
				RegisterKeyRange(3<<16, 0xDB, 0xDE);
			}
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
			UnregisterKeyEvent(3<<16, VK_ESCAPE);
			if (!IsNull(%activeChannel)) %activeChannel.chat.Unfocus();
			else {
				UnregisterKeyEvent(3<<16, VK_VOLUME_DOWN, VK_VOLUME_UP, VK_RETURN, VK_PRIOR, VK_NEXT, VK_HOME, VK_END);
				UnregisterKeyRange(3<<16, VK_LEFT, VK_DOWN);
				UnregisterKeyRange(3<<16, VK_0, VK_9);
				UnregisterKeyRange(3<<16, VK_A, VK_Z);
				UnregisterKeyRange(3<<16, 0xBA, 0xC0);
				UnregisterKeyRange(3<<16, 0xDB, 0xDE);
			}
		}
	}

	function Show() {
		%Resync(%GetChannels());
		%activeChannel.chat.ShowName();
	}

	//function Hide() {
	//}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 0xF) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
				if (!($state & ($buttons^3))) {
					$channels = %GetChannels();
					%Resync($channels);
					$u = %active;
					if ($buttons == 1) {
						if (!$u) $u = size($channels);
						while ($u > 0) {
							$u--;
							if (!IsNull($channels[$u].chat)) break;
						}
					}
					else {
						while ($u < size($channels)) {
							$u++;
							if (!IsNull($channels[$u].chat)) break;
						}
						$u %= size($channels);
						%SetActive($u);
					}
					%active = $u;
					%SetActive($channels);
				}
			}
			else {
				if (!($state & ($buttons^12))) {
					if ($buttons & 8) {
						%Unfocus();
					}
					else {
						%KeyDown(, , , VK_RETURN);
					}
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function KeyDown($event, $param, $modifiers, $vk, $string) {
		$channels = %GetChannels();
		if ($vk == VK_ESCAPE) {
			%Unfocus();
			return 1;
		}
		if (IsNull(%activeChannel)) {
			$vk = MediaFlip($vk);
			NeedRedraw();
			%Resync($channels);
			if ($vk == VK_RETURN) {
				if (!IsNull($channels[%selected].chat)) {
					%active = %selected;
					%SetActive($channels);
				}
				%search = "";
				return 1;
			}
			if (($modifiers & 2) && $vk ==s VK_W) {
				%search = "";
				%active = %selected;
				%SetActive($channels);
				%GetActiveService($channels).KeyDown(%activeChannel, @$);;
				%active = 0;
				%SetActive($channels);
				return 1;
			}
			if (size($string)) {
				if (GetTickCount() - %searchTime >= 1400) {
					%search = "";
				}
				%searchTime = GetTickCount();
				%search +=s $string;
				$index = %selected;
				while (1) {
					if (substring($channels[$index].name, 0, size(%search)) ==s %search) break;
					$index = ($index + 1) % size($channels);
					if ($index == %selected) break;
				}
				%selected = $index;
				return 1;
			}
			%search = "";
			if ($vk == VK_VOLUME_UP || $vk == VK_DOWN) {
				%selected++;
				if (%selected >= size($channels)) {
					%selected = 1;
				}
			}
			else if ($vk == VK_VOLUME_DOWN || $vk == VK_UP) {
				%selected--;
				if (%selected < 1) {
					%selected = size($channels)-1;
				}
			}
			else if ($vk == VK_HOME) {
				%selected = 1;
			}
			else if ($vk == VK_END) {
				%selected = size($channels)-1;
			}
			else if ($vk == VK_PRIOR) {
				if (%selected >= 1) {
					%selected = size($channels)-1;
				}
				else {
					%selected-=5;
					if (%selected < 1) %selected = 1;
				}
			}
			else if ($vk == VK_NEXT) {
				if (%selected == size($channels)-1) {
					%selected = 1;
				}
				else {
					%selected += 5;
					if (%selected >= size($channels)) %selected = size($channels)-1;
				}
			}
			return 1;
		}
		else {
			return %GetActiveService($channels).KeyDown(%activeChannel, @$);
		}
	}

	function GetActiveService($channels) {
		%Resync($channels);
		if (!%active) return;
		$index = %active-1;
		for ($i=0; $i<size(%services); $i++) {
			if (size(%services[$i].channels) > $index) {
				return %services[$i];
			}
			$index -= size(%services[$i].channels);
		}
	}

	function GetChannels() {
		$channels = list(null);
		for ($i=0; $i<size(%services); $i++) {
			$channels = (@$channels, @%services[$i].channels);
		}
		return $channels;
	}

	// Syncs up %active and %activeChannel.  *Only* use this function to do so.
	function SetActive($channels) {
		if (!size($channels)) $channels = %GetChannels();
		if (!Equals(%activeChannel, $channels[%active])) {
			if (%hasFocus) {
				%Unfocus();
				%activeChannel.visible = 0;
				%activeChannel = $channels[%active];
				%activeChannel.visible = 1;
				%activeChannel.chat.ShowName();
				%Focus();
			}
			NeedRedraw();
		}
	}

	// Fixes %active if channel list has changed since last redraw.
	// Need to do before redisplay and before changing %active when navigating
	// left/right.
	function Resync($channels) {
		if (!Equals(%activeChannel, $channels[%active])) {
			%active = size($channels) - 1;
			while (%active > 0) {
				if (!IsNull(%activeChannel) && Equals(%activeChannel, $channels[%active])) break;
				%active--;
			}
			%SetActive($channels);
		}
	}

	function Draw() {
		ClearScreen();
		$channels = %GetChannels();
		%Resync($channels);
		if (%activeChannel.visible == 0 && !IsNull(%activeChannel.visible)) {
			for ($i=0; $i<size($channels); $i++) {
				if (Equals(%activeChannel.visible, $channels[$i])) {
					%active = $i;
					%SetActive($channels);
					break;
				}
			}
		}
		if (!%active) {
			if (%selected < 1) %selected = 1;
			if (%selected >= size($channels))
				%selected = size($channels) - 1;
			$i = %selected - ((%selected-1)%5);
			$pos = 0;
			UseFont(%font);
			$fh = GetFontHeight();
			while ($pos < 42 && $i < size($channels)) {
				if (IsNull($channels[$i].chat)) {
					UseFont(%font);
				}
				else {
					UseFont(%boldFont);
				}
				$x = 1 + 5 * $channels[$i].indent;
				DisplayText($channels[$i].name, $x, $pos);
				UseFont(%font);
				if (%hasFocus && %selected == $i) {
					InvertRect(0,$pos, 159, $pos+$fh-1);
				}
				$pos += $fh;
				$i++;
			}
		}
		else {
			%activeChannel.chat.Draw();
		}
	}
};
