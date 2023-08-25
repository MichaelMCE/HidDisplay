// Config screen not finished yet...
//#import <views\ConfigView.c>
// Note:  Uses globals LightRestoreState, LightBlinkRestoreState, and alert

// Represents the state of G15's lights.
struct G15State {
	var %Light, %LCDLight, %MLights;
	function G15State($_auto, $_Light, $_LCDLight, $_MLights) {
		if ($_auto) {
			$state = G15GetState();
			%Light = $state[0];
			%LCDLight = $state[1];
			%MLights = $state[2];
		}
		else {
			%Light = $_Light;
			%LCDLight = $_LCDLight;
			%MLights = $_MLights;
		}
	}

	function setState() {
		G15SetLight(%Light);
		Wait(10);
		G15SetLight(%Light);
		Wait(10);
		G15SetLCDLight(%LCDLight);
		Wait(10);
		G15SetLCDLight(%LCDLight);
		Wait(10);
		G15SetMLights(%MLights);
		Wait(10);
		G15SetMLights(%MLights);
	}
};

struct ScreenSaverLightToggle {
	var %saving, %enabled;
	function ScreenSaverLightToggle($_eventHandler) {
		$_eventHandler.Insert(-1, $this);
		SetEventHandler("ScreenSaver", "HandleEvent", $_eventHandler);
		SetEventHandler("MonitorOff", "HandleEvent", $_eventHandler);
		//AddConfig(CONFIG_CHECKBOX, "Turn off lights on saver", $this, "SetMode", "SaverLightToggle");
		%enabled = 1;
	}

	function MonitorOff($event, $param, $starting) {
		%ScreenSaver($event, $param, $starting);
	}

	function ScreenSaver($event, $param, $starting) {
		// Fix for switching from screensaver to power save mode.
		if (%saving) {
			Wait(5000);
		}
		$power = SystemState();
		$starting = $power.screenSaverOn | $power.monitorOff;
		if (%enabled) {
			if ($starting) {
				if (!%saving) {
					$lightsOff = G15State(0);
					// Not blinking.
					if (IsNull(LightRestoreState)) {
						LightRestoreState = G15State(1);
					}
					else {
						// Blinking.
						LightBlinkRestoreState = $lightsOff;
					}
					$lightsOff.setState();
				}
			}
			else if (%saving) {
				// Not blinking
				if (IsNull(LightBlinkRestoreState)) {
					LightRestoreState.setState();
					LightRestoreState = null;
				}
				else {
					// Blinking, don't bother doing anything.
					LightBlinkRestoreState = null;
				}
			}
		}
		%saving = $starting;
	};

	function SetMode($on) {
		if (%saving && $on != %enabled) {
			%ScreenSaver();
			%enabled = $on;
			%ScreenSaver(0,0,1);
		}
		%enabled = $on;
	}
}

function LightAlert($blinks, $mainLight, $lcdLight, $mLights) {
	if ($blinks <= 0) return;
	if (!IsList(alert)) {
		alert = list();
		// Saver off.
		if (IsNull(LightRestoreState)) {
			LightRestoreState = G15State(1);
		}
		else {
			// Saver on.
			LightBlinkRestoreState = G15State(0);
		}
	}
	if ($mainLight || $lcdLight || $mLights) {
		$old = alert[0];
		if (alert[0] < $blinks) {
			alert[0] = $blinks;
			alert[1] = $mainLight;
			alert[2] = $lcdLight;
			alert[3] = $mLights;
		}
		if (!$old) {
			alert[4] = 0;
			alert[5] = G15GetState();
			CreateFastTimer("LightAlerter", 500, 1);
		}
	}
}

function LightAlerter($timer) {
	if (alert[4] == 1) {
		alert[0] --;
		if (alert[0] <= 0) {
			KillTimer($timer);
			alert = null;
			if (IsNull(LightBlinkRestoreState)) {
				LightRestoreState.setState();
				LightRestoreState = null;
			}
			else {
				LightBlinkRestoreState.setState();
				LightBlinkRestoreState = null;
			}
			return;
		}
	}
	$temp = alert[4] = !alert[4];
	if (alert[2]) {
		G15SetLCDLight(2*$temp);
		Wait(10);
		G15SetLCDLight(2*$temp);
		Wait(10);
	}
	if (alert[1]) {
		G15SetLight(2*$temp);
		Wait(10);
		G15SetLight(2*$temp);
		Wait(10);
	}
	if (alert[3]) {
		G15SetMLights(15*$temp);
		Wait(10);
		G15SetMLights(15*$temp);
	}
}

