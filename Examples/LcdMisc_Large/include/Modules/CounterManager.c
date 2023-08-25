#requires <Modules\Graph.c>
// Updates graphs at constant rates.  May add long term graphs at some point.
// To add to this, you can use inheritance instead of creating a new class.
// Note that you will have to either replace GetCounterManager() calls in
// the classes that use it, or (preferably) set _counterManager to point to
// an object of the new class before initializing views.

struct CounterManager {
	var %cpu, %cpuGraph, %downGraph, %upGraph, %miniFont, %cpuData, %down, %up, %cpuTotalGraph;
	var %cpuSum;
	var %proc;
	var %procList;
	var %procData;
	
	function CounterManager() {
		%cpu = PerformanceCounter("Processor", "*", "% Processor Time");
		%cpuGraph = list();
		%cpuTotalGraph = Graph(0, 0, 100, DWIDTH, 3);
		
		%downGraph = Graph(0, 0, 3.5*1024*1024, DWIDTH, 3);
		%upGraph = Graph(0, 0, 512*1024, DWIDTH, 3);

		eventHandler.Insert(-1, $this);
	}

	function CPUUpdate() {
		%cpuData = %cpu.GetValue();

		if (!size(%cpuGraph[$i])){
			for ($i = 0; $i<size(%cpuData)-1; $i++)
				%cpuGraph[$i] = Graph(0, 0, 100, DWIDTH, 3);
		}

		%cpuTotalGraph.Update(%cpuData["_Total"]);		// update the icon

		for ($i = size(%cpuGraph)-1; $i >= 0; $i--) {
			%cpuGraph[$i].Update(%cpuData[$i]);
			//WriteLogLn(FormatValue(%cpuData[$i]), 1);
		}
	}

	function NetworkUpdate() {
		%down = GetDownstream();
		//%down = GetAllDown()["RT73 USB Wireless LAN Card #6"];
		//%down = GetAllDown()["MAC Bridge Miniport"];
		//%down = GetAllDown()["TP-Link Wireless USB Adapter"];
		%downGraph.Update(%down);

		%up = GetUpstream();
		//%up = GetAllUp()["RT73 USB Wireless LAN Card #6"];
		//%up = GetAllUp()["MAC Bridge Miniport"];
		//%up = GetAllUp()["TP-Link Wireless USB Adapter"];
		%upGraph.Update(%up);
	}

	function CounterUpdate() {
		%CPUUpdate();
		%NetworkUpdate();
	}
};

function GetCounterManager() {
	if (IsNull(_counterManager)) {
		_counterManager = CounterManager();
	}
	return _counterManager;
}

