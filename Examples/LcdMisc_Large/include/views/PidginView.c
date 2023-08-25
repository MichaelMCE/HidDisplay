#requires <Modules\Chat.c>
#requires <Views\View.c>
#requires <util\Text.c>

struct PidginUser {
	// %chatUrl has url for displaying chat lines, %chatName has string used for sending
	// messages.  Related, except when %chatUrl is null.  Values often signficiantly
	// different in that case.
	var %name, %chat, %chatName, %status, %chatUrl, %used, %lastLine;
	function PidginUser($_name, $_chatName, $_status) {
		%name = $_name;
		%chatName = $_chatName;
		%status = $_status;
	}
};

struct PidginView extends View {
	var %active, %font, %users, %selectedUser, %url, %boldFont;

	var %newUsers;

	var %updateCount, %lastUpdate, %activeUpdate;//, %updateNumber;

	var %search;
	var %searchTime;

	function PidginView ($_url) {
		if (size($_url)) {
			%url = $_url;
		}
		else {
			%url = GetString("URLs", "Pidgin");
		}
		%lastUpdate = Time()-10;
		%Update();
		%font = Font("04b03", 8);
		%boldFont = Font("04b03", 8, 0, 1);
		%toolbarImage = LoadImage("Images\Pidgin.png");
		%noDrawOnAudioChange = 1;
		%noDrawOnCounterUpdate = 1;
	}

	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
			// Test was chosen so it matches ReplaceUsers().
			if (!IsNull(%users[%active].chat)) %users[%active].chat.Focus();
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
			if (!IsNull(%users[%active].chat)) %users[%active].chat.Unfocus();
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
		%users[%active].chat.ShowName();
	}

	function KeyDown($event, $param, $modifiers, $vk, $string) {
		if (!size(%users)) return 1;
		if (IsNull(%users[%active].chat)) {
			$vk = MediaFlip($vk);
			NeedRedraw();
			if ($vk == VK_RETURN) {
				if (size(%users[%selectedUser].chatName)) {
					%SetActive(%selectedUser);
				}
				%search = "";
				return 1;
			}
			if (size($string)) {
				if (GetTickCount() - %searchTime >= 1400) {
					%search = "";
				}
				%searchTime = GetTickCount();
				%search +=s $string;
				$index = %selectedUser;
				while (1) {
					if (substring(%users[$index].name, 0, size(%search)) ==s %search) break;
					$index = ($index + 1) % size(%users);
					if ($index == %selectedUser) break;
				}
				%selectedUser = $index;
				return 1;
			}
			%search = "";
			if ($vk == VK_VOLUME_UP || $vk == VK_DOWN) {
				%selectedUser++;
				if (%selectedUser >= size(%users)) {
					%selectedUser = 0;
				}
			}
			else if ($vk == VK_VOLUME_DOWN || $vk == VK_UP) {
				%selectedUser--;
				if (%selectedUser < 0) {
					%selectedUser = size(%users)-1;
				}
			}
			else if ($vk == VK_HOME) {
				%selectedUser = 0;
			}
			else if ($vk == VK_END) {
				%selectedUser = size(%users)-1;
			}
			else if ($vk == VK_PRIOR) {
				if (!%selectedUser) {
					%selectedUser = size(%users)-1;
				}
				else {
					%selectedUser-=5;
					if (%selectedUser < 0) %selectedUser = 0;
				}
			}
			else if ($vk == VK_NEXT) {
				if (%selectedUser == size(%users)-1) {
					%selectedUser = 0;
				}
				else {
					%selectedUser += 5;
					if (%selectedUser >= size(%users)) %selectedUser = size(%users)-1;
				}
			}
			return 1;
		}
		else {
			if ($vk == VK_RETURN) {
				$text = %users[%active].chat.editor.GetText();
				if (size($text)) {
					%users[%active].chat.ClearEditor();
					NeedRedraw();
					$command = RegExp($text, "^\w*\\([^\w]*)\w*(.*)");
					if (!IsNull($command)) {
						if ($command[0][0][0] !=S "\") { //"
							%users[%active].chat.ExecCommand($command[0][0], $command[1][0]);
							return 1;
						}
						$text = strreplace($text, "\", "", 1); //"
					}
					HttpGetWait(%url +s "sendMessage?" +s %users[%active].chatName +s "=" +s $text);
					%UpdateUsersWait();
				}
				return 1;
			}
			if ($modifiers == 2) {
				if ($vk == VK_W) {
					while (1) {
						$hWnd = FindWindow(, %users[%active].name, , $hWnd);
						if (!$hWnd) break;
						$process = GetWindowModuleFileName($hWnd);
						if ($process ==s "Pidgin.exe") {
							CloseWindow($hWnd);
							break;
						}
					}
					%SetActive(size(%users));
					return 1;
				}
				else if ($vk == VK_O) {
					%SetActive(size(%users));
					return 1;
				}
			}
			return %users[%active].chat.KeyDown(@$);
		}
	}

	function SetActive($u) {
		if ($u < size(%users) && IsNull(%users[$u].chat)) {
			%users[$u].chat = Chat(%users[$u].name, null, null);
		}
		if (%hasFocus && %active != $u) {
			%Unfocus();
			%active = $u;
			%users[%active].chat.ShowName();
			%Focus();
		}
		%active = $u;
		%users[$u].chat.updated = 1;
		NeedRedraw();
	}

	function G15ButtonDown($event, $param, $buttons) {
		%search = "";
		if ($buttons & 0xF) {
			if ($buttons & 3) {
				$state = G15GetButtonsState();
				if (!($state & ($buttons^3))) {
					$u = %active;
					if ($buttons == 1) {
						while ($u >= 0) {
							$u--;
							if (!IsNull(%users[$u].chat)) break;
						}
						if ($u < 0) $u = size(%users);
						%SetActive($u);
					}
					else {
						if ($u == size(%users)) $u = -1;
						while ($u < size(%users)) {
							$u++;
							if (!IsNull(%users[$u].chat)) break;
						}
						%SetActive($u);
					}
				}
			}
			else {
				if ($buttons & 8) {
					%Unfocus();
				}
				else if ($buttons & 4) {
					%KeyDown(, , , VK_RETURN);
				}
			}
			NeedRedraw();
			return 1;
		}
	}


	function ReplaceUsers() {
		$newActive = %active;
		if (%active >= size(%users)) {
			$newActive = size(%newUsers);
		}
		else {
			for ($i=0; $i<size(%newUsers); $i++) {
				if (%users[%active].name ==s %newUsers[$i].name) {
					$newActive = $i;
					if (IsNull(%newUsers[$i].chat)) {
						%newUsers[$i].chat = %users[%active].chat;
					}
					break;
				}
			}
			if ($i == size(%newUsers)) {
				$newActive = size(%newUsers);
			}
		}
		if (%users[%selectedUser].name !=s %newUsers[%selectedUser].name) {
			for ($i=0; $i<size(%newUsers); $i++) {
				if (%users[%selectedUser].name ==s %newUsers[$i].name) {
					%selectedUser = $i;
					break;
				}
			}
		}

		// If just connecting to Pidgin, nothing is new.
		if (IsNull(%users)) {
			for ($i=0; $i<size(%newUsers); $i++) {
				%newUsers[$i].chat.ToBottom();
				%newUsers[$i].chat.updated = 0;
			}
		}

		// Not sure how often this happens, if ever.
		if (!Equals(%users[%active].chat, %newUsers[$newActive].chat) && %hasFocus) {
			%Unfocus();
			%users = %newUsers;
			%active = $newActive;
			%users[%active].chat.ShowName();
			%Focus();
		}
		else {
			%users = %newUsers;
			%active = $newActive;
		}
		NeedRedraw();
	}

	function UpdateUserWait($i) {
		%updateCount ++;
		$html = HttpGetWait(%url +s %newUsers[$i].chatUrl);
		$pos = 0;
		$lines = list();
		if (size(%newUsers[$i].lastLine)) {
			$end = strstr($html, %newUsers[$i].lastLine);
		}
		if (!$end) $end = size($html);
		while (1) {
			$res = RegExp($html, "\(##?:##:##\)&nbsp;<B>([^<]*)</B>:&nbsp;([.\n]*?)<BR>|n\(##?:##:##\)&nbsp;<B>", $pos);
			if (IsNull($res)) {
				$res = RegExp($html, "\(##?:##:##\)&nbsp;<B>([^<]*)</B>:&nbsp;([.\n]*?)<BR>|n<HR>", $pos);
				if (IsNull($res)) break;
			}
			$pos = $res[1][1] + size($res[1][0]);
			if ($pos >= $end) break;
			push_back($lines, $res);
			if (!$start) {
				$start = 1;
				%newUsers[$i].lastLine = substring($html, $res[0][1]-19, $pos) +s "<BR>";
			}
		}
		for ($j = size($lines) - 1; $j>=0; $j--) {
			$p = 0;
			$name = $lines[$j][0][0];
			while ($t = strstr($name, "/", $p+1)) {
				$p = $t;
			}
			if ($p)
				$name = substring($name, 0, $p);
			// Fix capitalization.
			if ($name ==s %newUsers[$i].name) {
				$name = %newUsers[$i].name;
			}
			%newUsers[$i].chat.AddUserText($name, $lines[$j][1][0]);
		}
		%updateCount --;
		if (!%updateCount) %ReplaceUsers();
	}

	function UpdateUsersWait() {
		if (!%updateCount) {
			%updateCount ++;
			$html = HttpGetWait(%url);
			if (size($html)>20) {
				%newUsers = list();
				$pos = stristr($html, "<HR><B>");
				$split = $pos;
				if (!IsNull($pos)) {
					$pos2 = stristr($html, "<B>", $pos);
					while (1) {
						$pos = stristr($html, "<A HREF=conversation?time=", $pos);
						if (!$pos) break;
						if ($pos2 && $pos2 < $pos) {
							$end = stristr($html, "</B>", $pos2);
							if ($end)
								push_back(%newUsers, PidginUser(substring($html, $pos2+3, $end)));
							$pos2 = stristr($html, "<B>", $pos);
						}

						$pos = strstr($html, "&", $pos);
						if (!$pos) break;
						$mid = strstr($html, " >", $pos);
						if (!$mid) continue;
						$chatName = substring($html, $pos+1, $mid);
						$pos = strstr($html, "<", $mid);
						if (!$pos) break;
						$name = substring($html, $mid+2, $pos);
						$pos+=4;
						$status = "";
						if ($html[$pos+1] ==S "(") {
							$pos3 = stristr($html, "<BR>", $pos);
							if ($html[$pos3-1] ==S ")" && $pos < $pos3) {
								$status = substring($html, $pos+1, $pos3);
							}
						}

						push_back(%newUsers, PidginUser($name, $chatName, $status));
					}
				}
				else $split = size($html);
				$pos = 0;
				while (($pos = stristr($html, "<A HREF=conversation?time=", $pos)) && $pos < $split) {
					$mid = strstr($html, " >", $pos);
					$pos++;
					if (!$mid) continue;
					$chatUrl = substring($html, $pos+7, $mid);

					$t = strstr($chatUrl, "&");
					if (!$t) continue;
					$chatName = substring($chatUrl, $t+1, size($chatUrl));

					$pos = strstr($html, "<", $mid);
					if (!$pos) break;
					$name = substring($html, $mid+2, $pos);

					for ($i = 0; $i<size(%newUsers); $i++) {
						if (IsNull(%newUsers[$i].chat)) {
							if ($chatName ==s  %newUsers[$i].chatName) break;
							$test = stristr($name, %newUsers[$i].name, 0, size(%newUsers[$i].name));
							if (!IsNull($test)) {
								$p = size(%newUsers[$i].name);
								if (size($name) == $p || $name[$p] ==S "/") {
									%newUsers[$i].chatName = $chatName;
									break;
								}
							}
						}
					}
					if ($i == size(%newUsers)) {
						if (!$AddOthers) {
							$AddOthers = 1;
							push_back(%newUsers, PidginUser("Other Connections"));
							$i++;
						}
						push_back(%newUsers, PidginUser($name, $chatName, ""));
					}
					%newUsers[$i].chatUrl = $chatUrl;
					for ($j = 0; $j<size(%users); $j++) {
						if (%users[$j].used || %users[$j].name !=S %newUsers[$i].name) continue;
						%users[$j].used = 1;
						%newUsers[$i].chat = %users[$j].chat;
						%newUsers[$i].lastLine = %users[$j].lastLine;
					}
					if (IsNull(%newUsers[$i].chat)) {
						%newUsers[$i].chat = Chat(%newUsers[$i].name, null, null);
					}
					//Wait(50);
					//%UpdateUserWait($i);
					SpawnThread("UpdateUserWait", $this, list($i));
				}

				$pos = stristr($html, "<HR><B>Buddies</B><BR>");
			}
			else {
				%newUsers = null;
			}
			%updateCount --;
			if (!%updateCount) %ReplaceUsers();
		}
	}

	function Update() {
		$time = Time();
		if (!%updateCount) {
			if ($time-%lastUpdate >= 5 - 3 * %hasFocus) {
				%activeUpdate = $time;
				%lastUpdate = $time;
				SpawnThread("UpdateUsersWait", $this);
			}
			else if (%selectedUser < size(%users) && %hasFocus && %activeUpdate < $time) {
				%activeUpdate = $time;
				// Update current conversation
				SpawnThread("UpdateUserWait", $this, list(%selectedUser));
			}
		}
	}

	function CounterUpdate() {
		%Update();
	}

	function Draw() {
		ClearScreen();
		UseFont(%font);
		if (IsNull(%users[%active].chat)) {
			$i = %selectedUser - (%selectedUser % 5);
			if (!size(%users)) {
				if (IsNull(%users)) {
					DisplayText("Can't connect to Pidgin.");
				}
				else {
					DisplayText("No active users.");
				}
				return;
			}
			$pos = 0;
			$h = GetFontHeight();
			while ($pos < 41 && $i < size(%users)) {
				if (IsNull(%users[$i].chatName)) {
					UseFont(%boldFont);
					$s = 1;
				}
				else {
					if (!IsNull(%users[$i].chat)) {
						if (%users[$i].chat.updated) {
							DisplayText("*", 2, $pos);
						}
						UseFont(%boldFont);
					}
					$s = 6;
				}
				$len = DisplayText(%users[$i].name, $s, $pos);
				UseFont(%font);
				DisplayText(%users[$i].status, $s + $len+5, $pos);
				if ($i == %selectedUser && %hasFocus) {
					InvertRect(0, $pos, 159, $pos+$h-1);
				}
				$pos += $h;
				$i++;
			}
		}
		else {
			%users[%active].chat.Draw();
		}
	}
};

function GetPidginView ($url) {
	$out = PidginView($url);
	if (size($out.url)) return $out;
}
