#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <util\Audio.c>
#import <constants.h>
#import <Modules\Graph.c>

struct GameStatus {
	var %counters, %lastPing, %lastPingTime, %ip;
	function GameStatus() {
		%lastPingTime = GetTickCount()-100000;
		%counters = GetCounterManager();
    }

	function Ping() {
		if (IsNull(%ip)) %ip = IPAddrWait("www.google.com")[0];
		if (IsNull(%ip)) %lastPing = "N/A";
		else {
			%lastPing = %ip.PingWait();
			if (IsInt(%lastPing)) %lastPing = %lastPing +s " ms";
		}
	}

	function Hide() {
		CleanupDdraw();
	}

	function Draw() {
		$t = GetTickCount();
		if ($t - %lastPingTime >= 5000) {
			%lastPingTime = $t;
			SpawnThread("Ping", $this);
		}
		ClearScreen();

		DisplayHeader();

		DrawRect(80, 7, 80, 43);
		UseFont(0);

		DisplayText("CPU:", 0, 9);
		DisplayTextRight(FormatValue(%counters.cpuData["_Total"], 0, 0), 49, 9);
		%counters.cpuTotalGraph.Draw(50,15, 78, 9);

		$fraps = GetFrapsData();
		if (IsNull($fraps)) {
			DisplayText("No Fraps", 0, 17);
		}
		else {
			DisplayText("FPS:", 0, 17);
			DisplayTextRight($fraps["fps"], 43, 17);
		}

		$memory = GetMemoryStatus();

		DisplayTextRight("MEM:", 19, 24);
		DisplayTextRight(FormatValue($memory[1]/1024.0/1024.0) +s " M", 55, 24);
		DisplayTextRight("VM:", 19, 30);
		DisplayTextRight(FormatValue($memory[3]/1024.0/1024.0) +s " M", 55, 30);

		DisplayText("vol:", 0, 37);
		$volume = GetMaxAudioState(0);
		$vString = FormatValue($volume[0]) +s "%";
		DisplayTextRight($vString, 51, 37);
		// Mute
		if ($volume[1]) {
			DrawRect(51-TextSize($vString)[0], 40, 51, 40);
		}



		DisplayText("dn:"+s FormatSize(%counters.down, 1,0,1), 82, 9);
		%counters.downGraph.Draw(124, 15, 159, 9);

		DisplayText("up:" +s FormatSize(%counters.up, 1,0,1), 82, 17);
		%counters.upGraph.Draw(124, 23, 159, 17);

		$fans = NvAPI_GPU_GetTachReading();
		if (size($fans[0])) {
			DisplayTextRight("NV Fan:", 123, 24);
			DisplayTextRight($fans[0], 160, 24);
		}
		else {
			$vram = GetVram();
			DisplayTextRight("VRAM:", 123, 24);
			DisplayTextRight(FormatValue($vram[1]/(1024.0*1024.0), 0, 1), 160, 24);
		}

		$temps = NvAPI_GPU_GetThermalSettings();
		if (size($temps)) {
			if (!IsNull($temps[1][0])) {
				DisplayTextRight($temps[1][0].currentTemp, 160, 30);
				DisplayTextRight($temps[0][0].currentTemp, 140, 30);
			}
			else if (!IsNull($temps[0][1])) {
				DisplayTextRight($temps[0][1].currentTemp, 160, 30);
				DisplayTextRight($temps[0][0].currentTemp, 140, 30);
			}
			else {
				DisplayTextRight($temps[0][0].currentTemp, 160, 30);
			}
			DisplayTextRight("NV Temp:", 123, 30);
			//DisplayText($temp.ambientTemp, 0, 27);
		}
		else {
			DisplayTextRight(FormatValue($vram[0]/(1024.0*1024.0), 0, 1), 160, 30);
		}

		// Old function to get nvidia temps.
		/*$temp = NvThermalSettings();
		if (!IsNull($temp)) {
			DisplayTextRight($temp.coreTemp, 160, 30);
			DisplayTextRight("NV Temp:", 123, 30);
			//DisplayText($temp.ambientTemp, 0, 27);
		}
		else {
			DisplayTextRight(FormatValue($vram[0]/(1024.0*1024.0), 0, 1), 160, 30);
		}//*/

		DisplayTextRight("Ping:", 120, 37);
		$w = TextSize(%lastPing)[0];
		if ($w < 50) {
			DisplayTextRight(%lastPing, 159, 37);
		}
		else {
			DisplayText(%lastPing, 123, 37);
		}
	}
};
