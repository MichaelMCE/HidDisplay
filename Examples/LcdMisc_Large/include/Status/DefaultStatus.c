#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <util\Audio.c>
#import <constants.h>
#import <Modules\Graph.c>
#requires <Modules\DriveInfo.c>
#requires <Shared Memory.c>

struct DefaultStatus {
	var %counters;
	function DefaultStatus() {
		%counters = GetCounterManager();
    }

	function Draw() {
		ClearScreen();

		DisplayHeader();

		DrawRect(80, 7, 80, 43);

		DisplayText("CPU1:", 0, 9);
		DisplayTextRight(FormatValue(%counters.cpuData[0], 0, 0) +s "%", 48, 9);
		%counters.cpuGraph[0].Draw(50,15, 78, 9);
		// Code for a simple bar instead of a graph.
		//DrawRect(50, 9, 50+(80-50)/100.0*$cpuData[0], 13);

		DisplayText("CPU2:", 0, 17);
		DisplayTextRight(FormatValue(%counters.cpuData[1], 0, 0) +s "%", 48, 17);
		%counters.cpuGraph[1].Draw(50, 23, 78, 17);
		//DrawRect(50, 16, 50+(80-50)/100.0*$cpuData[1], 20);

		$memory = GetMemoryStatus();

		DisplayTextRight("MEM:", 19, 24);
		DisplayTextRight(FormatValue($memory[1]/1024.0/1024.0) +s " M", 55, 24);
		DisplayTextRight("VM:", 19, 30);
		DisplayTextRight(FormatValue($memory[3]/1024.0/1024.0) +s " M", 55, 30);


		DisplayText("dn:"+s FormatSize(%counters.down, 1,0,1), 82, 9);
		%counters.downGraph.Draw(124, 15, 159, 9);

		DisplayText("up:" +s FormatSize(%counters.up, 1,0,1), 82, 17);
		%counters.upGraph.Draw(124, 23, 159, 17);

		DisplayTextRight(GetRemoteIP(), 160, 24);


		DisplayText("vol:", 0, 37);
		$volume = GetMaxAudioState(0);
		$vString = FormatValue($volume[0]) +s "%";
		DisplayTextRight($vString, 51, 37);
		// Mute
		if ($volume[1]) {
			DrawRect(51-TextSize($vString)[0], 40, 51, 40);
		}

		DisplayDriveInfo(82,32,159,42);

		/*
		DisplayText("C:", 93, 30);
		DisplayTextRight(FormatValue(CachedCall("GetDiskSpaceFree", "C:", 10)/1024.0/1024.0/1024.0,0,1), 135, 30);
		$temp = CachedCall("SafeSmart", "C:", 45)[SMART_ATTRIBUTE_TEMPERATURE].value;
		if ($temp > 0 && ($temp < 200)) DisplayTextRight("(" +s $temp +s ")", 160, 30);

		DisplayText("D:", 93, 37);
		DisplayTextRight(FormatValue(CachedCall("GetDiskSpaceFree", "D:", 10)/1024.0/1024.0/1024.0,0,1), 135, 37);
		$temp = CachedCall("SafeSmart", "D:", 46)[SMART_ATTRIBUTE_TEMPERATURE].value;
		if ($temp > 0 && ($temp < 200)) DisplayTextRight("(" +s $temp +s ")", 160, 37);
		//*/
	}
};
