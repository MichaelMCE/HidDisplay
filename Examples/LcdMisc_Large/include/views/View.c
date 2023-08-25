// Needed to use LCD buttons.
#import <constants.h>
#requires <util\G15.c>

struct View {
	
	var %toolbarImage;	// Must be 32 by 32 pixels.
	var %toolbarImageL;

	// 0 if the view doesn't have focus, 1 if it does.
	// Only used by the MenuHandler, to determine if it
	// should ignore input or not.
	var %hasFocus;

	// Automatically initialized by menuHandler.
	// Used to add/remove events and make overlays.
	var %eventHandler;

	// Counter updates won't trigger a redraw if 1.
	var %noDrawOnCounterUpdate;
	// Same for audio change.
	var %noDrawOnAudioChange;

	// Just to prevent a warning.
	function View() {
	}

	function CounterUpdate() {
		if (!%noDrawOnCounterUpdate) NeedRedraw();
	}

	// If don't want to redraw
	function AudioChange() {
		if (!%noDrawOnAudioChange) NeedRedraw();
	}
};
