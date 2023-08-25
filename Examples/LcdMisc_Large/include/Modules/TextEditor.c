#import <constants.h>
#requires <util\Text.c>

// Uses UndoInfo
#requires <Modules\LineEditor.c>

struct Position {
	var %line, %pos;
	function Position ($_line, $_pos) {
		%line = $_line;
		%pos = $_pos;
	}
};

/* Must be unfocused before destruction.
 */
struct TextEditor {
	var %text, %cursorPos, %selStart, %hasFocus, %flashTimer, %cursor, %textSize, %offset,
		%font, %width,
		%cursorPx, %selPx, %offsetHeight,
		%undo, %redo, %splitUndo;

	function TextEditor($_width, $_font) {
		%text = list("");
		%offset = Position(0,0);
		%cursorPos = Position(0,0);
		%selStart = Position(0,0);
		%font = $_font;
		%width = $_width;
		%Update();
	}

	function GetPx($pos) {
		$start = 0;
		$line = %text[$pos.line];
		$fh = GetFontHeight();
		$height = 0;
		while (!IsNull($end = strstr($line, "|n", $start))) {
			if ($end >= $pos.pos) break;
			$start = $end+1;
			$height += $fh;
		}
		if ($pos.line != %offset.line) {
			$i = $pos.line;
			while ($i < %offset.line) {
				$height -= TextSize(%text[$i])[1];
				$i++;
			}
			while ($i > %offset.line) {
				$i--;
				$height += TextSize(%text[$i])[1];
			}
		}
		$p = TextSize(substring($line, $start, $pos.pos))[0];
		if ($p) $p--;
		return list($p, $height);
	}

	function Update() {
		UseFont(%font);
		%cursorPx = %GetPx(%cursorPos);
		$offsetPx = %GetPx(%offset);
		$fh = GetFontHeight();

		if ($offsetPx[1] > %cursorPx[1]) {
			%offset = Position(%cursorPos.line, %cursorPos.pos);
		}
		else if ($offsetPx[1] + DHEIGHT - $fh < %cursorPx[1]) {
			%offset = %cursorPos;
			for ($i=0; $i<DHEIGHT-2*$fh; $i+=$fh) {
				%offset = %LineUp(%offset, 0);
			}
		}

		%cursorPx = %GetPx(%cursorPos);
		$offsetPx = %GetPx(%offset);
		%offsetHeight = $offsetPx[1];
		%selPx = %GetPx(%selStart);

		%cursorPx[1] -= %offsetHeight;
		%selPx[1] -= %offsetHeight;

	}

	function Save($_file) {
		$writer = FileWriter($_file, WRITER_UTF8);
		if (IsNull($writer)) return;
		if (!$writer.Write(%text[0])) return;
		for ($i=1; $i < size(%text); $i++) {
			if (!$writer.Write("|r|n") ||
				!$writer.Write(%text[$i])) return;
		}
		return 1;
	}

	function Load($_file) {
		$reader = FileReader($_file, READER_NORMAL);
		if (IsNull($reader)) return;
		%offset = Position(0,0);
		%cursorPos = Position(0,0);
		%selStart = Position(0,0);

		%text = list();

		UseFont(%font);

		while (IsString($line = $reader.Read(-1))) {
			%text[size(%text)] = FormatText($line, DWIDTH);
		}
		%Update();
		return 1;
	}

	function ChangeFormat($_width, $_font) {
		%font = $_font;
		%width = $_width;
		%Update();
	}

	function GetText() {
		return %text;
	}

	function Clear() {
		%SetText("");
	}

    function SetText($_text) {
		%undo = null;
		%redo = null;
		// Make sure it's really text.
		%text = "" +s $_text;
		%selStart = %cursorPos = size($_text);
		%offsetHeight = 0;
		%Update();
	}

	function CursorFlash() {
		%cursor ^= 1;
		NeedRedraw();
	}

	function Focus() {
		if (!%hasFocus) {
			%flashTimer = CreateFastTimer("CursorFlash", 500,, $this);
			RegisterKeyRange(3<<16, VK_0, VK_Z);
			RegisterKeyEvent(3<<16, VK_BACK, VK_TAB, VK_RETURN);
			RegisterKeyRange(3<<16, VK_SPACE, VK_HELP);
			RegisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
			RegisterKeyRange(3<<16, 0xBA, 0xC0);
			RegisterKeyRange(3<<16, 0xDB, 0xDF);
			RegisterKeyRange(3<<16, 0xE1, 0xE4);
			%hasFocus = 1;
			%cursor = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%cursor = 0;
			KillTimer(%flashTimer);
			%flashTimer = 0;
			UnregisterKeyRange(3<<16, VK_0, VK_Z);
			UnregisterKeyEvent(3<<16, VK_BACK, VK_TAB, VK_RETURN);
			UnregisterKeyRange(3<<16, VK_SPACE, VK_HELP);
			UnregisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
			UnregisterKeyRange(3<<16, 0xBA, 0xC0);
			UnregisterKeyRange(3<<16, 0xDB, 0xDF);
			UnregisterKeyRange(3<<16, 0xE1, 0xE4);
			%hasFocus = 0;
		}
	}

	// Responsible for all string modifications - insertions, deletions,
	// and substitutions.
	function ReplaceSelected($string) {
		if (%selStart.line == %cursorPos.line && %selStart.pos == %cursorPos.pos && !size($string)) {
			// Nothing to do.  Check needed to avoid adding stuff to the undo buffer.
			return;
		}
		$start = %cursorPos;
		$end = %selStart;
		if ($start.line > $end.line ||
			($start.line == $end.line && $start.pos > $end.pos)) {
			$start = $end;
			$end = %cursorPos;
		}
		$string = strreplace($string, "|r|n", "|n");
		$string = strreplace($string, "|r", "|n");
		$line = $start.line;
		$temp = strswap(%text[$line], "", 0, $start.pos);
		$prefix = strreplace($temp[1], "|n", "");
		%text[$line] = $temp[0];
		$endLine = $end.line;
		if ($endLine == $start.line)
			$end = Position($end.line, $end.pos - $start.pos);
		while ($endLine != $line) {
			$mid = $mid +s strreplace(%text[$line], "|n", "") +s "|n";
			pop(%text, $line);
			$endLine --;
		}
		$temp = strswap(%text[$line], "", 0, $end.pos);
		pop(%text, $line);
		$suffix = strreplace($temp[0], "|n", "");
		$mid = $mid +s strreplace($temp[1], "|n", "");

		$temp = strsplit($prefix +s $string +s $suffix, "|n", 0, 0, 1);
		if (!size($temp)) $temp = list("");
		$count = size($temp);
		while (size($temp)) {
			insert(%text, $line, FormatText(pop($temp), %width));
		}

		$prefix2 = %text[$line];
		$p1 = 0;
		$p2 = 0;
		while ($p1 < size($prefix)) {
			if ($prefix2[$p2] ==S "|n") $p2++;
			$p2 ++;
			$p1 ++;
		}
		$start = Position($line, $p2);

		$suffix2 = %text[$line+$count-1];
		$p1 = size($suffix);
		$p2 = size($suffix2);
		while ($p1>0) {
			$p2 --;
			$p1 --;
			if ($suffix2[$p2] ==S "|n") $p2--;
		}
		if ($p2 < 0) $p2 = 0;

		%cursorPos = Position($line+$count-1, $p2);
		%selStart = Position(%cursorPos.line, %cursorPos.pos);
		%Update();

		%undo = UndoInfo($start, %cursorPos, $mid, %undo);
		%splitUndo = 0;
		%redo = null;
	}

	function Undo($reallyRedo) {
		if ($reallyRedo) {
			$move = %redo;
		}
		else {
			$move = %undo;
		}
		if (!IsNull($move)) {
			if (!$reallyRedo)
				%undo = %redo;
			$move2 = $move.next;

			%cursorPos = $move.start;
			%selStart = $move.end;

			%ReplaceSelected($move.string);

			if ($reallyRedo) {
				%redo = $move2;
			}
			else {
				%redo = %undo;
				%undo = $move2;
			}
		}
	}

	function NextChar($_pos) {
		$pos = Position($_pos.line, $_pos.pos);
		$line = %text[$pos.line];
		if ($pos.pos < size($line)) {
			$pos.pos ++;
			$hex = ParseBinaryInt($line, $pos.pos, 1, 1);
			while ($pos.pos < size($line) && $hex >= 0x80 && $hex < 0xC0) {
				$pos.pos ++;
				$hex = ParseBinaryInt($line, $pos.pos, 1, 1);
			}
		}
		else if ($pos.line+1 < size(%text)) {
			$pos.line++;
			$pos.pos = 0;
		}
		return $pos;
	}

	function PrevChar($_pos) {
		$pos = Position($_pos.line, $_pos.pos);
		if ($pos.pos) {
			$pos.pos --;
			$line = %text[$pos.line];
			$hex = ParseBinaryInt($line, $pos.pos, 1, 1);
			while ($pos.pos && $hex >= 0x80 && $hex < 0xC0) {
				$pos.pos --;
				$hex = ParseBinaryInt($line, $pos.pos, 1, 1);
			}
		}
		else if ($pos.line) {
			$pos.line--;
			$pos.pos = size(%text[$pos.line]);
		}
		return $pos;
	}

	function MatchPx($x, $y, $px) {
		$line = %text[$y];

		while($x < size($line)) {
			$next = $x+1;
			$hex = ParseBinaryInt($line, $next, 1, 1);
			while ($next < size($line) && $hex >= 0x80 && $hex < 0xC0) {
				$next ++;
				$hex = ParseBinaryInt($line, $next, 1, 1);
			}

			$c = $line[$x];
			if ($c ==S "|n") break;
			$p2 = $p1 + TextSize(substring($line, $x, $next))[0];
			if ($p1 <= $px && $px <= $p2) {
				if ($px - $p1 > $p2 - $px) {
					$x = $next;
				}
				break;
			}
			$p1 = $p2;
			$x = $next;
		}
		return $x;
	}

	function LineDown($pos, $px) {
		$y = $pos.line;
		$line = %text[$y];
		$x = strstr($line, "|n", $pos.pos);
		if ($x) {
			$x++;
		}
		else if ($y < size(%text)-1) {
			// Next line.
			$y++;
			$x = 0;
		}
		else {
			// Last line.
			return Position($y, size($line));
		}
		return Position($y, %MatchPx($x, $y, $px));
	}

	function LineUp($pos, $px) {
		$y = $pos.line;
		$line = %text[$y];
		$x = $pos.pos;
		while ($x && $line[$x-1] !=S "|n") $x --;
		if ($x) $x --;
		else if ($y) {
			$y --;
			$line = %text[$y];
			$x = size($line);
		}
		else return Position(0,0);
		while ($x && $line[$x-1] !=S "|n") $x --;
		return Position($y, %MatchPx($x, $y, $px));
	}

	function GetSelText() {
		$start = %cursorPos;
		$end = %selStart;
		if ($start.line > $end.line ||
			($start.line == $end.line && $start.pos > $end.pos)) {
			$start = $end;
			$end = %cursorPos;
		}

		$endLine = $end.line;
		$line = $start.line;
		$startPos = $start.pos;
		while ($endLine != $line) {
			$string = $string +s strreplace(substring(%text[$line], $startPos, size(%text[$line])), "|n", "") +s "|r|n";
			$startPos = 0;
			$line++;
		}
		return $string +s strreplace(substring(%text[$line], $startPos, $end.pos), "|n", "");
	}

	// Complete text edit control behavior...
	function KeyDown($event, $param, $modifiers, $vkey, $string) {
		if (($modifiers & 12)) return;
		if (!($modifiers & 2) || ($vkey >= VK_PRIOR && $vkey <= VK_DOWN)) {
			$processed = 1;
			if ($vkey == VK_BACK || $vkey == VK_DELETE) {
				if (%selStart.line == %cursorPos.line && %selStart.pos == %cursorPos.pos) {
					if ($vkey == VK_DELETE) {
						%selStart = %NextChar(%cursorPos);
					}
					else {
						%selStart = %PrevChar(%cursorPos);
					}
				}
				%ReplaceSelected("");
			}
			else if (size($string)) {
				%ReplaceSelected($string);
				$modifiers = 0;
			}
			else if ($vkey == VK_RIGHT) {
				%cursorPos = %NextChar(%cursorPos);
			}
			else if ($vkey == VK_LEFT) {
				%cursorPos = %PrevChar(%cursorPos);
			}
			else if ($vkey == VK_END) {
				if ($modifiers & 2) {
					%cursorPos.line = size(%text)-1;
					%cursorPos.pos = size(%text[%cursorPos.line]);
				}
				else {
					$line = %text[%cursorPos.line];
					%cursorPos.pos = strstr($line, "|n", %cursorPos.pos);
					if (!%cursorPos.pos) %cursorPos.pos = size($line);
				}
			}
			else if ($vkey == VK_HOME) {
				if ($modifiers & 2) {
					%cursorPos = Position(0,0);
				}
				else {
					$line = %text[%cursorPos.line];
					// Slow but simple.
					while (%cursorPos.pos > 0 && $line[%cursorPos.pos-1] !=S "|n") %cursorPos.pos--;
				}
			}
			else if ($vkey == VK_DOWN) {
				%cursorPos = %LineDown(%cursorPos, %cursorPx[0]);
			}
			else if ($vkey == VK_UP) {
				%cursorPos = %LineUp(%cursorPos, %cursorPx[0]);
			}
			else if ($vkey == VK_NEXT) {
				for ($i=0; $i<5; $i++)
					%cursorPos = %LineDown(%cursorPos, %cursorPx[0]);
			}
			else if ($vkey == VK_PRIOR) {
				for ($i=0; $i<5; $i++)
					%cursorPos = %LineUp(%cursorPos, %cursorPx[0]);
			}
			else return;
			if (!($modifiers & 1)) {
				%selStart = Position(%cursorPos.line, %cursorPos.pos);
			}
		}
		else {
			if ($vkey == VK_A) {
				$line = size(%text)-1;
				%cursorPos = Position($line, size(%text[$line]));
				%selStart = Position(0,0);
			}
			else if ($vkey == VK_C) {
				SetClipboardData(%GetSelText());
			}
			else if ($vkey == VK_X) {
				SetClipboardData(%GetSelText());
				%splitUndo = 1;
				%ReplaceSelected("");
			}
			else if ($vkey == VK_V) {
				$data = GetClipboardData(CF_UNICODETEXT)[1];
				if (IsString($data) && size($data)) {
					%splitUndo = 1;
					%ReplaceSelected($data);
				}
				else return;
			}
			else if ($vkey == VK_Z) {
				if (!($modifiers & 1))
					%Undo();
				else
					%Undo(1);
			}
			else if ($vkey == VK_Y) {
				%Undo(1);
			}
			else {
				return;
			}
			%splitUndo = 1;
		}
		NeedRedraw();
		%Update();
		%cursor = 1;
		return 1;
	}

	function Draw($x, $y) {
		UseFont(%font);
		$height = GetFontHeight();
		$pos = $y - %offsetHeight;
		for ($i=%offset.line; $i<size(%text) && $pos < DHEIGHT; $i++) {
			DisplayText(%text[$i],$x,$pos);
			$pos += TextSize(%text[$i])[1];
		}
		if (%cursor) {
			InvertRect(%cursorPx[0]+$x, $y+%cursorPx[1], %cursorPx[0]+$x, $y+$height+%cursorPx[1]-1);
		}
		if (%selPx[0] != %cursorPx[0] || %selPx[1] != %cursorPx[1]) {
			$start = %cursorPx;
			$end = %selPx;
			if ($start[1] > $end[1] ||
				($start[1] == $end[1] && $start[0] > $end[0])) {
				$start = $end;
				$end = %cursorPx;
			}
			// Optimization for very large highlighted regions.
			if ($start[1] < 0) {
				$start = list(0, 0);
			}
			if ($end[1] > 60) {
				$end = list(%width, 60);
			}
			$lineStart = $start[0];
			$line = $start[1];
			while ($line < $end[1]) {
				$lineEnd = %width;
				InvertRect($x + $lineStart, $y + $line, $x + $lineEnd, $y - 1 + ($line += $height));
				$lineStart = 0;
			}
			$lineEnd = $end[0];
			InvertRect($x + $lineStart, $y + $line, $x + $lineEnd, $y + $line + $height-1);
		}

	}
};

