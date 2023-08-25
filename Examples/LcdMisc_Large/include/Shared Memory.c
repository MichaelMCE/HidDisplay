#requires <util\XML.c>

/*  GetSpeedFanData()
 *    Collects data stored in shared memory by SpeedFan
 *      http://www.almico.com/speedfan.php
 *    Uses version 1 of SpeedFan's shared data, which all versions of SpeedFan have
 *      used since shared memory support was first added. Returns null when SpeedFan
 *      is not running or the data is not in a recognized format.
 *    Creates a dictionary with the following entries;
 *     temps;              //List of all temp values reported, generally in degrees C
 *     fans;               //RPMs of fans
 *     volts;              //All reportage voltages
 */
function GetSpeedFanData() {
	$data = GetSharedMemory("SFSharedMemory_ALM");
	$version = ParseBinaryInt($data, 0, 2, 1);
	$memSize = ParseBinaryInt($data, 4, 4);
	if ($version != 1 || $memSize != 402) return;

	$temps = list();
	for ($i = ParseBinaryInt($data, 12, 2, 1); $i;) {
		$temps[--$i] = ParseBinaryInt($data, 18+4*$i, 4)/100.0;
	}

	$fans = list();
	for ($i = ParseBinaryInt($data, 14, 2, 1); $i;) {
		$fans[--$i] = ParseBinaryInt($data, 146+4*$i, 4);
	}

	$volts = list();
	for ($i = ParseBinaryInt($data, 16, 2, 1); $i;) {
		$volts[--$i] = ParseBinaryInt($data, 274+4*$i, 4)/100.0;
	}
	return ("temps":$temps,"fans":$fans,"volts":$volts);
}

/*  GetRivaTunerHWData()
 *    Collects data stored in shared memory by RivaTuner.  Only uses the hardware monitoring
 *      component.
 *    Url:  http://www.guru3d.com/index.php?page=rivatuner
 *    Uses version 1 and 1.1 of the shared data.  Tries to be compatible with future versions.
 */
function GetRivaTunerHWData() {
	$data = GetSharedMemory("RTHMSharedMemory");
	// Order is flipped.
	if (ParseBinaryBinary($data, 0, 4) ==s "MHTR") {
		$version = ParseBinaryInt($data, 4, 4);
		$count = ParseBinaryInt($data, 8, 4);
		if ($version == 0x10000) {
			$entrySize = 44;
			$dimSize = 8;
			$floatoffset = 40;
			$pos = 16;
		}
		else if ($version >= 0x10001) {
			$entrySize = ParseBinaryInt($data, 16, 4);
			$floatoffset = 56;
			$dimSize = 16;
			$pos = 20;
		}
		else return;
		$end = $count * $entrySize + $pos;
		$out = dict();
		for ( ;$pos < $end; $pos += $entrySize) {
			$out[ParseBinaryStringUTF8($data, $pos, 32)] = list(ParseBinaryStringUTF8($data, $pos+32, $dimSize), ParseBinaryFloat($data, $pos+$floatoffset, 4));
		}
	}
	return $out;
}


/*  GetEverestData()
 *    Collects data stored in shared memory by Everest.  Creates a dictionary with entries
 *    temp", "fans", and "volt".  Each of those contains a dictionary containing one entry
 *    for each id from everest (With the id being a key).  Each of those contains a 2 element
 *    dictionary with the keys "label" and "value".
 *    Url:  http://lavalys.com/
 *    Uses version 1 and 1.1 of the shared data.  Tries to be compatible with future version.
 */
function GetEverestData() {
	$data = GetSharedMemory("EVEREST_SensorValues");
	if (!IsNull($data)) {
		$xml = ParseXML($data);
		$out = dict();
		for ($i = 0; $i<size($xml); $i+=2) {
			$data = $xml[$i+1];
			$type = $xml[$i];
			$dtype = $out[$type];
			if (IsNull($dtype)) {
				$dtype = $out[$type] = dict();
			}
			$id = SimpleXML($data, list("id"))[3];
			$label = SimpleXML($data, list("label"))[3];
			$value = SimpleXML($data, list("value"))[3];
			$dtype[SimpleXML($data, list("id"))[3]] = ("label":SimpleXML($data, list("label"))[3], "value":SimpleXML($data, list("value"))[3]);
		}
	}
	return $out;
}


/*  GetATTData()
 *    Written by Skitzman69.
 *    Grabs the shared data from ATT (ATI Tray Tools) written by Ray Adams,
 *      http://www.techpowerup.com/atitool/
 *    I am using v1.2.6.964, and there is no version identifier in the shared
 *      memory data, so we just have to assume the structure will never
 *      change... Ya Right!!!
 *    I create a dictionary with the following names;
 *     CurGPU;             //Current GPU Speed
 *     CurMEM;             //Current MEM Speed
 *     isGameActive;       //If game from profile is active, this field will be 1 or 0 if not.
 *     is3DActive;         //1=3D mode, 0=2D mode
 *     isTempMonSupported; //1 - if temperature monitoring supported by ATT
 *     GPUTemp;            //GPU Temperature
 *     ENVTemp;            //ENV Temperature
 *     FanDuty;            //FAN Duty
 *     MAXGpuTemp;         //MAX GPU Temperature
 *     MINGpuTemp;         //MIN GPU Temperature
 *     MAXEnvTemp;         //MAX ENV Temperature
 *     MINEnvTemp;         //MIN ENV Temperature
 *     CurD3DAA;           //Direct3D Antialiasing value
 *     CurD3DAF;           //Direct3D Anisotropy value
 *     CurOGLAA;           //OpenGL Antialiasing value
 *     CurOGLAF;           //OpenGL Anisotropy value
 *     IsActive;           //is 3d application active
 *     CurFPS;             // current FPS
 *     FreeVideo;          //Free Video Memory
 *     FreeTexture;        //Free Texture Memory
 *     Cur3DApi;           //Current API used in applciation
 *                           // 0 - OpenGL
 *                           // 1 - Direct3D8
 *                           // 2 - Direct3D9
 *                           // 3 - DirectDraw
 *                           // 4 - Direct3Dx (old direct3d api)
 *                           // 5 - Video Overlay
 *                           // -1 - No active rendering application
 *     MemUsed;
 */
function GetATTData() {
	$rawdata = GetSharedMemory("ATITRAY_SMEM");
	if (IsNull($rawdata)) return;

	$data = dict();

	$enum=("CurGPU","CurMEM","isGameActive","is3DActive","isTempMonSupported",
		"GPUTemp","ENVTemp","FanDuty","MAXGpuTemp","MINGpuTemp","MAXEnvTemp",
		"MINEnvTemp","CurD3DAA","CurD3DAF","CurOGLAA","CurOGLAF","IsActive",
		"CurFPS","FreeVideo","FreeTexture","Cur3DApi","MemUsed");
	for ($i=0 ; $i<size($enum) ; $i++) {
		$data[($enum[$i])] = ParseBinaryInt($rawdata, $i*4, 4, 1);
	}

	return ($data);
}
