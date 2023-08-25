#import <constants.h>
#requires <util\Text.c>

struct UndoInfo {
	var %start, %end, %string, %next;
	function UndoInfo($_start, $_end, $_string, $_next) {
		%start = $_start;
		%end = $_end;
		%string = $_string;
		%next = $_next;
	}
};

/* Registers key listener, but doesn't handle enter.
 * Takes care of basically all the work. Must be unfocused
 * before destruction.
 */
struct LineEditor {
	var %text, %cursorPos, %selStart, %hasFocus, %flashTimer, %cursor, %textSize, %offset,
		%font, %width,
		%cursorPx, %selPx, %height, %startPx,
		%undo, %redo, %splitUndo;

	function LineEditor($_width, $_font) {
		%text = "";
		%font = $_font;
		%width = $_width;
		%Update();
	}

	function ChangeDrawParams($_width, $_font) {
		%font = $_font;
		%width = $_width;
		%Update();
	}

	function Update() {
		UseFont(%font);
		$temp = TextSize(substring(%text, 0, %cursorPos));
		%cursorPx = $temp[0]-1;
		%height = $temp[1];
		if (%cursorPos != %selStart) {
			%selPx = TextSize(substring(%text, 0, %selStart))[0]-1;
		}
		if (%cursorPx < %startPx)
			%startPx = %cursorPx;
		else if (%cursorPx >= %startPx + %width) {
			%startPx = %cursorPx - %width+1;
		}

	}

	function ReFormat($_font, $_width) {
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
		%selStart = %cursorPos = size(%text);
		%startPx = -1;
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
			RegisterKeyEvent(3<<16, VK_BACK, VK_TAB);
			RegisterKeyRange(3<<16, VK_SPACE, VK_HELP);
			RegisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
			RegisterKeyRange(3<<16, 0xBA, 0xC0);
			RegisterKeyRange(3<<16, 0xDB, 0xDF);
			RegisterKeyRange(3<<16, 0xE1, 0xE4);
			RegisterKeyEvent(3<<16, VK_RETURN);
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
			UnregisterKeyEvent(3<<16, VK_BACK, VK_TAB);
			UnregisterKeyRange(3<<16, VK_SPACE, VK_HELP);
			UnregisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
			UnregisterKeyRange(3<<16, 0xBA, 0xC0);
			UnregisterKeyRange(3<<16, 0xDB, 0xDF);
			UnregisterKeyRange(3<<16, 0xE1, 0xE4);
			UnregisterKeyEvent(3<<16, VK_RETURN);
			%hasFocus = 0;
		}
	}

	// Responsible for all string modifications - insertions, deletions,
	// and substitutions.
	function ReplaceSelected($string) {
		$start = %cursorPos;
		$end = %selStart;
		if ($start > $end) {
			$start = $end;
			$end = %cursorPos;
		}
		$string = strreplace($string, "|n", "");
		$string = strreplace($string, "|r", "");
		if (%cursorPos == %selStart && !size($string)) return;
		$temp = strswap(%text, $string, $start, $end);
		%text = $temp[0];
		%selStart = %cursorPos = $start + size($string);
		if (!IsNull(%undo) && !%splitUndo) {
			if (%undo.end != %undo.start) {
				if ($start == %undo.end && size($temp[1]) == 0) {
					// Normal typing.
					%undo.end += size($string);
					return;
				}
			}
			else if (!size($string)) {
				// Backspace.
				if ($end == %undo.start) {
					%undo.start--;
					%undo.end--;
					%undo.string = $temp[1] +s %undo.string;
					return;
				}
				// Delete
				if ($start == %undo.start) {
					%undo.string = %undo.string +s $temp[1];
					return;
				}
			}
		}
		%undo = UndoInfo($start, %cursorPos, $temp[1], %undo);
		%splitUndo = 0;
		%redo = null;
	}

	function Undo($reallyRedo) {
		if ($reallyRedo)
			$_undo = %redo;
		else
			$_undo = %undo;
		if (!IsNull($_undo)) {

			$start = $_undo.start;

			$temp = strswap(%text, $_undo.string, $start, $_undo.end);

			%text = $temp[0];
			%selStart = %cursorPos = $start + size($_undo.string);

			$temp = UndoInfo($start, %cursorPos, $temp[1]);

			if ($reallyRedo) {
				%redo = %redo.next;
				$temp.next = %undo;
				%undo = $temp;
			}
			else {
				%undo = %undo.next;
				$temp.next = %redo;
				%redo = $temp;
			}
		}
	}

	function NextChar($pos) {
		if ($pos < size(%text)) {
			$pos ++;
			$hex = ParseBinaryInt(%text, $pos, 1, 1);
			while ($pos < size(%text) && $hex >= 0x80 && $hex < 0xC0) {
				$pos ++;
				$hex = ParseBinaryInt(%text, $pos, 1, 1);
			}
		}
		return $pos;
	}

	function PrevChar($pos) {
		if ($pos) {
			$pos --;
			$hex = ParseBinaryInt(%text, $pos, 1, 1);
			while ($pos && $hex >= 0x80 && $hex < 0xC0) {
				$pos --;
				$hex = ParseBinaryInt(%text, $pos, 1, 1);
			}
		}
		return $pos;
	}

	// Complete text edit control behavior...
	function KeyDown($event, $param, $modifiers, $vkey, $string) {
		if (($modifiers & 12)) return;
		if (!($modifiers & 2)) {
			$processed = 1;
			if ($vkey == VK_BACK || $vkey == VK_DELETE) {
				if (%selStart == %cursorPos) {
					if ($vkey == VK_DELETE) {
						%selStart = %NextChar(%cursorPos);
					}
					else {
						%cursorPos = %PrevChar(%cursorPos);
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
			else if ($vkey == VK_DOWN || $vkey == VK_END || $vkey == VK_NEXT) {
				%cursorPos = size(%text);
			}
			else if ($vkey == VK_UP || $vkey == VK_HOME || $vkey == VK_PRIOR) {
				%cursorPos = 0;
			}
			else return;
			if (!($modifiers & 1)) {
				%selStart = %cursorPos;
			}
		}
		else {
			$start = %cursorPos;
			$end = %selStart;
			if ($start > $end) {
				$start = $end;
				$end = %cursorPos;
			}
			if ($vkey == VK_A) {
				%cursorPos = size(%text);
				%selStart = 0;
			}
			else if ($vkey == VK_C) {
				if ($start == $end) return;
				SetClipboardData(substring(%text, $start, $end));
			}
			else if ($vkey == VK_X) {
				if ($start == $end) return;
				SetClipboardData(substring(%text, $start, $end));
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
		$pos = $x-%startPx;
		DisplayText(%text, $pos, $y);
		if (%selStart != %cursorPos) {
			$first = %selStart;
			$last = %cursorPos;
			$firstPx = %selPx;
			if ($first > $last) {
				$first = $last;
				$last = %selStart;
				$firstPx = %cursorPx;
			}

			SetDrawColor(colorHighlightText);
			SetBgColor(colorHighlightBg);
			ClearRect(%cursorPx+$pos, $y, $pos + %selPx, $y+%height-1);
			DisplayText(substring(%text, $first, $last), $pos + $firstPx, $y);
			if (%cursor) {
				XorRect(%cursorPx+$pos, $y, %cursorPx+$pos, $y+%height-1, colorHighlightText^colorHighlightBg);
			}
			SetDrawColor(colorText);
			SetBgColor(colorBg);
		}
		else {
			if (%cursor)
				DrawRect(%cursorPx+$pos, $y, %cursorPx+$pos, $y+%height-1);
		}
	}

	// currently only work with widths <= 154 or so
	function DrawBox($_text) {
		$_W = 479;
		$_H = 272;
		
		$s = FormatText($_text);
		$h = $s[1];
		UseFont(%font);
		$fh = GetFontHeight();
		$totalH = $fh + $h + 23;
		$top = 21-$totalH/2;
		DrawRect(0, $top, $_W, $top);
		$bottom = $totalH+$top-1;
		ClearRect(0, $top+1, $_W, $bottom-1);
		DrawRect(0, $bottom, $_W, $bottom);

		$start = ($_W/2)-%width/2;
		$end = $start + %width-1;

		$bottom -= 2;

		DrawRect($start-2, $bottom-$fh-5, $end+2, $bottom-$fh-5);
		DrawRect($start-2, $bottom-2, $end+2, $bottom-2);

		DisplayTextCentered($_text, $_W/2, $top+3);
		%Draw($start, $bottom-$fh-3);
		ClearRect(0, $bottom-$fh-3, $start-1, $bottom-3);
		ClearRect($end+1, $bottom-$fh-3, $_W, $bottom-3);

		DrawRect($end+2, $bottom-$fh-4, $end+2, $bottom-3);
		DrawRect($start-2, $bottom-$fh-4, $start-2, $bottom-3);
		DrawRect(0, $top, 0, $bottom);
		DrawRect($_W, $top, $_W, $bottom);
	}
};

