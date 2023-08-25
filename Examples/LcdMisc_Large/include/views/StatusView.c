#import <Views\View.c>

struct StatusView extends View {
	var %views, %active;

	/* Occasionally wait functions can confuse things a bit.  Used to make
	 * sure messages don't cross.  Most windows won't have a problem if they
	 * think they're really visible, this just adds a layer of imperfect
	 * protection.
	 */
	var %visible;

	function StatusView ($image) {
		if (IsString($image)) {
			%toolbarImage = LoadImage32($image);
			pop($,0);
		} else {
			//%toolbarImage = LoadImage32("images\status.png");
		}
		RemoveNulls($);
		%views = $;
    }

    function HandleEvent($event) {
		if ($event ==s "G15ButtonDown" || $event ==s "G15ButtonUp"){
			
		}else {
			if (%views[%active].call($event, $) || %views[%active].HandleEvent(@$))
				return 1;
		}
	}

	function Focus() {
		if (%visible) {
			%views[%active].Hide();
			%active = (%active+1)%size(%views);
			%views[%active].Show();
			NeedRedraw();
		}
	}

	function Show() {
		if (!%visible) {
			%views[%active].Show();
			%visible = 1;
		}
	}

	function Hide() {
		if (%visible) {
			%views[%active].Hide();
			%visible = 0;
		}
	}


	function Draw() {
		%views[%active].Draw(@$);
	}
};
