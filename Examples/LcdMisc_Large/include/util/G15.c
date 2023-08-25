#import <constants.h>





function IsG19Installed() {
	return size(GetG15s(SDK_320_240_32));
}

// If left/right or up/down are both pressed, returns 0.
// Can specify individual keyboard to check, if so desired.
function FilterButton($button, $g15id) {
	if ($button & 0x33) {
		if (G15GetButtonsState($g15id) & (($button >> 1) + ($button << 1)))
			return 0;
	}
	return $button;
}
