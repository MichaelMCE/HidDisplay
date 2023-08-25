/* Dumb Chat display manager.  Handles updating displayed text,
 * scrolling text, etc.  Does not handle any message processing.
 * Does jump to last lien on new text.
 */

#requires <Views\CalculatorView.c>
#requires <Modules\LineEditor.c>

struct ChatLine {
	var %height;
	//var %user;
	var %text;
	function ChatLine($_text) {
		%text = $_text;
		%height = TextSize($_text)[1];
	}
	// var %stamp;
};

struct Chat {
	var %self;
	var %names;
	var %text;
	var %editor;
	var %id;
	var %font;
	var %hasFocus;
	var %updated;

	var %chatLog, %chatLogLine;

	var %showName;

	var %currentLine;
	var %currentOffset;
	var %tabCompleteHint;
	var %tabCompleteEnd;

	function Chat($_id, $_self, $startText) {
		%id = $_id;
		%self = $_self;
		%chatLog = list();

		%font = Font("04b03", 8);
		%text = list();
		%names = list();
		if (size($startText)) {
			%AddText($startText);
		}
		%editor = LineEditor(160, %font);
		%tabCompleteHint = -1;
		%ShowName();
	}

	function ShowName() {
		%showName = Time()+3;
		// Shouldn't be needed, since only triggered by redraw related events.
		// NeedRedraw();
	}

	function Focus() {
		if (!%hasFocus) {
			RegisterKeyEvent(0, VK_VOLUME_DOWN, VK_VOLUME_UP);
			%editor.Focus();
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
			%editor.Unfocus();
			UnregisterKeyEvent(0, VK_VOLUME_DOWN, VK_VOLUME_UP);
		}
	}

	function ClearEditor() {
		$t = %editor.GetText();
		if (size($t)) {
			%chatLog[size(%chatLog)] = $t;
			%chatLogLine = size(%chatLog);
			%editor.SetText();
		}
	}

	function ToBottom() {
		%currentLine = size(%text)-1;
		%currentOffset = 0;
	}

	function ClearNames() {
		%names=list();
	}

	function AddNameUnsorted($user) {
		%names[size(%names)] = $user;
	}

	function AddName($user) {
		%names[size(%names)] = $user;
		%SortNames();
	}

	function RemoveName($name) {
		$min = 0;
		$max = size(%names)-1;
		while ($min < $max) {
			$mid = ($min+$max)/2;
			if (%names[$mid][0] <s $name) {
				$min = $mid+1;
			}
			else {
				$max = $mid;
			}
		}
		if (%names[$min][0] ==s $name) {
			return pop(%names, $min);
		}
		return null;
	}

	function FindName($name) {
		$min = 0;
		$max = size(%names)-1;
		while ($min < $max) {
			$mid = ($min+$max)/2;
			if (%names[$mid][0] <s $name) {
				$min = $mid+1;
			}
			else {
				$max = $mid;
			}
		}
		if (%names[$min][0] ==s $name) {
			return %names[$min];
		}
		return null;
	}

	function SortNames() {
		$n = list();
		for ($i=size(%names)-1; $i>=0; $i--) {
			$n[$i] = %names[$i][0];
		}
		$indices = indexstablesort($n);
		$n2 = $n;
		$n = %names;
		%names = list();
		for ($i=size($indices)-1; $i>=0; $i--) {
			%names[$i] = $n[$indices[$i]];
		}
	}


	function KeyDown($event, $param, $modifiers, $vk, $string) {
		$vk = MediaFlip($vk);
		UseFont(%font);
		$fh = GetFontHeight();
		NeedRedraw();

		if ($vk == VK_TAB) {
			$line = %editor.GetText();
			if (size($line) && size(%names)) {
				$start = %editor.cursorPos;
				$end = $start;
				while ($line[$end] !=S " " && $line[$end] !=S "|t" && $end < size($line)) $end++;
				while ($line[$start-1] !=S " " && $line[$start-1] !=S "|t" && $start) $start--;
				if ($start == $end) {
					while ($start && ($line[$start-1] ==S " " || $line[$start-1] ==S "|t")) {
						$start --;
					}
					$end = $start;
					while ($line[$end] !=S " " && $line[$end] !=S "|t" && $end < size($line)) $end++;
					while ($line[$start-1] !=S " " && $line[$start-1] !=S "|t" && $start) $start--;
				}
				if (%tabCompleteHint == -1) {
					%tabCompleteHint = size(%names)-1;
					%tabCompleteEnd = $end;
				}
				else {
					if (%tabCompleteHint >= size(%names))
						%tabCompleteHint = size(%names)-1;
				}
				$find = substring($line, $start, %tabCompleteEnd);
				if ($end != $start) {
					$pos = %tabCompleteHint;
					while (1) {
						$pos = ($pos + 1)  % size(%names);
						$n = %names[$pos];
						if (!IsString($n)) $n = $n[0];
						if (substring($n, 0, size($find)) ==s $find) {
							%editor.cursorPos = $end;
							%editor.selStart = $start;
							%editor.KeyDown($event, $param, 0, VK_A, $n);
							%tabCompleteHint = $pos;
							return 1;
						}
						if ($pos == %tabCompleteHint) return 1;
					}
				}
			}
			return 1;
		}
		%tabCompleteHint = -1;
		if ($vk == VK_DOWN) {
			if ($modifiers & 2) {
				%KeyDown(,,, VK_VOLUME_UP);
			}
			else if (%chatLogLine < size(%chatLog)) {
				%editor.SetText(%chatLog[++%chatLogLine]);
			}
			else {
				$t = %editor.GetText();
				if (size($t)) {
					%chatLog[size(%chatLog)] = $t;
					%chatLogLine = size(%chatLog);
					%editor.SetText();
				}
			}
			return 1;
		}
		else if ($vk == VK_UP) {
			if ($modifiers & 2) {
				%KeyDown(,,, VK_VOLUME_DOWN);
			}
			else if (%chatLogLine >= 0) {
				%editor.SetText(%chatLog[--%chatLogLine]);
			}
			return 1;
		}
		else if ($vk == VK_VOLUME_DOWN) {
			if (%currentOffset + $fh < %text[%currentLine].height) {
				%currentOffset += $fh;
			}
			else if (%currentLine) {
				%currentLine--;
				%currentOffset = 0;
			}
			return 1;
		}
		else if ($vk == VK_VOLUME_UP) {
			if (%currentOffset) {
				%currentOffset -= $fh;
			}
			else if (%currentLine < size(%text)-1) {
				%currentLine++;
				%currentOffset = %text[%currentLine].height - $fh;
			}
			return 1;
		}
		else if ($vk == VK_PRIOR) {
			for ($i=0; $i<4;$i++) {
				%KeyDown(,,, VK_VOLUME_DOWN);
			}
			return 1;
		}
		else if ($vk == VK_NEXT) {
			for ($i=0; $i<4;$i++) {
				%KeyDown(,,, VK_VOLUME_UP);
			}
			return 1;
		}
		else if ($modifiers & 2) {
			if ($vk == VK_END) {
				%currentLine = size(%text)-1;
				%currentOffset = 0;
				return 1;
			}
			else if ($vk == VK_HOME) {
				%currentLine = 0;
				%currentOffset = %text[0].height - $fh;
				return 1;
			}
			else if ($vk == VK_I) {
				%ShowName();
				return 1;
			}
		}
		return %editor.KeyDown(@$);
	}

	// At the moment, only called externally.
	function ExecCommand($command, $params) {
		if ($command ==s "CLEAR" || $command ==s "CLS") {
			%text = list();
		}
		else if ($command ==s "Ping") {
			if (!size($params))
				%AddText("Ping: Error, no ip given");
			else if (IsNull($ipAddr = IPAddrWait($params)[0])) {
				%AddText("Ping: " +s " Can't resolve " +s $params);
			}
			else {
				$res = $ipAddr.PingWait();
				if (!IsInt($res)) {
					%AddText("Ping " +s $params +s ": " +s $res);
				}
				else {
					%AddText("Ping " +s $params +s ": " +s $res +s " ms");
				}
			}
		}
		else if ($command ==s "dns") {
			if (!size($params))
				%AddText("DNS: Error, no dns given");
			else if (IsNull($ipAddr = IPAddrWait($params)[0])) {
				%AddText("DNS: Can't resolve " +s $params);
			}
			else if (IsInt(strstr($params, ":")) || !IsNull(RegExp($params, "^[0-9\.]*$"))) {
				$dns = $ipAddr.GetDNSWait();
				if (!IsString($dns)) {
					%AddText("DNS: Can't resolve " +s $params);
				}
				else {
					%AddText("DNS: " +s $params +s " resolves to " +s $dns);
				}
			}
			else {
				%AddText("DNS: " +s $params +s " resolves to " +s $ipAddr.GetString());
			}
		}
		else if ($command ==s "echo") {
			%AddText($params);
		}
		else if ($command ==s "calc" || $command ==s "eval") {
			%AddText($params +s " = " +s Eval($params));
		}
		else return 0;
		return 1;
	}

	function AddText($text2) {
		UseFont(%font);
		// Can allow one extra pixel because there's 1 pixel of whitespace to the right of characters.
		// Can't do this when a cursor is needed.
		$s2 = FormatText($text2, 161);
		$p = strstr($s2, "|n");
		if ($p) {
			$first = substring($text2, 0, $p);
			if ($text2[$p] ==s "|n") $p++;
			$text2 = $first +s "|n  " +s strreplace(FormatText(substring($text2, $p, -1), 153), "|n", "|n  ");
		}
		%text[size(%text)] = ChatLine($text2);
		%updated = 1;
	}

	function AddUserText($_user, $_text) {
		if (substring($_text, 0, 4) ==s "/me ") {
			$text2 = "* " +s $_user +s substring($_text, 3, -1);
		}
		else {
			$text2 = $_user +s ": " +s $_text;
		}
		%AddText($text2);
	}

	function Draw() {
		if (%updated) {
			%ToBottom();
			%updated = 0;
		}
		ClearScreen();
		UseFont(%font);

		$pos = %currentLine;
		$pos2 = %currentOffset;
		//$pos = size(%text)-1;
		//$pos2 = 0;

		$out = 34;
		while ($out >= -7 && $pos >= 0) {
			$out -= %text[$pos].height - $pos2;
			DisplayText(%text[$pos].text, 0, $out);
			$pos --;
			$pos2 = 0;
		}

		DrawRect(0,34,159,34);
		ClearRect(0,35,159,42);
		%editor.Draw(0, 35);
		if (%showName) {
			if (%showName < Time()) {
				%showName = 0;
			}
			else {
				ClearRect(0,0,159,8);
				DrawRect(0,9,159,9);
				DisplayTextCentered(%id, 79);
			}
		}
	}
};