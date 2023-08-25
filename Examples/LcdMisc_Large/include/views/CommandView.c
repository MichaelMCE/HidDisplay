// Not complete.

#import <constants.h>
#import <Views\View.c>
#requires <Modules\TextEditor.c>

struct CommandScreen {
	var %path,
		%editor,
		%processInfo,
		%searchString,
		%searchIndex;
	function CommandScreen($_font) {
		%path = "C:\";//"
		%editor = TextEditor(160, $_font);
	}
};

struct CommandView extends View {
	var %screens,
		%current,
		%font;

	function CommandView() {
		%toolbarImage = LoadImage("Images\Command.png");
		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
		%current = 0;
		%font = Font("04b03", 8);
		%screens = list(CommandScreen(%font));
	}

	function KeyDown($event, $param, $modifiers, $vk) {
		if (IsNull(%screens[%current].processInfo)) {
			$editor = %screens[%current].editor;
			$path = %screens[%current].path;
			if ($vk == VK_TAB) {
				$[2] = 0;
				$[4] = "";
				$text = strreplace($editor.text[size($editor.text)-1], "|n");
				$pos = $editor.cursorPos.pos;
				$p = 0;
				while (1) {
					$temp = RegExp($text, "^\w*(|"[^|"]*|")()", $p);//"
					if (!size($temp)) {
						$temp = RegExp($text, "^\w*([^|"]*)()", $p);//"
						if (!size($temp)) {
							// Open quote.
							$temp = RegExp($text, "^\w*(.*)()");
						}
					}
					$v = $temp[0];
					$p = $temp[1][1];
					while (1) {
						$temp2 = RegExp($text, "^(|"[^|"]*|")()", $p);//"
						if (!size($temp2)) {
							$temp2 = RegExp($text, "^([^|"]*)()", $p);//"
							if (!size($temp2[0][0])) break;
						}
						$p = $temp2[1][1];
						$v[0] +=s $temp2[0][0];
					}
					$temp = RegExp($text, "^()\w*()", $p);
					$temp[0] = $v;
					$p = $temp[1][1];
					if ($p > $pos || $p == size($text)) {
						break;
					}
				}
				if (size(%screens[%current].searchString)) {
					$item = %screens[%current].searchString;
					$index = %screens[%current].searchIndex;
				}
				else {
					%screens[%current].searchString = $item = $temp[0][0];
					$index = -1;
				}
				$temp2 = RegExp($item, "^|".*|"$");
				$item = strreplace($item, "|"");//"
				if ($item[0] !=s "\" && //"
					$item[1] !=s ":") {
						$addedPath = 1;
						$item = $path +s $item;
				}
				$items = FileInfo($item +s "*");
				if ($items[0].name ==s "..") {
					pop($items, 0);
				}
				if (size($items)) {
					%screens[%current].searchIndex =
						$index = ($index+1) % size($items);
					if ($addedPath) $item = substring($item, size($path), -1);
					$temp2 = RegExp($item, "(.*\\).*");
					if (size($temp2)) {
						$item2 = $temp2[0][0];
						WriteLogLn($item2);
					}
					$item2 +=s $items[$index].name;
					if (!IsNull(strstr($item2, " "))) {
						$item2 = "|"" +s $item2 +s "|"";
					}
					$start = $temp[0][1];
					$end = $start + size($temp[0][0]);
					$[3] = VK_LEFT;
					while ($editor.cursorPos.pos > $start) {
						$editor.KeyDown(@$);
					}
					$[3] = VK_RIGHT;
					while ($editor.cursorPos.pos < $start) {
						$editor.KeyDown(@$);
					}
					$[2] = 1;
					$[3] = VK_LEFT;
					while ($editor.cursorPos.pos > $end) {
						$editor.KeyDown(@$);
					}
					$[3] = VK_RIGHT;
					while ($editor.cursorPos.pos < $end) {
						$editor.KeyDown(@$);
					}
					$[2] = 0;
					$[3] = VK_A;
					$[4] = $item2;
					$editor.KeyDown(@$);
				}
			}
			else {
				%screens[%current].searchString = "";
				if ($vk == VK_RETURN) {
					WriteLogLn("Goat?");
					$text = strreplace($editor.text[size($editor.text)-1], "|n");
					$[2] = 0;
					$[4] = "";
					$editor.KeyDown(@$);
					$test = RegExp($text, "^\w*|"([^|"]*)|"\w*(.*?)\w*$");//"
					if (!size($test)) {
						$test = RegExp($text, "^\w*([^\w]*)\w*(.*?)\w*$");
					}
					$file = $test[0][0];
					$params = $test[1][0];
					if (!size($file)) return 1;
					if ($file[0] !=s "\" && //"
						$file[1] !=s ":") {
							$file = $path +s $file;
					}
					$process = Run($file, $params, 3, $path);
				}
				else if (!($modifiers & 0x6) || $vk == VK_C || $vk == VK_X || $vk == VK_V) {
					if ($vk == VK_LEFT && !$editor.cursorPos.pos) return 1;
					else if ($vk == VK_UP) {
						$line = $editor.cursorPos.line;
						$editor.KeyDown(@$);
						if ($line != $editor.cursorPos.line) {
							$[3] = VK_DOWN;
							$editor.KeyDown(@$);
							$[3] = VK_HOME;
							$editor.KeyDown(@$);
						}
					}
					else {
						if ($vk == VK_UP && !$editor.cursorPos.line) {
							$[3] == VK_HOME;
						}
						else if ($vk == VK_PRIOR) {
							$[3] == VK_HOME;
						}
						$editor.KeyDown(@$);
					}
				}
			}
			//IsNull(%screens[%current].processInfo.KeyDown
			NeedRedraw();
			return 1;
		}
		%screens[%current].searchString = "";
		if ($vk == VK_ESCAPE) {
			%Unfocus();
			NeedRedraw();
			return 1;
		}
		/*
		$res = %browser.KeyDown(@$);
		if ($vk == VK_RETURN) {
			if (IsList($res)) {
				%LoadFile(@$res);
			}
		}
		return 1;
		//*/
	}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 0xF) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
			}
			else {
				if (!($state & ($buttons^12))) {
					if ($buttons & 8) {
						%Unfocus();
					}
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function Focus() {
		if (!%hasFocus) {
			%screens[%current].editor.Focus();
			RegisterKeyEvent(3<<16, VK_ESCAPE);
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%screens[%current].editor.Unfocus();
			UnregisterKeyEvent(3<<16, VK_ESCAPE);
			%hasFocus = 0;
		}
	}

	function Draw() {
		ClearScreen();

		UseFont(%font);
		%screens[%current].editor.Draw();
	}
};
