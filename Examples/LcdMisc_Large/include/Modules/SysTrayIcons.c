// #requires <Modules/SysTrayIcons.c>



function IconEventHandler ($event, $param, $obj, $lParam) {
	
	if ($event ==s "WindowsMessage") {
		if ($lParam == WM_LBUTTONDBLCLK) {
			Run("taskmgr.exe",,2);

		}else if ($lParam == WM_RBUTTONUP) {
			$sep = "        ---      ";
			$res = ContextMenuWait(@GetCursorPos(), 
					"Save snapshot",			// 0
					$sep,						
					"Lock Workstation",			// 2
					"Restart LCDMisc.",			// 3
					$sep,
					"Quit",						// 5
					$sep,
					"Clock",					// 7
					"Next Clock",				// 8
					$sep,
					"< Left",					// 10
					"Right >",					// 11
					$sep,
					"Mode switch"				// 13
					);
			
			if ($res == 0){
				$name = FormatTime("DDMMYY") +s "_" +s FormatTime("HHNNSS") +s ".bmp";
				SaveSnapshot($name);
				
			}else if ($res == 7){
				$alist = list();
				$alist[0] = SBUI_DK_9;
				PostEvent("sbuiDKDown", 1, $alist);
				
			}else if ($res == 8){
				$alist = list();
				$alist[0] = SBUI_DK_10;
				PostEvent("sbuiDKDown", 1, $alist);
				
			}else if ($res == 10){
				$alist = list();
				$alist[0] = SBUI_DK_1;
				PostEvent("sbuiDKDown", 1, $alist);

			}else if ($res == 11){
				$alist = list();
				$alist[0] = SBUI_DK_5;
				PostEvent("sbuiDKDown", 1, $alist);

			}else if ($res == 13){
				$alist = list();
				$alist[0] = SBUI_DK_3;
				PostEvent("sbuiDKDown", 1, $alist);
				
			}else if ($res == 2){
				LockSystem();
				
			}else if ($res == 3){
				if (Is64Bit(Is64Bit_LCDMISC))
					Run("lcdmisc64.exe",,2);
				else
					Run("lcdmisc.exe",,2);
			}else if ($res == 4){
				
			}else if ($res == 5){
				Quit();
			}
		}
	}
}

function CpuIconHandler ($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon")
		DrawMultiGraph(GetCounterManager().cpuGraph, 0, 15, 15, 0);
	else
		IconEventHandler(@$);
}


// %counters.cpuData["_Total"]

function CpuIconTotalHandler ($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon") {
		ColorRect(0, 0, 15, 15, RGB(0,0,96));
		SetDrawColor(RGB(130, 180, 255));
		//GetCounterManager().cpuTotalGraph.Draw(-1, 16, 16, -1);
		GetCounterManager().cpuTotalGraph.Draw(-1, 16, 16, -1);
	}else{
		IconEventHandler(@$);
	}
}

function CpuNetIconHandler ($event, $param, $obj, $res) {
	if ($event ==s "DrawIcon") {
		$counters = GetCounterManager();
		DrawMultiGraph(list($counters.downGraph, $counters.upGraph), 0, 15, 15, 0, RGB(255,255,255));
	}else{
		IconEventHandler(@$);
	}
}

function AddCpuIcon() {
	SysTrayIcon("Core Usage", "CpuIconHandler");
}

function AddTotalCpuIcon() {
	SysTrayIcon("CPU Usage", "CpuIconTotalHandler");
}

function AddNetIcon() {
	SysTrayIcon("Network Bandwidth", "CpuNetIconHandler");
}


