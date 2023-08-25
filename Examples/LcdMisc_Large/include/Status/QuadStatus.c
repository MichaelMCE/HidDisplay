#requires <framework\header.c>
#requires <util\CPUSaver.c>
#requires <util\Audio.c>
#import <constants.h>
#import <Modules\Graph.c>
#requires <Modules\DriveInfo.c>
#requires <Modules\CounterManager.c>

struct QuadStatus {
	var %miniFont, %counters;
	function QuadStatus() {
		%miniFont = Font("6px2bus", 6);
		%counters = GetCounterManager();
    }

	function Draw($event, $param, $name, $res) {

		ClearScreen();

		DisplayHeader();

		DrawRect(86, 7, 86, 43);
		DrawRect(58, 7, 58, 43);

		DisplayText("CPU:", 0, 9);
		DisplayTextRight(FormatValue(%counters.cpuData["_Total"], 0, 0), 36, 9);

		$memory = GetMemoryStatus();

		DisplayTextRight("MEM:", 19, 22);
		DisplayTextRight(FormatValue($memory[1]/1024.0/1024.0) +s " M", 55, 22);
		DisplayTextRight("VM:", 19, 29);
		DisplayTextRight(FormatValue($memory[3]/1024.0/1024.0) +s " M", 55, 29);


		DisplayText("vol:", 0, 37);
		$volume = GetMaxAudioState(0);
		$vString = FormatValue($volume[0]) +s "%";
		DisplayTextRight($vString, 51, 37);
		// Mute
		if ($volume[1]) {
			DrawRect(51-TextSize($vString)[0], 40, 51, 40);
		}


		%counters.cpuGraph[0].Draw(60,15, 84, 9);

		%counters.cpuGraph[1].Draw(60, 24, 84, 18);

		/* Tried displaying all 5 graphs in the middle and highlighting the
		 * middle one, which was total, and moving unique values into the middle
		 * row, highlighting it all the way across.  Looked bad.  Leaving it
		 * here just in case I ever try to get it looking nice again.
		 */

		//DisplayTextRight(FormatValue($cpuData["_Total"], 0, 0), 59, 23);
		//%cpuGraph4.Update($cpuData["_Total"]);
		//%cpuGraph4.Draw(60, 28, 84, 24);

		%counters.cpuGraph[2].Draw(60,33, 84, 27);

		%counters.cpuGraph[3].Draw(60, 42, 84, 36);



		DisplayText("d:"+s FormatSize(%counters.down, 1,0,1), 88, 9);
		%counters.downGraph.Draw(123, 15, 159, 9);

		DisplayText("u:" +s FormatSize(%counters.up, 1,0,1), 88, 17);
		%counters.upGraph.Draw(123, 23, 159, 17);

		$ip = GetRemoteIP();
		if (TextSize($ip)[0] > 73) {
			UseFont(%miniFont);
			// Smaller font has no whitespace above it, the larger one does.
			DisplayTextCentered($ip, 124, 25);
			UseFont(0);
		}
		else {
			DisplayTextCentered($ip, 124, 24);
		}

		DisplayDriveInfo(88,32,159,42);
	}
};
