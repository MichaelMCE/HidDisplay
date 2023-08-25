#import <constants.h>
#requires <util\G15.c>
#requires <util\Graphics.c>
#import <list.c>
#requires <framework\Overlay.c>
#requires <views\clock.c>


struct touchArea {
	var %x1, %y1;
	var %x2, %y2;
	var %event, %args;

	function touchArea ($X1, $Y1, $X2, $Y2, $EVENT, $ARGS){
		%x1 = $X1;
		%y1 = $Y1;
		%x2 = $X2;
		%y2 = $Y2;
		%event = $EVENT;
		%args = $ARGS;
	}

	function draw (){
		DrawRectangle(%x1, %y1, %x2, %y2, RGB(255,255,0));
	}
	function isButtonTouched ($X, $Y){
		if ($X >= %x1 && $X <= %x2){
			if ($Y >= %y1 && $Y <= %y2){	
				return 1;
			}
		}
		return 0;
	}
	function run (){
		//
	}
};

struct EventHandler extends ObjectList {
	var %overlay;
	var %btnOverlays, %tbtnOverlays;
	
	function EventHandler() {
		%ObjectList();
		SetEventHandler("G15ButtonDown", "HandleEvent", $this);
		SetEventHandler("Draw", "DrawEvent", $this);
		SetEventHandler("KeyUp", "HandleEvent", $this);
		SetEventHandler("KeyDown", "HandleEvent", $this);
		SetEventHandler("WMUser", "WMUser", $this);
		SetEventHandler("CounterUpdate", "CounterUpdate", $this);
		SetEventHandler("AudioChange", "HandleEvent", $this);
		SetEventHandler("Quit", "Quit", $this);
		SetEventHandler("touchDown2", "touchDownEvent", $this);
		SetEventHandler("MediaControl", "HandleEvent", $this);
		
		SetEventHandler("DriveChange", "driveNotificationEvent", $this);
		
		SetEventHandler("sbuiEventDKDown", "sbuiDKStateChanged", $this);
		SetEventHandler("sbuiEventDKUp", "sbuiDKStateChanged", $this);
		SetEventHandler("sbuiEventDK", "sbuiDKStateChanged", $this);
		
		SetEventHandler("sbuiEventClose", "sbuiApplStateChanged", $this);
		SetEventHandler("sbuiEventExit", "sbuiApplStateChanged", $this);
		SetEventHandler("sbuiEventActivate", "sbuiApplStateChanged", $this);
		SetEventHandler("sbuiEventDeactivate", "sbuiApplStateChanged", $this);
		
				
				
		$_W = DWIDTH-1; $_H = DHEIGHT-1;
		%tbtnOverlays = 0;
		%btnOverlays = list();
		%btnOverlays[%tbtnOverlays++] = touchArea(2, 200, 2+82, 200+82, "G15ButtonDown", G15_LEFT);
		%btnOverlays[%tbtnOverlays++] = touchArea($_W-82, 200, $_W-2, 200+82, "G15ButtonDown", G15_RIGHT);
		%btnOverlays[%tbtnOverlays++] = touchArea(($_W/2)-(82/2), 2, ($_W/2)+(82/2), 82, "G15ButtonDown", G15_UP);
		%btnOverlays[%tbtnOverlays++] = touchArea(($_W/2)-(82/2), $_H-82, ($_W/2)+(82/2), $_H-2, "G15ButtonDown", G15_DOWN);
		
		%btnOverlays[%tbtnOverlays++] = touchArea(($_W/2)-(82/2), 200, ($_W/2)+(82/2), 200+82, "G15ButtonDown", G15_OK);
		%btnOverlays[%tbtnOverlays++] = touchArea(2, $_H-82, 82, $_H-2, "G15ButtonDown", G15_CANCEL);
		%btnOverlays[%tbtnOverlays++] = touchArea($_W-82, $_H-82, $_W-2, $_H-2, "G15ButtonDown", G15_MENU);
		%btnOverlays[%tbtnOverlays++] = touchArea(0, 0, 81, 81, "G15ButtonDown", SHOW_CLOCKA);
		%btnOverlays[%tbtnOverlays++] = touchArea($_W-82, 0, $_W, 81, "G15ButtonDown", SHOW_CLOCKD);
		
	}

	function driveNotificationEvent ($event, $param, $notifyType, $drive, $isUSB) {
		//WriteLogLn($event +s " @@ " +s $notifyType +s " - " +s $drive +s " - " +s $isUSB, 1);
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].call($event, $) | %objs[$i].HandleEvent(@$))
				return;
		}
		
		/*
		notifyTypes are:
		DRIVE_ADDED				drive has been inserted, isUSB is set if its a USB drive
		DRIVE_REMOVED			drive containing any and all media attached to drive, has been unpluged/ejected from computer. isUSB isn't valid
		DRIVE_MEDIA_INSERTED	media (eg; SD card) inserted in to drive (eg; USB memory/SD card reader)
		DRIVE_MEDIA_REMOVED		as above but removed
		*/
		
	}
	
	function sbuiApplStateChanged ($event, $param, $state) {
		//WriteLogLn($event +s " @@ " +s $state, 1);
		
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].call($event, $) | %objs[$i].HandleEvent(@$))
				return;
		}
		
		//%HandleEvent($event, $param, $key);
		
		if ($event == "sbuiEventClose" || $event == "sbuiEventActivate")
			Quit();
	}
	function sbuiDKStateChanged ($event, $param, $key) {
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].call($event, $) | %objs[$i].HandleEvent(@$))
				return;
		}
		
		//%HandleEvent($event, $param, $key);

		if ($event ==s "sbuiEventDKDown" || $event ==s "sbuiEventDKUp"){
			//WriteLogLn($event +s " - " +s $key, 1);
		}
	}

	function CounterUpdate() {
		NeedRedrawIcons();
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].CounterUpdate(@$) | %objs[$i].HandleEvent(@$)) break;
		}
	}

	function WMUser($event, $param, $wParam, $lParam) {
		if ($wParam == 0 && $lParam == 0) {
			// Allows other apps to force a redraw.
			NeedRedraw();
		} else {
			%HandleEvent(@$);
		}
	}

	function Quit() {
		%HandleEvent("Quit");
		%objs = list();
	}

	// Used by either non-overlays to kick out an overlay, or by a new overlay to
	// replace an old overlay.  Should not be called by an overlay to remove itself,
	// unless it doesn't mind the call to Close().
	function SetOverlay ($newOverlay) {
		NeedRedraw();
		if (!IsNull(%overlay)) {
			// Just in case.
			$oldOverlay = %overlay;
			%overlay = null;

			%Remove($oldOverlay);

			// An overlay should *NOT* call SetOverlay in response to this event.
			$oldOverlay.Close();
		}
		if (IsObject($newOverlay)) {
			%overlay = $newOverlay;
			%Insert(-1, $newOverlay);
		}
	}

	// Intended for overlays to use to remove themselves.  Does not call Close().
	function RemoveOverlay ($oldOverlay) {
		if (Equals($oldOverlay, %overlay)) {
			NeedRedraw();
			%Remove($oldOverlay);
			%overlay = null;
		}
	}
	
	function touchDownEvent ($event, $param, $tpos) {
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].call($event, $) | %objs[$i].HandleEvent(@$))
				return;
		}
		
		$tpoint = strsplit($tpos, ",", 0, 0, 0);
		$x = $tpoint[0];
		$y = $tpoint[1];
		
		for ($i = 0; $i < size(%btnOverlays); $i++){
			$button = %btnOverlays[$i];
			if ($button.isButtonTouched($x, $y)){
				%HandleEvent($button.event, $param, $button.args);
				break;
			}
		}
	}
	
	function HandleEvent ($event, $param, $buttons) {
		$i = size(%objs);
		while ($i--) {
			if (%objs[$i].call($event, $) | %objs[$i].HandleEvent(@$))
				break;
		}
	}

	function DrawEvent ($event, $param) {
		$len = size(%objs);
		SetBgColor(colorBg);
		SetDrawColor(colorText);
		
		while ($i < $len) {
			if (%objs[$i].call($event, $)) break;
			$i++;
		}
		
		/*for ($i = 0; $i < size(%btnOverlays); $i++){
			$ov = %btnOverlays[$i];
			DrawRectangle($ov.x1, $ov.y1, $ov.x2, $ov.y2, RGB(255,255,0));
		}*/
	}
};


struct MenuHandler {
	var %eventHandler;
	var %views, %currentView, %currentViewOld;
	var %hideTimer;
	var %buttons;
	
	function MenuHandler($Handler) {
		%views = list();
		%eventHandler = $Handler;
		%eventHandler.Insert(0, $this);
		$[0] = 0;
		%AddView(@$);
		toolbarVisible = 0;

		%buttons = Icons();		
		%buttons.addIcon(2, 0, 180, "left.png", "G15ButtonDown", G15_LEFT);					// these two buttons are 128x120
		%buttons.addIcon(2, DWIDTH-129, 180, "right.png","G15ButtonDown", G15_RIGHT);
		
		//%buttons.addIcon(2, 208, 1, "up.png",  "G15ButtonDown", G15_UP);
		//%buttons.addIcon(2, 208,168,"down.png","G15ButtonDown", G15_DOWN);
		//%buttons.addIcon(2, 208, 104,"ok.png", "G15ButtonDown", G15_OK);
		//%buttons.addIcon(2, 1  ,206,"cancel.png", "G15ButtonDown", G15_CANCEL);
		SetEventHandler("touchDown", "touchDownEvent", $this);
		SetEventHandler("ShowToolbar", "ShowToolbarFn", $this);

		SetEventHandler("sbuiDKDown"     ,"sbuiDKDown", $this);
		SetEventHandler("sbuiDKUp"       ,"sbuiDKUp", $this);

		SetEventHandler("sbuiDK"         ,"sbuiDK", $this);
		SetEventHandler("sbuiClose"      ,"sbuiClose", $this);
		SetEventHandler("sbuiExit"       ,"sbuiExit", $this);
		SetEventHandler("sbuiActivate"   ,"sbuiActivate", $this);
		SetEventHandler("sbuiDeactivate" ,"sbuiDeactivate", $this);
		
		SetEventHandler("quit" ,"QuitEvent", $this);
				
		clock = Clock();
		drawClock = 0;
		drawClockIdx = CLOCK_DEFAULT;  // this sets the default clock
		
	}

	function QuitEvent ($event, $param, $arg) {
		Quit();
	}
			
	function sbuiDKDown ($event, $param, $key) {
		$alist = list();
		$alist[0] = $key;
		PostEvent("sbuiEventDKDown", 1, $alist);

		if ($key == SBUI_DK_1 || $key == SBUI_DK_5){	// dk_1:left, dk_5:right - change view

			%RestartHideTimer();
			toolbarVisible = 2;				// 2:signal not to display left/right touch icons
			drawClock = 0;
			%signalToolbarStateChange();
				
			if ($key == SBUI_DK_1) {
				$newViewOffset = size(%views)-1;
				%SetView(($newViewOffset+%currentView) % size(%views));

			}else if ($key == SBUI_DK_5) {
				$newViewOffset = 1;
				%SetView(($newViewOffset+%currentView) % size(%views));
			}
			
			if (%views[%currentView].hasFocus){
			 	%views[%currentView].Unfocus();
			}
		}else{
			if (toolbarVisible){
				toolbarVisible = 0;
				%signalToolbarStateChange();
			}
		}
		
	}
	function sbuiDKUp ($event, $param, $key) {
		$alist = list();
		$alist[0] = $key;
		PostEvent("sbuiEventDKUp", 1, $alist);
	}
	function sbuiDK ($event, $param, $arg) {
		$alist = list();
		$alist[0] = $arg;
		PostEvent("sbuiEventDK", 1, $alist);
	}
	function sbuiClose ($event, $param, $arg) {
		$alist = list();
		$alist[0] = $arg;
		PostEvent("sbuiEventClose", 1, $alist);
	}
	function sbuiExit ($event, $param, $arg) {
		$alist = list();
		$alist[0] = $arg;
		PostEvent("sbuiEventExit", 1, $alist);
	}
	function sbuiActivate ($event, $param, $arg) {
		$alist = list();
		$alist[0] = $arg;
		PostEvent("sbuiEventActivate", 1, $alist);
	}
	function sbuiDeactivate ($event, $param, $arg) {
		$alist = list();
		$alist[0] = $arg;
		PostEvent("sbuiEventDeactivate", 1, $alist);
	}

	function touchDownEvent ($event, $param, $tpos) {
		if (toolbarVisible){
			$tpoint = strsplit($tpos, ",", 0, 0, 0); // "x,y,dt,time"
			if (%buttons.runTouchedIcon($tpoint[0], $tpoint[1]))
				return;
		}
		$alist = list();
		$alist[0] = $tpos;
		PostEvent("touchDown2", 1, $alist);
	}
	
	function AddView ($index/*, view1, view2,...*/) {
		for ($i=1; $i<size($);) {
			if (!IsObject($[$i])) {
				pop($, $i);
				continue;
			}
			$[$i].eventHandler = %eventHandler;
			$i++;
		}
		if (size($) < 2) return;
		if ($index >= %currentView) {
			if (!size(%views)) {
				%currentView = size(%views);
				insert(%views, @$);
				%SetView(0);
				return;
			}
			$index += size(%currentView);
		}
		insert(%views, @$);
	}

	function SetView ($val) {
		if (IsObject($val)) {
			while ($i < size(%views)) {
				if (Equals(%views[$i], $val))
					break;
				$i++;
			}
			$val = $i;
		}
		if (IsInt($val) && $val >= 0 && $val < size(%views)) {
			if (%currentView < size(%views)) {
				%views[%currentView].Hide();
				%eventHandler.Remove(%views[%currentView]);
			}
			%currentViewOld = %currentView;
			%currentView = $val;
			%eventHandler.Insert(0, %views[%currentView]);
			%views[%currentView].Show();
			
			$alist = list();
			$alist[0] = %currentView;
			$alist[1] = %currentViewOld;
			PostEvent("viewChanged", 1, $alist);
		}
		NeedRedraw();
	}

	function Remove(/*obj1, obj2, etc*/) {
		for ($i=0; $i<size($); $i++) {
			listRemove(%views, $[$i]);
		}
	}

	function signalToolbarStateChange (){
		$i = size(%views);
		while ($i--)
			%views[$i].Toolbar(toolbarVisible);
	}
	
	function ShowToolbarFn ($event, $param, $args) {
		%RestartHideTimer();
		toolbarVisible = 1;
		%signalToolbarStateChange();
	}

	function setClockState ($state) {
		//WriteLogLn("setClockState :" +s drawClock, 1);

		if (drawClock)
			drawClock = 0;
			
		else if ($state == SHOW_CLOCKA)
			drawClock = 1;
			
		else if ($state == SHOW_CLOCKD)
			drawClock = 2;

		if (toolbarVisible){
			toolbarVisible = 0;
			%signalToolbarStateChange();
		}
		NeedRedraw();
	}
	
	function selectClockNext () {
		if (++drawClockIdx == 10)
			drawClockIdx = 0;
		NeedRedraw();
	}

	function selectClock ($idx) {
		drawClockIdx = $idx;
		NeedRedraw();
	}
	
	function G15ButtonDown ($event, $param, $button) {
		$button = FilterButton($button);
		
		if ($button == SHOW_CLOCKA){
			if (!drawClock)
				drawClock = 1;
			else
				drawClock = 0;
				
			if (toolbarVisible){
				toolbarVisible = 0;
				%signalToolbarStateChange();
			}
			NeedRedraw();
			return 1;
			
		}else if ($button == SHOW_CLOCKD){
			if (!drawClock){
				drawClock = 2;
			}else
				drawClock = 0;
				
			if (toolbarVisible){
				toolbarVisible = 0;
				%signalToolbarStateChange();
			}
			NeedRedraw();
			return 1;
			
		}else if (($button & 0x4F) && !%views[%currentView].hasFocus) {
			if ($button == G15_LEFT || $button == G15_RIGHT) {
				%RestartHideTimer();
				toolbarVisible = 1;
				drawClock = 0;
				%signalToolbarStateChange();
				
				if ($button == G15_LEFT) {
					$newViewOffset = size(%views)-1;
				}else{
					$newViewOffset = 1;
				}
				%SetView(($newViewOffset+%currentView) % size(%views));
				
			}else if ($button == G15_MENU) {
				// Doesn't hurt to call it when not needed.
				%RestartHideTimer();
				toolbarVisible ^= 1;
				drawClock = 0;
				%signalToolbarStateChange();
				
			}else if ($button == G15_OK) {
				%views[%currentView].Focus();
				toolbarVisible = 0;
				drawClock = 0;
				%signalToolbarStateChange();
				
			}else if ($button == G15_CANCEL){
				if (!toolbarVisible) {
					MessageBoxOverlay(%eventHandler, "Quit LCD Miscellany?|n(" +s GetVersionString(1) +s ")", "Yes", "No", "Quit", null);
				}
				toolbarVisible = 0;
				drawClock = 0;
				%signalToolbarStateChange();
			}
			NeedRedraw();
			return 1;
		}
	}

	function RestartHideTimer () {
		if (!%hideTimer) {
			%StartHideTimer();
		}else{
			%hideTimer = GetTickCount () + 4000;
		}
	}

	function StartHideTimer () {
		if (!%hideTimer) {
			%hideTimer = GetTickCount () + 4000;
			CreateTimer("AutoHide", 1, 0, $this);
		}
	}

	function AutoHide ($id) {
		if (GetTickCount() - %hideTimer >= 0 || %hideTimer == 0) {
			KillTimer($id);
			%hideTimer = 0;
			toolbarVisible = 0;
			%signalToolbarStateChange();
			NeedRedraw();
		}
	}

	function Draw ($event, $param, $name, $res) {
		//if (razersb_GetSelectedDisplay() == SBUI_DISPLAY_KEYS)
		//	return;
		
		if (toolbarVisible){
			$width = $res[0];
			$height = $res[1]-1;
			$iconHeight = $height-65;
			$iwidth = 64;
			$iseparation = 4;
			$iwtotal = $iwidth + $iseparation;

			$len = $iwtotal * size(%views);
			if ($len <= $width){
				$pos = $width/2 - $len/2;
			}else{
				$pos = $width/2-6 - $iwtotal*%currentView;
				if ($pos > 0) $pos = 0;
				else if ($pos < $width-$len) $pos = $width-$len;
			}
			
			$currentViewX = 0;
			
			for (; $i<size(%views); $i++){
				if (%currentView == $i)
					$currentViewX = $pos-16;
				else
					DrawImage(%views[$i].toolbarImage, $pos, $height-1-%views[$i].toolbarImage.Size()[1]);
				$pos += $iwtotal;
			}

			DrawImage(%views[%currentView].toolbarImageL, $currentViewX, $height-1-%views[%currentView].toolbarImageL.Size()[1]);
			
			if (toolbarVisible != 2)	// don't draw icons if event arrived from the DynamicKeys
				%buttons.drawIcons();
			
		/*}else if (!drawClock ){
			clock.DrawForeground(1);*/
			
		}else if (drawClock == 1 || drawClock == 2){
				clock.DrawForeground(1+drawClockIdx);		// analogue

		//}else if (drawClock == 2){
		//		clock.DrawForeground(0);		// digital
		}
	}
};




