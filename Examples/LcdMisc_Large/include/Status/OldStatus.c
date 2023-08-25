// Not used, by default.  Looks a bit less cluttered, so I'm keeping it around.

#requires <framework\header.c>

struct DefaultStatus {
	var %cpu;
	function DefaultStatus() {
		%cpu = PerformanceCounter("Processor", "*", "% Processor Time");
    }


	function Draw() {
		ClearScreen();

		DisplayHeader();

		DrawRect(80, 7, 80, 43);

		$cpuData = %cpu.GetValue();
		DisplayText("CPU1:", 0, 8);
		DisplayTextRight(FormatValue($cpuData[0], 0, 0) +s "%", 48, 8);
		DrawRect(50, 9, 50+(80-50)/100.0*$cpuData[0], 13);

		DisplayText("CPU2:", 0, 15);
		DisplayTextRight(FormatValue($cpuData[1], 0, 0) +s "%", 48, 15);
		DrawRect(50, 16, 50+(80-50)/100.0*$cpuData[1], 20);

		$memory = GetMemoryStatus();

		DisplayTextRight("MEM:", 19, 22);
		DisplayTextRight(FormatValue($memory[1]/1024.0/1024.0) +s " M", 55, 22);
		DisplayTextRight("VM:", 19, 29);
		DisplayTextRight(FormatValue($memory[3]/1024.0/1024.0) +s " M", 55, 29);


		DisplayTextRight("down:", 127, 8);
		DisplayTextRight(FormatSize(GetDownstream(), 1, 0), 160, 8);

		DisplayTextRight("up:", 127, 15);
		DisplayTextRight(FormatSize(GetUpstream(), 1, 0), 160, 15);

		DisplayTextRight(GetRemoteIP(), 160, 22);


		DisplayText("vol:", 0, 37);
		$volume = GetAudioValue(0, -4, 0, -0x50030001, 1) * 100/65535;

		DisplayTextRight(FormatValue($volume) +s "%", 50, 37);

		// Mute
		if (GetAudioValue(0, -4, 0, -0x20010002, 1)) {
			DisplayTextRight("M", 29, 37);
		}

		DisplayTextRight("C: " +s FormatSize(GetDiskSpaceFree("C:")), 160, 30);
		DisplayTextRight("D: " +s FormatSize(GetDiskSpaceFree("D:")), 160, 37);
	}
};
