#import <Views\View.c>

// Note:  Holds two subviews, sends focus and events to both views
// First view draws to G15 style LCDs, second to G19s.  Cannot
// be focused, designed to hold two status screens or two StatusViews.
struct DualView extends View {
	var %views;
	var %visible;
	var %switchView;

	function DualView($image) {
		if (IsString($image)) {
			%toolbarImage = LoadImage32($image);
			pop($,0);
		}else{
			%toolbarImage = ImageLoad("status1.png");
			%toolbarImageL = ImageLoad("status2.png");
		}

		RemoveNulls($);
		%views = $;
		
		%switchView = 0;
    }

    function HandleEvent($event) {
		for ($i=0; $i<size(%views); $i++) {
			if (%views[$i].call($event, $) || %views[$i].HandleEvent(@$))
				$out = 1;
		}
		return $out;
	}

	function Focus() {
		if (%visible) {
			for ($i=0; $i<size(%views); $i++) {
				%views[$i].Focus();
			}
		}
	}

	function Show() {
		if (!%visible) {
			for ($i=0; $i<size(%views); $i++) {
				%views[$i].Show();
			}
			%visible = 1;
		}
	}

	function Hide() {
		if (%visible) {
			for ($i=0; $i<size(%views); $i++) {
				%views[$i].Unfocus();
				%views[$i].Hide();
			}
			%visible = 0;
		}
	}


	function Draw ($event, $param, $name, $res) {
		if (%switchView == 0){
			%switchView = 1;

			if (DUAL_SCREEN){					// DUAL_SCREEN defined in constants.h
				//razersb_SelectDisplayKeys();
				%views[0].Draw(@$);
				NeedRedraw(1);
			}
				
		}else if (%switchView == 1){
			%switchView = 0;
				
			//razersb_SelectDisplayPad();				
			%views[1].Draw(@$);
		}
	}
};
