/*

Garmin training center database file parser.

Curently processes latitude and longitude with Altitude available but disabled by default (I don't personally require it)
But possible to add otherstuff such as temperature, cadence and/or heartrate with ease

*/


#requires <constants.h>
#requires <util/time.c>




struct TcxStats {
	var %minLat;
	var %maxLat;
	var %minLong;
	var %maxLong;
	var %minAlt;
	var %maxAlt;
	var %mAscent;
	var %mDescent;
	var %deltaAlt;
	var %scaleLat;
	var %scaleLong;
	var %aspect;
	var %lapSum;

	var %tDataPoints;

	var %totalTimeSeconds;
	var %distanceMeters;
	var %maximumSpeed;
	var %aveSpeed;
	var %calories;
	var %date;

	var %cadenceSum;
	var %cadenceCount;
	var %cadenceAve;
	
	function TcxStats () {
		%statsReset();
	}
	
	function statsReset () {
		%date = 0;
		%totalTimeSeconds = 0;
		%distanceMeters = 0;
		%maximumSpeed = 0;
		%aveSpeed = 0;
		%calories = 0;
		
		%cadenceSum = 0.0;
		%cadenceCount = 0;
		%cadenceAve = 0.0;
		
		%minLat = 9999.0;
		%maxLat = -9999.0;
		%minLong = 9999.0;
		%maxLong = -9999.0;
	
		%minAlt = 9999.0;
		%maxAlt = -9999.0;
		%mAscent = 0.0;
		%mDescent = 0.0;
		%deltaAlt = 0.0;
		
		%scaleLat = 1.0;
		%scaleLong = 1.0;
		
		%tDataPoints = 0;
		%aspect = 4.0/3.0;
	}
	
	
	function statsDump ($logHow) {
		Log("date: "+s %date, $logHow);
		Log("distance: "+s FormatValue(%distanceMeters/1000.0, 0, 2) +s " km", $logHow);
		Log("total time: "+s FormatDuration(%totalTimeSeconds), $logHow);
		Log("ave speed: "+s FormatValue(%aveSpeed, 0, 1) +s " km/h", $logHow);
		Log("max speed: "+s FormatValue(%maximumSpeed, 0, 1) +s " km/h", $logHow);
		Log("ave cadence: "+s %cadenceAve +s " rpm", $logHow);
		Log("calories: "+s %calories, $logHow);
		Log("", $logHow);

		Log("lat: " +s %minLat +s " : " +s %maxLat/* +s " - " +s FormatValue(%dLat, 3, 6)*/, $logHow);
		Log("long: " +s %minLong +s " : " +s %maxLong/* +s " - " +s FormatValue(%dLong, 3, 6)*/, $logHow);
		Log("alt: " +s %minAlt +s " : " +s %maxAlt/* +s " - " +s %deltaAlt*/, $logHow);
		if (%mAscent && %mDescent) Log("alt diff: " +s %mAscent +s " : " +s %mDescent, $logHow);
		//Log("route aspect: " +s %aspect, $logHow);
		//Log("scale: " +s %scaleLat +s " : " +s %scaleLong, $logHow);
		//Log(%tDataPoints +s " data points", $logHow);
		Log("", $logHow);
	}
	
};


struct Tcx extends TcxStats {
	var %mapImg;
	var %daspect, %tcxFile, %doRender;
	var %scaleFactor, %zoomFactor;
	
	var %posList;
	var %posListNormalized;
	var %posListScaled;

	var %speedList;

	var %positionTotal;	// total data points in position list. not the same as size(%posList)	
	var %dataSource;		// 1=file, 2=file from a location
	var %dataSourceFile;
	var %dataSourceLocation;	// 
	var %dLat, %dLong;
	
	var %scaleLatZoom;		// %scaleLat with Zoom factor applied
	var %scaleLongZoom;		// %scaleLong with Zoom factor applied
	
	
	function Tcx ($filename, $location){
		//%mapImg = ImageLoad("maps/Untitled44272.png");
		%doRender = 0;
		%dataSource = 0;
		%daspect = (DWIDTH+0.0) / (DHEIGHT+0.0);
		
		// 3.04 is the Strava.com map aspect/scale
		// 2.35 is my display aspect for a best fit (4/3 * 480/272)
		//%scaleFactor = 3.04;
		%scaleFactor = (4.0/3.0 * %daspect);	// 2.35
		%zoomFactor = 1.0;
		
		if (!IsNull($filename)){
			%dataSourceFile = $filename;
			%dataSource = 1;
			
		}else if (!IsNull($location)){
			%dataSourceFile = %getLastAddition($location);
			%dataSource = 2;
			%dataSourceLocation = $location;
			
		}else{
			Log("TCX:: Nothing to parse",1);
			return NULL;
		}
	};
	
	function getLastAddition ($path){
		if (IsNull($path))
			$path = %dataSourceLocation;
		
		$files = FileInfo($path +s "*.tcx");
		//sort($files);
		
		// most recent file will be the last file in the list
		if ($files[size($files)-1].name !=s ".."){
			$last = $path +s $files[size($files)-1].name;
			return $last;
		}
		return NULL;
	}
	
	function update ($whichSource){
		if (%dataSource == 2 || $whichSource == 2)
			%dataSourceFile = %getLastAddition(%dataSourceLocation);
		else if ($whichSource == 1)
			%dataSourceFile = %tcxFile;
		
		if (!IsNull(%dataSourceFile)){
			if (%dataSourceFile !=s %tcxFile){
				%tcxFile = %dataSourceFile;
				%doRender = (%parse(%tcxFile) > 10);
			}
		}
	}
	
	/*function calcModeSpeed (){
		$hist = list();
		$dataLen = size(%speedList);
		resize($hist, $dataLen);
		
		for ($i = 0; $i < $dataLen; $i++);
			$hist[]
	}*/
	
	function calcNormalization (){
		if (%aspect <= %scaleFactor){
			$factor = %aspect/%scaleFactor;
			%scaleLong = $factor / %dLong;
			%scaleLat = 1.0/%dLat;
			
			//Log("long factor: " +s $factor);
		}else{
			$factor = 1.0/(%aspect/%scaleFactor);
			%scaleLat = $factor / %dLat;
			%scaleLong = 1.0 / %dLong;
						
			//Log("lat factor: " +s $factor);
		}		
	}
	
	function zoom ($factor){
		%zoomFactor = $factor;
		%scaleLatZoom = %scaleLat * %zoomFactor;
		%scaleLongZoom = %scaleLong * %zoomFactor;
		
		//Log("scaleLatZoom: " +s %scaleLatZoom);
		//Log("scaleLongZoom: " +s %scaleLongZoom);
	}
	
	function normalize (){
		$listLen = size(%posList);
		%posListNormalized = list($listLen);
				
		for ($i = 0; $i < $listLen; $i += 2){
			%posListNormalized[$i] = (%posList[$i] - %minLong) * %scaleLongZoom;
			%posListNormalized[$i+1] = 1.0 - ((%posList[$i+1] - %minLat) * %scaleLatZoom);
			
			//Log($i +s " # " +s FormatValue(%posList[$i], 3, 6) +s "  :  " +s FormatValue(%posListNormalized[$i+1], 3, 6));
		}
	}
	
	function scale ($width, $height){
		if (!$width) $width = DWIDTH-1.0;
		if (!$height) $height = DHEIGHT-1.0;
		
		$listLen = size(%posListNormalized);
		%posListScaled = list($listLen);
		
		for ($i = 0; $i < $listLen; $i += 2){
			%posListScaled[$i] = %posListNormalized[$i] * $width;
			%posListScaled[$i+1] = %posListNormalized[$i+1] * $height;
		}
	}
	
	function parse ($path){
		//Log($path, 1);
		
		$xml = FileReader($path, READER_TEXT_CONVERT);
		if (IsNull($xml)) return 0;
		
		$gpxString = $xml.Read(-1);
		if (IsNull($gpxString) || strlen($gpxString) < 100) return 0;
		
		$gpx = ParseXML($gpxString);
		if (size($gpx) < 3) return 0;

		$gpxlist = $gpx[1];				// "TrainingCenterDatabase" should always be [1]
		for ($i = 0; $i < size($gpxlist); $i++){	// find Activities branch
			if ($gpxlist[$i] ==S "Activities"){
				$gpxActivities = $gpxlist[$i+1];
				break;
			}
		}
		if (IsNull($gpxActivities)) return 0;

		for ($i = 0; $i < size($gpxActivities); $i++){
			if ($gpxActivities[$i] ==S "Activity"){	// find then parse an Activity
				$gpxActivity = $gpxActivities[$i+1];
				break;
			}
		}
		if (IsNull($gpxActivity)) return 0;

		%statsReset();
		%positionTotal = 0;
		
		/*$speedCt = 0;
		$speedListLen = 512;
		%speedList = list();
		resize(%speedList, $speedListLen);*/
		
		$listLen = 512;
		%posList = list();
		resize(%posList, $listLen);
		$lapReal = 0;
		%lapSum = 0;

		Log("#####################", $logHow);
		Log("#####################", $logHow);
		Log("", $logHow);

		for ($lap = 0; $lap < size($gpxActivity); $lap++){
			if ($gpxActivity[$lap] ==S "Id"){
				$id = $gpxActivity[++$lap];
				$activityId = $id[3];
				if (!%date || IsNull(%date)){
					%date = strreplace($activityId, "T", ", ");
					%date = strreplace(%date, "Z", " (+1 hr for daylight saving)");
				}
				
			}else if ($gpxActivity[$lap] ==S "Lap"){
				$gpxLap = $gpxActivity[++$lap];
				$preAlt = 0.0;
				$start = 0;

				$idx = listFind($gpxLap, "TotalTimeSeconds");
				if ($idx >= 0) %totalTimeSeconds = $gpxLap[$idx+1][3];
				$idx = listFind($gpxLap, "DistanceMeters");
				if ($idx >= 0) %distanceMeters = $gpxLap[$idx+1][3];
				$idx = listFind($gpxLap, "MaximumSpeed");
				if ($idx >= 0) %maximumSpeed = ($gpxLap[$idx+1][3] * 60.0*60.0)/1000.0;
				$idx = listFind($gpxLap, "Calories");
				if ($idx >= 0) %calories = $gpxLap[$idx+1][3];

				$idx = listFind($gpxLap, "Cadence");
				if ($idx >= 0){
					%cadenceAve = $gpxLap[$idx+1][3];
					%cadenceSum += %cadenceAve;
					%cadenceCount++;
				}

				if (%distanceMeters && %totalTimeSeconds)
					%aveSpeed = ((%distanceMeters / %totalTimeSeconds) * 60.0*60.0)/1000.0;
				
				for ($track = 0; $track < size($gpxLap); $track++){
					if ($gpxLap[$track] ==S "Track"){
						$gpxTrk = $gpxLap[++$track];

						for ($tp = 0; $tp < size($gpxTrk); $tp++){
							if ($gpxTrk[$tp] ==S "Trackpoint"){
								$trackPt = $gpxTrk[++$tp];
								
								for ($pos = 0; $pos < size($trackPt); $pos++){
									if ($trackPt[$pos] ==S "Position"){
										$trackPtPos = $trackPt[++$pos];

										if (size($trackPtPos) > 1){
											$trackPtPosLat = $trackPtPos[3];
											$trackPtPosLong = $trackPtPos[5];
											if (IsNull($trackPtPosLong)) continue;
					
											$lat = $trackPtPosLat[3];
											$long = $trackPtPosLong[3];
					
											push_back(%posList, $long);
											push_back(%posList, $lat);
					
											if ($lat < %minLat) %minLat = $lat;
											if ($lat > %maxLat) %maxLat = $lat;
											if ($long < %minLong) %minLong = $long;
											if ($long > %maxLong) %maxLong = $long;

											//Log(%positionTotal +s " # " +s $trackPtPosLat[3] +s " : " +s $trackPtPosLong[3]);
											%positionTotal++;
					
											if (%positionTotal*2 >= $listLen-2){
												$listLen = %positionTotal*4;
												resize(%posList, $listLen);
											}
										}
									/*}else if ($trackPt[$pos] ==S "DistanceMeters"){
										$mps = $trackPt[++$pos][3];
										%speedList[$speedCt++] = $mps;
										if ($speedCt >= $speedListLen){
											$speedListLen *= 2;
											resize(%speedList, $speedListLen);
										}*/
									}else if ($trackPt[$pos] ==S "AltitudeMeters"){
										$alt = $trackPt[++$pos][3];
										if ($alt < %minAlt) %minAlt = $alt;
										if ($alt > %maxAlt) %maxAlt = $alt;
										
										/*$diff = $alt - $preAlt;
										if (abs($diff) >= 0.961425){
											if ($diff > 0.0){
												%mAscent += $diff;
												//%deltaAlt += $diff;
											}else if ($diff < 0.0){
												%mDescent -= $diff;
												//%deltaAlt += $diff;
											}
										}
										
										$preAlt = $alt;
										*/
									/*}else if ($trackPt[$pos] ==S "Cadence"){
										$cadence = $trackPt[++$pos][3];
										if ($cadence > 0 && $cadence < 300){
											%cadenceSum += $cadence;
											%cadenceCount++;
										}*/
									}
								}
							}
						}
					}
				}

				%lapSum += %aveSpeed;
				Log("## lap " +s ++$lapReal +s " ## ", $logHow);
				%statsDump();
			}
			%cadenceAve = %cadenceSum / %cadenceCount;
		}

		%tDataPoints = %positionTotal;
		$listLen = %positionTotal*2;			// *2 = Lat and Long co-ords only
		resize(%posList, $listLen);
		//resize(%speedList, $speedCt);

		// calc MPS mode 
		//%calcModeSpeed();
		
		// calc deltas
		%dLat = (%maxLat - %minLat);
		%dLong = (%maxLong - %minLong);
		%deltaAlt = (%maxAlt - %minAlt);
		%aspect = (%dLong / %dLat);


		// calc normalization factors
		%calcNormalization();

		%zoom(0.97);	// normalize() and scale() must be called after each call to zoom();
		
		// normalize data points
		%normalize();
		
		// scale for display 
		%scale();		// scale(w,h) or use default when not supplied
		
		//%statsDump();
		Log("average speed: " +s FormatValue(%lapSum /= $lapReal, 0, 2) +s " km/h", $logHow);
		Log("average cadence: "+s FormatValue(%cadenceAve, 0, 2) +s " rpm", $logHow);
		Log("route aspect: " +s %aspect, $logHow);
		Log("scale: " +s %scaleLat +s " : " +s %scaleLong, $logHow);
		Log(%tDataPoints +s " data points", $logHow);


		//Log("dscale: " +s FormatValue(%dLat, 3, 6) +s " : " +s FormatValue(%dLong, 3, 6), $logHow);

		return %tDataPoints;
	}
	
	function render ($width, $height){
		if (!%doRender) return;

		//DrawImage(%mapImg, 9, 0);
		
		$listLen = size(%posListScaled);
		$offsetX = 0.0;
		$offsetY = 0.0;
		$scale = ((%daspect * %aspect)/%scaleFactor);


		// center route
		if (%aspect <= %scaleFactor){
			$scale *= %zoomFactor;
			$offsetX = ($width - ($height * $scale)) / 2.0;
			$offsetY = ($height-($height * %zoomFactor))/2.0;
		}else{
			$scale /= %zoomFactor;
			$offsetY = ($height - ($width / $scale)) / 2.0;
			$offsetX = ($width-($width * %zoomFactor))/2.0;
		}

		//Log("scale: " +s $scale);
		//Log("offsetX: " +s $offsetX);
		//Log("offsetY: " +s $offsetY);
	
		SetDrawColor((255<<24) | 0x0);
		for ($i = 0; $i < $listLen; $i+=2){
			$x1 = %posListScaled[$i]   + $offsetX;
			$y1 = %posListScaled[$i+1] - $offsetY;

			DrawRect($x1-2, $y1-2, $x1+2, $y1+2);
		}
				
		SetDrawColor((255<<24) | 0xFF00FF);
		for ($i = 0; $i < $listLen; $i+=2){
			$x1 = %posListScaled[$i]   + $offsetX;
			$y1 = %posListScaled[$i+1] - $offsetY;

			DrawRect($x1-1, $y1-1, $x1+1, $y1+1);
		}

		SetDrawColor((255<<24) | 0xFFFFFF);
		for ($i = 0; $i < $listLen-2; $i+=2){
			$x1 = %posListScaled[$i]   + $offsetX;
			$y1 = %posListScaled[$i+1] - $offsetY;
			$x2 = %posListScaled[$i+2] + $offsetX;
			$y2 = %posListScaled[$i+3] - $offsetY;

			DrawLine($x1, $y1, $x2, $y2);
		}
	}
	
}

