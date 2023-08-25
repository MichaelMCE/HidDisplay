#import <constants.h>
#requires <util\Text.c>
#requires <Modules\LineEditor.c>
#requires <framework\Overlay.c>

struct FileBrowser {
	var %path, %sel, %hasFocus, %stealArrows, %font, %selInfo, %pathName;
	var %search, %searchTime;
	var %fileList, %fileCount, %selRoot;

	var %run, %runHistory, %runHistoryItem;
	var %rename, %renameName;
		
	function FileBrowser ($_path, $_stealArrows, $_pathName) {
		%pathName = $_pathName;
		%path = $_path;
		if (size(%pathName)) {
		//	%path = GetString("File Browser", %pathName, $_path);
		//	$file = GetString("File Browser", %pathName +s " file");
		}
		if (size(%path) == 0 || %path[1] != ':' || %path[size(%path)-1] != "\"){
		//	%path = "C:\";
			
		}else if (size($file)) {
			$files = FileInfo(%path +s "*");
			for ($i=0; $i<size($files); $i++) {
				if ($files[$i].name ==s $file) {
					%sel = $i;
					break;
				}
			}
		}
		%stealArrows = $_stealArrows;
		%font = Font("Lucida Sans Unicode", 32,0,0,0,CLEARTYPE_QUALITY);
		%fileList = list();
		%fileCount = 0;
		%selRoot = 0;
	}

	function Focus() {
		if (!%hasFocus) {
			RegisterKeyEvent(0, VK_VOLUME_DOWN, VK_VOLUME_UP);
			if (%stealArrows) {
				RegisterKeyRange(3<<16, VK_0, VK_Z);
				RegisterKeyEvent(3<<16, VK_BACK, VK_TAB);
				RegisterKeyRange(3<<16, VK_SPACE, VK_HELP);
				RegisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
				RegisterKeyRange(3<<16, 0xBA, 0xC0);
				RegisterKeyRange(3<<16, 0xDB, 0xDF);
				RegisterKeyRange(3<<16, 0xE1, 0xE4);
				RegisterKeyEvent(3<<16, VK_RETURN, VK_ESCAPE);
				RegisterKeyRange(3<<16, VK_F1, VK_F12);
			}
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		%run = null;
		%rename = null;
		%search = "";
		if (%hasFocus) {
			UnregisterKeyEvent(0, VK_VOLUME_DOWN, VK_VOLUME_UP);
			%hasFocus = 0;
			if (%stealArrows) {
				UnregisterKeyRange(3<<16, VK_0, VK_Z);
				UnregisterKeyEvent(3<<16, VK_BACK, VK_TAB);
				UnregisterKeyRange(3<<16, VK_SPACE, VK_HELP);
				UnregisterKeyRange(3<<16, VK_NUMPAD0, VK_DIVIDE);
				UnregisterKeyRange(3<<16, 0xBA, 0xC0);
				UnregisterKeyRange(3<<16, 0xDB, 0xDF);
				UnregisterKeyRange(3<<16, 0xE1, 0xE4);
				UnregisterKeyEvent(3<<16, VK_RETURN, VK_ESCAPE);
				UnregisterKeyRange(3<<16, VK_F1, VK_F12);
			}
		}
	}
	
	function Show (){
		%fileList = %getFiles(%path);
		%fileCount = size(%fileList);
	}
	
	
	function KeyDown ($event, $param, $modifiers, $vk, $string) {
		$vk = MediaFlip($vk);
		NeedRedraw();
		if ($vk == VK_ESCAPE) {
			// Takes care of rename and run dialogs.
			%G15ButtonDown(, , 0x8);
			return 1;
		}
		if (!IsNull(%rename)) {
			if ($vk == VK_RETURN) {
				$text = %rename.GetText();
				if (size($text)) {
					if (MoveFile(%path +s %renameName, %path +s $text)) {
						%G15ButtonDown(, , 0x8);
						KeyDown(, , , $vk, $string);
					}
				}
				%G15ButtonDown(, , 0x8);
			}else{
				%rename.KeyDown(@$);
			}
			return 1;
		}
		if (!IsNull(%run)) {
			if ($vk == VK_RETURN) {
				$text = %run.GetText();
				if (size($text)) {
					$temp = RegExp($text, "^\w*|"([^|"]*)|"\w*(.*)$");//"
					if (!size($temp[0][0])) {
						$temp = RegExp($text, "^\w*([^\w]*)\w*(.*)$");
					}
					if (size($temp[0][0])) {
						if (!Run($temp[0][0], $temp[1][0], 2))
							RunSimple($temp[0][0], $temp[1][0], 2);
						%runHistory = GetString("File Browser", "Run History", "le");
						if (IsString(%runHistory) && IsList(%runHistory = Bedecode(%runHistory))) {
							%runHistory[size(%runHistory)] = $text;
							if (size(%runHistory) > 50) {
								pop_range(%runHistory,0,0);
							}
							SaveString("File Browser", "Run History", BencodeExact(%runHistory), 1);
						}
					}
				}
				%G15ButtonDown(, , 0x8);
			}
			else if ($vk == VK_UP) {
				if (%runHistoryItem) {
					%run.SetText(%runHistory[--%runHistoryItem]);
				}
			}
			else if ($vk == VK_DOWN) {
				if (%runHistoryItem < size(%runHistory)-1) {
					%run.SetText(%runHistory[++%runHistoryItem]);
				}
				else {
					$text = %run.GetText();
					if (size($text)) {
						%runHistory[%runHistoryItem++] = $text;
						%run.SetText("");
					}
				}
			}
			else
				%run.KeyDown(@$);
			return 1;
		}
		if (($modifiers & 2) || $vk == VK_F2) {
			if ($vk == VK_V) {
				// Not the right way to do it.  Can't find out how without menus and InvokeCommand,
				// which would pop up a dialog on overwrite, which I don't want.
				/*
				$files = GetClipboardData(CF_HDROP);
				if ($files[0] == CF_HDROP) {
					$files = strsplit($files, "|r|n");
					for ($i=0; $i<size($files); $i++) {
					}
				}
				//*/
			}else if (!IsString(%selInfo) && %selInfo.name !=s "..") {
				if ($vk == VK_R) {
					%runHistory = GetString("File Browser", "Run History", "le");
					if (!IsString(%runHistory) || !IsList(%runHistory = Bedecode(%runHistory))) {
						%runHistory = null;
					}
					%runHistoryItem = size(%runHistory);

					%run = LineEditor(800-16, %font);
					%run.Focus();
					$text = %path +s %selInfo.name;
					if (strstr($text, " ")) $text = "|"" +s $text +s "|"";
					%run.SetText($text);
				}
				else if ($vk == VK_F2) {
					%rename = LineEditor(800-16, %font);
					%rename.SetText(%selInfo.name);
					%rename.Focus();
					%renameName = %selInfo.name;
				}
			}
			return 1;
		}
		if ($vk == VK_DELETE || $vk == VK_BACK) {
			%search = "";
			MessageBoxOverlay(eventHandler, "Are you sure you want to delete|n" +S %selInfo.name +S "?",
								"Yes", "No", "DeleteFile",,,,list(%path +s %selInfo.name));
			return 1;
		}
		if (size($string) && $vk != VK_RETURN) {
			if (GetTickCount() - %searchTime >= 1400) {
				%search = "";
			}
			%searchTime = GetTickCount();
			%search +=s $string;
			$files = %fileList;/* = %getFiles(%path);
			%fileCount = size(%fileList);*/
			$index = %sel;

			while (1) {
				if (IsString($files[$index])) {
					$name = $files[$index];
				}else{
					$name = $files[$index].name;
				}
				if (substring($name, 0, size(%search)) ==s %search) break;
				$index = ($index + 1) % %fileCount;
				if ($index == %sel) break;
			}
			%sel = $index;
			return 1;
		}
		%search = "";
		
		if ($vk == VK_VOLUME_UP || $vk == VK_DOWN) {
			%sel++;
			if (%sel > %fileCount)
				%sel = %fileCount-1;
			if (%sel < 0)
				%sel = 0;
		}else if ($vk == VK_VOLUME_DOWN || $vk == VK_UP) {
			%sel--;
			if (%sel < 0)
				%sel = 0;
			
		}else if ($vk == VK_NEXT || $vk == VK_RIGHT) {
			//%fileCount = size(FileInfo(%path +s "*")) + size(DriveList());
			%sel += 5;
			if (%sel >= %fileCount && %sel < %fileCount + 4)
				%sel = %fileCount - 1;
				
		}else if ($vk == VK_PRIOR || $vk == VK_LEFT) {
			if (%sel && %sel < 5) {
				%sel = 0;
			}else {
				%sel -= 5;
			}
		}else if ($vk == VK_HOME) {
			%sel = 0;
		}else if ($vk == VK_END) {
			%sel = -1;
			
		}else if (($vk == VK_RETURN) && !IsNull(%selInfo)) {
			if (IsString(%selInfo)){	// a drive letter
				if (%selInfo ==s %path){	// close drive tree if reselected
					%selInfo = "";
					%selRoot = %sel;
				}else{
					%selRoot = %sel+1;
				}
				%sel = 0;
				%path = %selInfo;
				%fileCount = 0;  // force file list refresh
			}else if (%selInfo.attributes & FILE_ATTRIBUTE_DIRECTORY) { // a directory
				%fileCount = 0;  // force file list refresh
				if (%selInfo.name !=s "..") {
					%sel = 0;
					%path = %path +s %selInfo.name +s "\";
				}else {
					$match = RegExp(%path, "^(.*\\)(.*)\\$");
					$newPath = $match[0][0];
					$oldDir = $match[1][0];
					if (IsString($newPath)) {
						%path = $newPath;
						$files = FileInfo(%path +s "*");
						%sel = 0;
						for ($i=0; $i<size($files); $i++) {
							if ($files[$i].name ==S $oldDir) {
								%sel = $i;
								break;
							}
						}
					}
				}
			}else { // a file
				if (size(%pathName)) {
					SaveString("File Browser", %pathName, %path);
					SaveString("File Browser", %pathName +s " file", %selInfo.name, 1);
				}
				return list(%path, %path +s %selInfo.name);
			}
		}
		return 1;
	}

	function G15ButtonDown($event, $param, $buttons) {
		%search = "";
		if ($buttons & 0xF) {

			if (!IsNull(%run)) {
				%run.Unfocus();
				%run = null;
				NeedRedraw();
				%runHistory = null;
				return 1;
			}
			if (!IsNull(%rename)) {
				%rename.Unfocus();
				%rename = null;
				%renameName = null;
				return 1;
			}

			$state = G15GetButtonsState();
			if ($buttons & 3) {
				if (!($state & ($buttons^3))) {
				}
			}
		}
	}

	function getFiles ($PATH) {
		$files = list();
		$drives = DriveList();
		$fileInfo = FileInfo($PATH +s "*");
		$idx = 0;

		for ($i = 0; $i < size($drives); $i++){
			if ((substring($drives[$i],0,1) ==s substring($PATH,0,1)) && !IsNull($PATH)){
				$files[$idx++] = $PATH;
				for ($j = 0; $j < size($fileInfo); $j++){
					$files[$idx++] = $fileInfo[$j];
				}
			}else{
				$files[$idx++] = $drives[$i];
			}
		}
		return $files;
	}
	
	function Draw ($x, $y) {
		$_W = 800; $_H = 480;

		if (%fileCount < 1){
			%fileList = %getFiles(%path);
			%fileCount = size(%fileList);
			%sel = %selRoot;
		}
		$files = %fileList;
		
		if (!IsNull(%rename)) {
			for ($i=0; $i < %fileCount; $i++) {
				if (%renameName ==s $files[$i].name) {
					%sel = $i;
					break;
				}
			}
			if ($i == %fileCount) {
				%G15ButtonDown(, , 0x8);
			}
		}
		
		UseFont(%font);
		$height = GetFontHeight(%font);
		
		if (%sel >= %fileCount)
			%sel = %fileCount-1;
		if (%sel < 0)
			%sel = 0;

		$index = 0;
		if (%sel >= ($_H/$height)-1)
			$index = (%sel - ($_H/$height)) + 1;
		
		$pos = 0;
		$tw = 0;
		$hlColor = RGB(45,45,45);
		$hlColori = RGB(220,20,20);
		$blue   = 0x508DC5;			// blue hint
		$purple = 0xA80072;			// purple hint
		$yellow = 0xA87200;			// yellow hint
		$green  = 0x007200;			// green hint
		$xoffset = 16;
		
		while ($pos < $_H && $index < %fileCount){
			if (!IsNull(%rename) && $index == %sel){
				%rename.Draw(0, $pos);
			}else {
				if (IsString($files[$index])) {
					$name = $files[$index];
					if ($name[0] >s "B") {
						$info = VolumeInformation($name);
						//$name = /*"|2" +s*/ $name;
						if (size($info.volumeName)){
							$name = $name +s "|t["/*|2"*/ +s $info.volumeName +s /*"|2*/"]";
						}
					}
					if ($index == %sel && %hasFocus){
						$tw = TextSize($name)[0]+1;
						DrawRectangleFilled(0, $pos+2, $tw+1, $pos+$height+3, $hlColor);
						$colour = $hlColori;
					}else{
						$colour = $blue;
					}
					DisplayTextShadowed($name, 2, $pos, $colour, RGBA(255,255,255,240));
					
				}else if ($files[$index].attributes & FILE_ATTRIBUTE_DIRECTORY){
					if ($index == %sel && %hasFocus){
						$tw = TextSize($files[$index].name)[0] + $xoffset;
						DrawRectangleFilled($xoffset, $pos+2, $tw+1, $pos+$height+3, $hlColor);
						$colour = $hlColori;
					}else{
						$colour = $purple;
					}
					
					DisplayTextShadowed($files[$index].name, $xoffset+2, $pos, $colour, RGBA(255,255,255,240));
				}else{
					$tw = TextSize($files[$index].name)[0] + $xoffset;
					if ($index == %sel && %hasFocus) {
						DrawRectangleFilled($xoffset, $pos+2, $tw+1, $pos+$height+3, $hlColor);
						$colour = $hlColori;
					}else{
						$colour = $yellow;
					}

					DisplayTextShadowed($files[$index].name, $xoffset+2, $pos, $colour, RGBA(255,255,255,240));
					DisplayTextShadowed("(" +s FormatSize($files[$index].bytes,,,3) +s ")", $tw+11, $pos, $green, RGBA(255,255,255,240));
				}

				/*if ($index == %sel && %hasFocus) {
					if (IsNull(%run)) {
						InvertRect(0, $pos, $tw+1, $pos+$height-1);
					}
				}*/
			}
			$pos += $height;
			$index++;
		}
		%selInfo = $files[%sel];
		if (!IsNull(%run)) {
			%run.DrawBox("Run:");
		}
	}
};

