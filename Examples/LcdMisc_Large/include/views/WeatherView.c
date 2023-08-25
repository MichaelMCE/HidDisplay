#import <Views\View.c>
#requires <Modules\DisplayText.c>
#requires <framework\header.c>
#requires <util\XML.c>

struct WeatherView extends View {
	var %weatherImage,
		%colorWeatherImage,
		%wind,
		%windImage,
		%windChill,
		%windDirection,
		%weatherDescription,
		%humidity,
		%temperature,
		%units,
		%forecast,
		%offset,
		%scrollTimer,
		%url,
		%location,
		%lastUpdate,
		%displayLocation,
		%miniFont,
		%bigFont,
		%background;

	function WeatherView ($_url, $_location) {
		%lastUpdate = Time()-60*60-5;
		%miniFont = Font("6px2bus", 6);
		%bigFont = Font("Arial", 18,0,0,0,CLEARTYPE_QUALITY);

		%url = $_url;
		if (!size(%url))
			%url = GetString("URLs", "Weather");
		//%url = "http://127.0.0.1/forecastrss.xhtml";
		//if (IsNull(stristr(%url, "u=c"))) {
		//	%toolbarImage = LoadImage32("Images\Weather.png");
		//}else {
			%toolbarImage = LoadImage32("Images\weather2.png");
			%toolbarImageL = LoadImage32("Images\weather1.png");
		//}
		%windImage = LoadImage32("Images\down1.png");
		%background = LoadImage32("Images\weatherBackground.png");

		// Draw on counter updates and after 6 second intervals for scrolling.
		// Means more redraws than are really needed, but scrolling is too fast at 1 second,
		// too slow at 2 second, and need to update the time display, too.
		%noDrawOnCounterUpdate = 0;
		%noDrawOnAudioChange = 1;

		%displayLocation = $_location;
	}

	function Show() {
		if (!%scrollTimer) {
			%offset = 0;
			%scrollTimer = CreateTimer("Bump", 6,,$this);
		}
	}

	function Hide() {
		if (%scrollTimer) {
			KillTimer(%scrollTimer);
			%scrollTimer = 0;
		}
	}

	function Bump() {
		NeedRedraw();
		if (size(%forecast) > 2) {
			%offset += 28;
			if (%offset < 28*3) return;
		}
		%offset = 0;
	}

	function Focus() {
		%hasFocus = 0;
		%lastUpdate = 0;
		%colorWeatherImage = NULL;
		NeedRedraw();
	}

	function Unfocus() {
		%hasFocus = 0;
	}
		
	function G15ButtonDown ($event, $param, $button) {
		WriteLogLn("15butdown");
		
		$button = FilterButton($button);
		if ($button & 0x3F) {
			if ($button == G15_UP || $button == G15_LEFT) {
				
			}else if ($button == G15_DOWN || $button == G15_RIGHT) {
				
			}else if ($button == G15_OK) {
			}else if ($button == G15_CANCEL) {
				
			}
			NeedRedraw();
			return 1;
		}
	}
	
	// Update using Yahoo Weather.
	function Update () {
		$xml = HttpGetWait(%url);
		if (IsNull($xml)) {
			// Update every 5 minutes when server's down.
			%lastUpdate = Time()-55*60;
		}else {
			$xml = SimpleXML(ParseXML($xml), ("rss", "channel"));
			$item = SimpleXML($xml, list("item"));
			%wind = SimpleXML($xml, ("yweather:wind", , "speed")) +s " " +s SimpleXML($xml, ("yweather:units", , "speed"));
			%windDirection = SimpleXML($xml, ("yweather:wind", , "direction"));
			%windChill = SimpleXML($xml, ("yweather:wind", , "chill"));
			%weatherDescription = SimpleXML($item, ("yweather:condition", , "text"));
			%humidity = SimpleXML($xml, ("yweather:atmosphere", , "humidity"));
			%units = SimpleXML($xml, ("yweather:units", , "temperature"));
			%temperature = SimpleXML($item, ("yweather:condition", , "temp"));
			%location = SimpleXML($xml, ("yweather:location", , "city")) +s ", " +s SimpleXML($xml, ("yweather:location", , "region"));

			if (%humidity >= 40 && (%units ==S "F" && %temperature >= 80) || (%units ==S "C" && %temperature >= (80-32)*5/9.0)) {
				if (%units ==S "C") {
					%temperature = %temperature * (9/5.0) + 32;
				}
				// Actually heat index.
				%windChill =
					- 42.379
					+ 2.04901523*%temperature
					+10.14333127*%humidity
					- 0.22475541*%temperature*%humidity
					- 0.00683783*%temperature*%temperature
					- 0.05481717*%humidity*%humidity
					+ 0.00122874*%temperature*%temperature*%humidity
					+ 0.00085282*%temperature*%humidity*%humidity
					- 0.00000199*%temperature*%temperature*%humidity*%humidity;
				if (%units ==S "C") {
					%temperature = (%temperature - 32)*(5/9.0);
					%windChill = (%windChill - 32)*(5/9.0);
					%temperature = FormatValue(%temperature);
				}
				%windChill = FormatValue(%windChill);
			}
			//%temperature = SimpleXML($item, ("yweather:condition", , "temp"));
			%forecast = list();

			for ($i=0; $i<5; $i++) {
				$index = FindNextXML($item, "yweather:forecast", $index+1);
				$data = $item[$index+1][1];
				if (IsNull($data)) break;
				%forecast[$i] = (SimpleXML($data, list("day")),
								 SimpleXML($data, list("low")),
								 SimpleXML($data, list("high")),
								 SimpleXML($data, list("text"))
				);
			}

			// Call before getting image, so can redraw while getting img.
			NeedRedraw();

			$html = SimpleXML($item, list("description"))[3];
			if (size($html)) {
				$html = ParseXML($html);
				if (!IsNull($html)) {
					$img = SimpleXML($html, ("img", ,"src"));
					if (!IsNull($img)) {
						$str = list();
						$str = strsplit($img,"/",0,0,0);
						$str = strsplit($str[size($str)-1],".",0,0,0);
						if (!IsNull($str[0]))
							$img = "http://l.yimg.com/a/i/us/nws/weather/gr/" +s $str[0] +s "d.png";
						$img = HttpGetWait($img);
						if (!IsNull($img)) {
							//%weatherImage = LoadMemoryImage($img, 0.6);
							%colorWeatherImage = LoadMemoryImage32($img);
							NeedRedraw();
						}
					}
				}
			}
		}
	}

	function DrawG15() {
		UseFont(0);
		if (IsNull(%temperature)) {
			DisplayTextWithBack("No weather data.", 0, 8);
		}else {

			DisplayTextWithBack("Temp: " +s %temperature, 0, 8);
			DisplayTextWithBack("Index: " +s %windChill, 0, 15);
			DisplayTextWithBack(%windDirection, 0, 22);
			DisplayTextWithBack(%wind, 40, 22);			

			DisplayTextWithBack("Humid: " +s %humidity, 0, 29);

			DrawImage(%weatherImage, 49, 8);

			// Clear pixels jsut about description.
			ClearRect(49,37,79,42);

			if (TextSize(%weatherDescription)[0] > 80) {
				$voffset = 1;
				UseFont(%miniFont);
			}
			DisplayTextWithBack(%weatherDescription, $voffset, 37);
			UseFont(0);

			ClearRect(79,37,159,42);
			DrawRect(80, 7, 80, 42);
			$realOffset = %offset;

			for (; $i<size(%forecast); $i++) {
				$height = 8 + 14 * $i - $realOffset;
				$day = %forecast[$i];
				DisplayTextWithBack($day[0],82,$height);
				DisplayTextRight($day[1] +s " -",135,$height);
				DisplayTextRight($day[2],159,$height);
				InvertRect(81, $height, 159, $height+6);
				if (TextSize($day[3])[0] > 79) {
					UseFont(%miniFont);
					$height++;
				}
				DisplayText($day[3],82,$height+7);
				UseFont(0);
			}
		}
		if (!%displayLocation)
			DisplayHeader();
		else {
			ClearRect(0,0,159,6);
			DisplayText(%location,0,0);
			DrawRect(0, 7, 159, 7);
		}
	}
	
	function FtoC ($F)
	{	return $F;
		//return FormatValue(($F-32)*(5/9.0));
	}
	
	function DrawG19 ($event, $param, $name, $res) {
		UseFont(%bigFont);
		DisplayHeader($res);
		if (IsNull(%temperature)) {
			DisplayTextWithBack("No weather data.", 0, 8);
		}else{
			$deg = "°" +s %units;
			DisplayTextWithBack(%location +s " weather", 1, 20);
			ColorRect(0,44,200,46,  RGBA(40,40,40,200));
			
			DisplayTextWithBack("Temp: " +s %FtoC(%temperature) +s $deg, 1, 46);
			DisplayTextWithBack("Index: " +s %FtoC(%windChill) +s $deg, 1, 66);
			//DisplayText("Wind: " +s %windDirection +s "° " +s %wind, 1, 86);
			
			DisplayTextWithBack("Wind:", 1, 86);
			DrawRotatedScaledImage(%windImage, 51, 89, %windDirection*0.01745329251994329);
			DisplayTextWithBack(%wind, 75, 86);			
			DisplayTextWithBack("Humidity: " +s %humidity, 1, 106);

			SetDrawColor(RGB(240,240,240));
			DrawImage(%colorWeatherImage, 280, 14);
			DisplayTextCentered("("+s %weatherDescription +s")", 390, 144);
			
			$pos = 172;
			ColorRect(0,$pos+1,479,$pos+22, RGBA(20,20,20,80));
			SetDrawColor(colorHighlightText);
			DisplayTextCentered("Forecast", $res[0]/2, $pos);
			SetDrawColor(colorText);
			$pos = $pos + 6;
			
			for ($i=0; $i<2; $i++) {
				$x = 190*$i+2;
				$day = %forecast[$i];
				DisplayTextCenteredWithBack($day[0],140+$x,$pos+22);
				DisplayTextCenteredWithBack(%FtoC($day[1]) +s $deg +s " - " +s %FtoC($day[2]) +s $deg,140+$x, $pos+42);
				DisplayTextCenteredWithBack($day[3],140+$x,$pos+62);
			}
		}
	}

	function Draw ($event, $param, $name, $res) {
		if (Time()-%lastUpdate >= 5*60) {
			%lastUpdate = Time();
			SpawnThread("Update", $this);
		}
		if ($res[0] >= 320){
			DrawImage(%background, 0, 0);
			%DrawG19(@$);
		}else{
			ClearScreen();
			%DrawG15();
		}

	}
};
