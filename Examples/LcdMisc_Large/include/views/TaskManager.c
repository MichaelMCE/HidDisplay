#import <Views\View.c>
#requires <list.c>
#requires <framework\Overlay.c>
#requires <util\Text.c>

struct TaskManager extends View {
	var %smallFont, %bigFont;
	// Counters
	var %cpu, %mem, %pid;
	// Cached selected item info.
	var %selectedIndex, %selectedPid, %selectedName;
	// Sort mode.
	var %sort;
	// Sort direction.
	var %dir;
	// Entries per page.
	var %page;
	// Entry count.  Only used for page downs.
	var %processes;

	var %keySteal;
	var %background;
	var %search;
	var %searchTime;

	var %indices,
		%cpuList,
		%memList,
		%pidList,
		%nameList;

	var %expanded,
		%expandedIndex,
		%selectedHwnd,
		%selectedHwndIndex;
		
	var %namefont;

	function TaskManager($noKeySteal, $_smallFont, $_bigFont) {
		%smallFont = $_smallFont;
		%bigFont = $_bigFont;
		%selectedHwndIndex = -1;
		%keySteal = !$noKeySteal;
		%toolbarImage = ImageLoad("process1.png");
		%toolbarImageL = ImageLoad("process2.png");
		%background = ImageLoad("ibottom.png");
		
		%sort = GetString("Task Manager", "Sort") % 3;
		%dir = GetString("Task Manager", "Direction");
		%cpu = PerformanceCounter("Process", "*", "% Processor Time", 1);
		%mem = PerformanceCounter("Process", "*", "Working Set", 1);
		%pid = PerformanceCounter("Process", "*", "ID Process", 1);
		%cpu.SetAutoUpdate(0);
		%mem.SetAutoUpdate(0);
		%pid.SetAutoUpdate(0);

		%noDrawOnAudioChange = 1;
		%namefont = Font("Arial", 52,0,0,0,CLEARTYPE_QUALITY);
	}


	function KeyDown($event, $param, $modifiers, $vk, $string) {
		$vk = MediaFlip($vk);
		if (%hasFocus) {
			if ($vk == VK_DELETE || $vk == VK_BACK || $vk == VK_RETURN) {
				if (%selectedHwndIndex == -1) {
					MessageBoxOverlay(%eventHandler, "Terminate Process:|n" +S %selectedName +S "?",
										"Yes", "No", "Kill",,$this,,list(%selectedPid));
				}else if (%selectedHwnd > 0) {
					$text = GetWindowText(%selectedHwnd);
					if (!size($text)) $text = "No Name";
					MessageBoxOverlay(%eventHandler, "Close Window:|n" +S $text +S "?",
										"Yes", "No", "Close",,$this,,list(%selectedHwnd, %selectedPid));
				}
				%search = "";
				return 1;
			}
			
			if ($vk >= VK_1 && $vk <= VK_6) {
				SetProcessPriority(%selectedPid, $vk-VK_1);
				NeedRedraw();
				%search = "";
				return 1;
			}
			
			if (size($string)) {
				if (GetTickCount() - %searchTime >= 1400) {
					%search = "";
				}
				%searchTime = GetTickCount();
				%search +=s $string;
				$index = %selectedIndex;
				while (1) {
					if (substring(%nameList[%indices[$index]], 0, size(%search)) ==s %search) break;
					$index = ($index + 1) % size(%indices);
					if ($index == %selectedIndex) break;
				}
				%selectedIndex = $index;

				%selectedPid = -1;
				%selectedName = "";
				NeedRedraw();
				return 1;
			}
			%search = "";
			if ($vk == VK_VOLUME_UP || $vk == VK_DOWN) {
				if (%expandedIndex == %selectedIndex) {
					$hWnds = %GetWindowDict()[%pidList[2*%indices[%selectedIndex]+1]];
					%selectedHwndIndex++;
					if (%selectedHwndIndex == size($hWnds)) {
						%selectedHwndIndex = -1;
						%selectedIndex++;
					}
				}
				else {
					%selectedIndex++;
				}
				%selectedHwnd = 0;
				if (%selectedIndex == size(%indices)) {
					%selectedIndex = 0;
				}
			}else if ($vk == VK_VOLUME_DOWN || $vk == VK_UP) {
				if (%selectedHwndIndex >= 0) {
					%selectedHwndIndex --;
				}
				else {
					%selectedIndex--;
					if (%selectedIndex < 0) {
						%selectedIndex = size(%indices)-1;
					}
					if (%expandedIndex == %selectedIndex) {
						$hWnds = %GetWindowDict()[%pidList[2*%indices[%selectedIndex]+1]];
						if (size($hWnds)) {
							%selectedHwndIndex = size($hWnds)-1;
						}
					}
				}
				%selectedHwnd = 0;
			}else if ($vk == VK_PRIOR) {
				for ($i=0; $i<%page; $i++) {
					$index = %selectedIndex;
					%KeyDown($event,,, VK_UP);
					if (!%selectedIndex && %selectedHwndIndex < 0) break;
					if ($index < %selectedIndex) break;
				}
			}else if ($vk == VK_NEXT) {
				for ($i=0; $i<%page; $i++) {
					$index = %selectedIndex;
					$hwndIndex = %selectedHwndIndex;

					%KeyDown($event,,, VK_DOWN);
					if ($index > %selectedIndex) {
						if ($i) {
							%selectedIndex = $index;
							%selectedHwndIndex = $hwndIndex;
						}
						break;
					}
				}
			}else if ($vk == 0xB2) {
				%selectedIndex = 0;
			}else if ($vk == VK_HOME) {
				%selectedIndex = 0;
			}else if ($vk == VK_END) {
				%selectedIndex = 0;
				%KeyDown($event,,, VK_UP);
			}else if ($vk == VK_LEFT) {
				if (%expanded == %selectedPid) {
					if (%selectedHwndIndex >= 0) {
						%selectedHwndIndex = -1;
						%selectedHwnd = 0;
					}
					else {
						%expanded = 0;
						%expandedIndex = 0;
					}
				}
			}else if ($vk == VK_RIGHT) {
				if (%expanded != %selectedPid) {
					%expanded = %selectedPid;
					%selectedHwndIndex = -1;
					%selectedHwnd = 0;
				}
			}else {
				if ($vk == 0xB0) {
					%sort = (%sort+1)%3;
					%dir = %sort;
				}
				else if ($vk == 0xB1) {
					%sort = (%sort+2)%3;
					%dir = %sort;
				}
				else if ($vk == 0xB3) {
					%dir = !%dir;
				}
				else return;

				SaveString("Task Manager", "Sort", %sort);
				SaveString("Task Manager", "Direction", %dir);

				// Force redaw.
				%indices = null;
				NeedRedraw();

				return 1;
			}


			%selectedPid = -1;
			%selectedName = "";
			NeedRedraw();
			return 1;
		}
	}

	function Update() {
		%cpu.Update();
		%mem.Update();
		%pid.Update();
	}


	function HandleEvent ($event, $param, $key) {
		if ($event ==s "sbuiEventDKDown"){
			//WriteLogLn($event +s " @ " +s $key, 1);

			if ($key == SBUI_DK_3){	// move down
				%Focus();
				%KeyDown(0,0,0, VK_DOWN);
				%Unfocus();
				//%Update();
				NeedRedraw();
				
			}else if ($key == SBUI_DK_2){	// move up
				%Focus();
				%KeyDown(0,0,0, VK_UP);
				%Unfocus();
				//%Update();
				NeedRedraw();

			}else if ($key == SBUI_DK_9){	// focus proc item
				if (!%hasFocus)
					%Focus();
				else
					%Unfocus();

				//%G15ButtonDown(,,G15_CANCEL);
				NeedRedraw();
				
			}else if ($key == SBUI_DK_10){	// kill process
				if (!%hasFocus){
					%Focus();
				}else if (%selectedPid > 4){
					%Kill(%selectedPid);
					%Unfocus();
				}
				
				NeedRedraw();
			}

			return 1;

		}
		return 0;
	}
			
	function Show () {
		/*
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_UP, "dkUp.png");
		DynamicKeySetImage(SBUI_DK_8, SBUI_DK_DOWN, "dkUpAlt.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_UP, "dkDown.png");
		DynamicKeySetImage(SBUI_DK_3, SBUI_DK_DOWN, "dkDownAlt.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_UP, "dkFocus.png");
		DynamicKeySetImage(SBUI_DK_6, SBUI_DK_DOWN, "dkFocusAlt.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_UP, "dkKill.png");
		DynamicKeySetImage(SBUI_DK_10, SBUI_DK_DOWN, "dkKillAlt.png");*/
		
		%cpu.SetAutoUpdate(1);
		%mem.SetAutoUpdate(1);
		%pid.SetAutoUpdate(1);

		// Volume Up and Down, Media Forward/Back
		//%Update();
		//Sleep(10);
		%Update();
	}

	function Hide() {
		if (%hasFocus){
			$alist = list();
			$alist[0] = G15_CANCEL;
			PostEvent("G15ButtonDown", 1, $alist);
		}
		
		// Save a k of memory.  Oh boy!
		%indices = null;
		%cpuList = null;
		%memList = null;
		%pidList = null;
		%nameList = null;
		
		%cpu.SetAutoUpdate(0);
		%mem.SetAutoUpdate(0);
		%pid.SetAutoUpdate(0);
		
		//DynamicKeySetImage(SBUI_DK_8, SBUI_DK_UP, "dk8.png");
		//DynamicKeySetImage(SBUI_DK_8, SBUI_DK_DOWN, "dkAlt8.png");
	}


	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
			// Wheel up/down, next/prev track, and play/pause
			RegisterKeyEvent(0, 0xAE, 0xAF, 0xB0, 0xB1, 0xB3, 0xB2);
			// Characters 1-6.
			RegisterKeyEvent(0, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36);
			if (%keySteal) {
				RegisterKeyRange(1<<16, VK_A, VK_Z);
				RegisterKeyRange(1<<16, VK_LEFT, VK_DOWN);
				RegisterKeyRange(1<<16, VK_PRIOR, VK_HOME);
				RegisterKeyEvent(1<<16, VK_DELETE, VK_BACK, VK_RETURN);
				RegisterKeyRange(1<<16, 0xBA, 0xC0);
				RegisterKeyRange(1<<16, 0xDB, 0xDE);
			}
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
			UnregisterKeyEvent(0, 0xAE, 0xAF, 0xB0, 0xB1, 0xB3, 0xB2);
			UnregisterKeyEvent(0, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36);
			if (%keySteal) {
				UnregisterKeyRange(1<<16, VK_A, VK_Z);
				UnregisterKeyRange(1<<16, VK_LEFT, VK_DOWN);
				UnregisterKeyRange(1<<16, VK_PRIOR, VK_HOME);
				UnregisterKeyEvent(1<<16, VK_DELETE, VK_BACK, VK_RETURN);
				UnregisterKeyRange(1<<16, 0xBA, 0xC0);
				UnregisterKeyRange(1<<16, 0xDB, 0xDE);
			}
		}
	}
	
	function Close ($hWnd, $id) {
		if (GetWindowProcessId($hWnd) == $id) {
			CloseWindow($hWnd);
		}
		NeedRedraw();
	}

	function Kill ($killPid) {
		// No checking if pid now points to a new process.
		// May add later, but pids aren't reused often.
		KillProcess($killPid);
		%Update();
		NeedRedraw();
	}

	function G15ButtonDown($event, $param, $buttons) {
		%search = "";
		if ($buttons & 0x3F) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
				if (!($state & ($buttons^3))) {
					if ($buttons == 1) {
						%KeyDown(0,0,0, VK_PRIOR);
					}
					else {
						%KeyDown(0,0,0, VK_NEXT);
					}
				}
			}
			else if ($buttons & 0x30) {
				if (!($state & ($buttons^0x30))) {
					if ($buttons == 0x10) {
						%KeyDown(0,0,0, VK_UP);
					}
					else {
						%KeyDown(0,0,0, VK_DOWN);
					}
				}
			}
			else {
				if ($buttons & 8) {
					%Unfocus();
				}
				else {
					// Don't try to kill idle, system, or invalid pids.
					if (%selectedPid > 4) {
						%KeyDown(0, 0, 0, VK_DELETE);
					}
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function GetWindowDict() {
		$windows = EnumChildWindows(0,1);
		$hwndDict = dict();
		for ($i=0; $i<size($windows); $i++) {
			$_pid = GetWindowProcessId($windows[$i]);
			if ($_pid) {
				$l = $hwndDict[$_pid];
				if (!size($l)) {
					$l = $hwndDict[$_pid] = list();
				}
				$l[size($l)] = $windows[$i];
			}
		}
		return $hwndDict;
	}

	function Draw ($event, $param, $name, $res) {
		$farRight = $right = $res[0]-1;
		$bottom = $res[1]-1;
		$bpp = $res[2];
		
		//UseFont(%bigFont);
		UseFont(%namefont);
		$farRight --;
		$right -= 4;
		$prefix = 1+TextSize("2")[0];
		$prefix2 = 2*$prefix;


		//ClearScreen();
		DrawImage(%background,0,0);
		
		$height = TextSize("99.9");
		$width = $height[0];
		$width2 = $right-(TextSize("20.22M")[0] + $width+$prefix);

		$height = $height[1]-10;
		if ($height <= 0) return;
		%page = $bottom/$height;
		
		if (%cpu.IsUpdated() || !IsList(%indices)) {
			%cpuList = %cpu.GetValueList();
			%memList = %mem.GetValueList();
			%pidList = %pid.GetValueList();

			%nameList = list();
			$procs = size(%pidList)-2;
			for ($j = $procs; $j>=0; $j-=2) {
				// Used later for normalizing.
				$sum += %cpuList[$j+1];

				%nameList[$j>>1] = %pidList[$j];
				if (%pidList[$j+1] == %selectedPid) {
					$found = 1;
					%selectedIndex = $j>>1;
				}
			}

			%indices = indexstablesort(%nameList);
			if (%sort) {
				if (%sort == 1)
					$sortList = %memList;
				else
					$sortList = %cpuList;
				$temp = list();
				$temp2 = $procs/2;
				for ($j = $temp2; $j>=0; $j--) {
					$temp[$j] = $sortList[2*%indices[$temp2-$j]+1];
				}
				$indices2 = %indices;
				%indices = indexstablesort($temp);
				for ($j = $temp2; $j>=0; $j--) {
					%indices[$j] = $indices2[$temp2-%indices[$j]];
				}
			}

			if (%dir) {
				listFlip(%indices);
			}
			// Generally the last element, but I'm paranoid.
			$_Total = listFind(%nameList, "_Total");
			if ($_Total >= 0) {
				pop(%indices, listFind(%indices, $_Total));

				// Don't want that in the sum.  Don't just divide
				// by total because it's not accurate when killing
				// or quitting apps.
				$sum -= %cpuList[2*$_Total+1];
			}
			if ($sum == 0) $sum = 1;

			// Normalize
			$sum /= 100.0;
			for ($j = $procs; $j>=0; $j-=2) {
				%cpuList[$j+1] /= $sum;
			}

			// Want this first.  Generally starts first anyways, but...
			$idle = listFind(%nameList, "Idle");
			if ($idle >= 0) {
				pop(%indices, listFind(%indices, $idle));
				insert(%indices, 0, $idle);
			}
		}
		else {
			$procs = size(%pidList)-1;
			for ($j = $procs; $j>=0; $j-=2) {
				if (%pidList[$j] == %selectedPid) {
					$found = 1;
					%selectedIndex = $j>>1;
				}
			}
		}
		if ($found) {
			%selectedIndex = listFindInt(%indices, %selectedIndex);
		}else {
			if (%selectedIndex < 0) {
				%selectedIndex = size(%indices)-1;
			}else if (%selectedIndex >= size(%indices)) {
				// Don't wrap if we were on last item and it was closed.
				if (%selectedPid  >= 0){
					%selectedIndex = size(%indices)-1;
				}else {
					%selectedIndex = 0;
				}
			}
		}

		$pos = (%selectedIndex/%page) * %page;
		%processes = size(%indices);
		$hwndDict = %GetWindowDict();

		$i=0;
		$realIndex = %selectedIndex;
		$listPos = %selectedIndex;
		$listLen = size(%indices);

		if (%expanded) {
			for ($p = 0; $p < size(%indices); $p++) {
				$_pid = %pidList[%indices[$p]*2+1];
				if ($_pid == %expanded) {
					%expandedIndex = $p;
					$expandedHwnds = $hwndDict[$_pid];
					break;
				}
			}
			if (!size($expandedHwnds)) {
				%expanded = 0;
				%expandedIndex = 0;
				%selectedHwnd == 0;
				%selectedHwndIndex = -1;
			}else {
				for ($p = 0; $p < size($expandedHwnds); $p++) {
					if (%selectedHwnd == $expandedHwnds[$p]) {
						%selectedHwndIndex = $p;
						break;
					}
				}
				if (%selectedHwndIndex >= 0) {
					if (%selectedHwndIndex >= size($expandedHwnds)) {
						%selectedHwndIndex = size($expandedHwnds)-1;
					}
				}
				%selectedHwnd = $expandedHwnds[%selectedHwndIndex];
				//*/
				if (%expandedIndex < %selectedIndex) {
					$realIndex = size($expandedHwnds) + %selectedIndex;
					$realPos = ($realIndex / %page) * %page - %expandedIndex;
					$pos = %expandedIndex;
					$i = -$realPos * $height;
				}else if (%selectedHwndIndex >= 0) {
					$realIndex = %selectedIndex + 1 + %selectedHwndIndex;
					$realPos = ($realIndex / %page) * %page - %expandedIndex;
					$pos = %expandedIndex;
					$i = -$realPos * $height;
				}//*/
				if ($i > 0) {
					$i -= %page * $height;
					$pos -= %page;
				}
				$listPos = $realIndex;
				$listLen = size(%indices) + size($expandedHwnds);
			}
		}

		if (0 && $bpp>1) {
			ColorRect($right-$width-$prefix, 0, $right-1, $bottom, colorColumnBg[0]);
			ColorRect($width2-3, 0, $right-$width-$prefix+4, $bottom, colorColumnBg[1]);
			ColorRect($prefix2, 0, $width2-3, $bottom, colorFirstColumnBg);
			$width2 -= 2;
			$prefix2++;
		}

		$inv = ($realIndex % %page) * $height;
		if (%hasFocus)
			$bgColor = RGBA(40,40,40,150);
		else if ($bpp > 1)
			$bgColor = RGBA(40,40,40,60);
		else
			$inv = -1;
	
	
		if ($inv >= 0)
			ColorRect(4, $inv+2, $right-1, $inv+$height+4, $bgColor);

		for (;$i<$bottom && $pos < size(%indices); $i+=$height, $pos++) {
			/*if ($inv == $i) {
				if (%hasFocus)
					SetDrawColor(colorHighlightText);
				else
					SetDrawColor(colorHighlightUnfocusedText);
			}else {
				SetDrawColor(colorText);
			}*/
			$index = %indices[$pos];
			//DisplayText(%nameList[$index], 8, $i);
			//ClearRect($width2, $i, 159, $i+$height-1);
			//SetDrawColor(RGBA(0,0,0,200));
			//DisplayText(Elipsisify(%nameList[$index], $width2-$prefix2-1), $prefix2-1, $i-1);
			//DisplayText(Elipsisify(%nameList[$index], $width2-$prefix2-1), $prefix2+1, $i+1);
			//DisplayText(Elipsisify(%nameList[$index], $width2-$prefix2-1), $prefix2+1, $i-1);
			//DisplayText(Elipsisify(%nameList[$index], $width2-$prefix2-1), $prefix2-1, $i+1);

			//SetDrawColor(RGBA(255,255,255,240));
			//DisplayText(Elipsisify(%nameList[$index], $width2-$prefix2-1), $prefix2, $i);
			//DisplayText		   (%nameList[$index], $prefix2, $i);
			
			// Process name
			$string = Elipsisify(%nameList[$index], ($right-260)-($prefix2-16));
			DisplayTextShadowed($string, $prefix2-16, $i, RGBA(0xFF,0x1D,0x0,255), RGBA(255,255,255,255));

			$index = 2*$index+1;

			// CPU usage
			DisplayTextShadowed(FormatValue(%cpuList[$index], "0", "1"), $right-260, $i, 0xB328F612, RGBA(255,255,255,240));
			
			// Mem usage
			DisplayTextShadowed(FormatSize(%memList[$index], 1, 0, 1), $right-$width-40, $i, RGBA(0x50,0x8D,0xC5,156), RGBA(255,255,255,240));
			
			// CPU Priority 			
			$priority = GetProcessPriority(%pidList[$index]);
			if (!IsNull($priority) && $priority != 2)
				DisplayText($priority+1, 3, $i);


			if (size($hwndDict[%pidList[$index]])) {
				$midy = $i+$height/2;
				//DrawRect($prefix+2, $midy, $prefix2-2, $midy);
				
				if (%expandedIndex == $pos) {
					$midx = ($prefix+$prefix2)/2;
					$dy = ($prefix2-$prefix-4)/2;
					//DrawRect($midx, $midy-$dy, $midx, $midy+$dy);
					
					for ($w=0; $w<size($expandedHwnds); $w++) {
						$i+=$height;
						if ($i >= 0) {
							if ($inv == $i)
								SetDrawColor(colorHighlightText);
							else 
								SetDrawColor(colorText);

							$text = GetWindowText($expandedHwnds[$w]);
							if (!size($text))
								$text = "No Name";

							DisplayText(Elipsisify($text, $right-3*$prefix), 3*$prefix, $i);
						}
					}
				}
			}
		}

		SetDrawColor(RGBA(160,160,160,120));
		$listPos = $listPos * ($bottom-$bottom/20.0) / ($listLen-1);
		//DrawRect($farRight-1, $listPos, $farRight+2, $listPos+$bottom/20.0);
		SetDrawColor(colorText);

		if (0 && %hasFocus) {
			$i = (($realIndex) % %page) * $height;
			InvertRect(0,$i,$right-2, $i+$height);
		}
		
		%selectedName = %nameList[%indices[%selectedIndex]];
		%selectedPid = %pidList[2*%indices[%selectedIndex]+1];
	}
};
