// Not complete

#import <Views\View.c>
#requires <Modules\LineEditor.c>

#define CONFIG_CHECKBOX 0

struct Config {
	var %type, %labels, %object, %funct, %stringName, %count;
	function Config ($_type, $_labels, $_object, $_funct, $_stringName, $_count) {
		%type = $_type;
		%labels = $_labels;
		%object = $_object;
		%funct = $_funct;
		%stringName = $_stringName;
		%count = $_count;
	}

	function Draw($y) {
		if (%type == CONFIG_CHECKBOX) {
			$temp = TextSize(%labels);
			DisplayText(%labels,0,$y);
			return $y + $temp[1];
		}
	}
};

// Only designed for one of these to exist.
struct ConfigView extends View {
	var %options, %font;

	function ConfigView () {
		%toolbarImage = LoadImage("Images\Config.png");
		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
		%font = Font("04b03", 8);
		%options = list();
	}

	function Add($option) {
		push_back(%options, $option);
	}

	function Focus() {
		if (!%hasFocus) {
			%hasFocus = 1;
		}
	}

	function Unfocus() {
		if (%hasFocus) {
			%hasFocus = 0;
		}
	}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 0xF) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
			}
			else {
				if ($buttons & 8) {
					%Unfocus();
				}
			}
			NeedRedraw();
			return 1;
		}
	}

	function Draw() {
		ClearScreen();
		$y = 0;
		UseFont(%font);
		for ($i=0; $i<size(%options); $i++) {
			$y += %options[$i].Draw($y);
		}
		//UseFont(0);

		//%editor.Draw();
		/*
		$i = size(%buffer);
		$h = 36;
		while ($h > 0 && $i > 0) {
			$i--;
			$line = %buffer[$i];
			$formattedLine = FormatText($line, 160);
			$height = TextSize($formattedLine)[1];
			DisplayText($formattedLine, 0, $h-=$height);
		}
		//*/
	}
};

function GetConfigView() {
	if (IsNull(_GlobalConfig)) {
		_GlobalConfig = ConfigView();
	}
	return _GlobalConfig;
}

function AddConfig() {
	GetConfigView().Add(Config(@$));
}
