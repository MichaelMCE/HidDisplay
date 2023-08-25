#import <Everything.h>
#requires <Views/ApplLaunch.h>

// Note:  For editing this file, I highly recommend an editor that
// colors code.  TextPad is what I use, though it's not free.  There's
// a free version of EmEditor, which has nice unicode support as well
// as coloring code.  There's also a demo version of the non-free version,
// which is easy to accidentally download.

function Init() {
	/* Global colors.  May move them elsehwere in the future.
	 */
	colorBg = RGB(0,0,0);
	colorText = RGB(250,250,250);

	colorHighlightBg = RGBA(60,60,60,200);
	colorHighlightText = RGBA(255,255,255,200);

	// Only used on colored screens, so don't need to be careful.
	colorHighlightUnfocusedBg = RGB(40,40,100);
	colorHighlightUnfocusedText = RGB(225,225,225);

	// Used in multi-column displays to make columns easier to read.
	colorFirstColumnBg = RGBA(20,20,20,140);
	// Even/odd column color options.
	colorColumnBg = list(RGBA(20, 20, 20,120), RGBA(80, 80, 80,140));

	// currently only use by task manager.  May add elsewhere eventually.
	colorScrollBar = RGB(0,0,190);

	/* Fonts list.  Here so it's easier to change them as desired.
	 * Only fonts used for text-only screens are here.  Other screens have to have item placement
	 * modified when changing font, so they're not here.
	 */

	// Fonts for small (160 by 43 monochrome LCDs
	// Primary internal font.  It's the larger of the two small all caps fonts.
	//$smallTaskManagerFont = 0;
	//$smallCalculatorFont = Font("Lucida Sans Unicode", 24,0,0,0,CLEARTYPE_QUALITY);
	//$smallUtorrentFont = Font("04b03", 8);
	/*$smallClipboardFont = Font("04b03", 8);
	$smallRssFont = Font("04b03", 8);
	$smallRssTitleFont = Font("04b03", 8, 0, 1);*/

	// Fonts for large (320 by 160) color LCDs.  Can get rid of these lines to save some
	// memory if you don't have one.
	// Probably have to switch them all to bold, but not sure.
	//$bigTaskManagerFont = Font("Arial", 32,0,0,0,CLEARTYPE_QUALITY);
	//$bigCalculatorFont = Font("Lucida Sans Unicode", 40,0,0,0,CLEARTYPE_QUALITY);
	//$bigUtorrentFont = Font("Lucida Sans Unicode", 18,0,0,0,CLEARTYPE_QUALITY);
	//$bigClipboardFont = Font("Lucida Sans Unicode", 18,0,0,0,CLEARTYPE_QUALITY);
	//$bigRssFont = Font("Arial", 18,0,0,0,CLEARTYPE_QUALITY);
	//$bigRssTitleFont = Font("Arial", 18,0,1,0,CLEARTYPE_QUALITY);
	//$bigBrowserFont = Font("Arial", 18,0,0,0,CLEARTYPE_QUALITY);
	
	// Global
	//smallHeaderFont = 0;
	//bigHeaderFont = Font("Tahoma", 22,0,0,0,CLEARTYPE_QUALITY);

	/* Uncomment to use US english day/month strings instead of whatever Windows
	 * is configured to use.
	 */
	// SetLocale(0x0409, 1);

	/* Enable low-level communication with the G15.  Makes playing with lights faster,
	 * allows V1/V2 differentiation, enables display when LCDMon.exe is not running,
	 * allows g- and m-key handling, etc.  Does not break compatibility when LCDMon is
	 * running.
	 */
	//G15EnableHID();
	eventHandler = EventHandler();

	// Change order (or comment out) to change order of views (or hide them)
	menuHandler = MenuHandler(eventHandler,
		//DualView (
			//StatusView(
				//sbuiStatusKeys(),
			//)
			,
			StatusView(
				sbuiStatusPad(),
			),
		//),

		ApplLaunch("media1.png", "media2.png", "background15.png", CreateIconGroup(1)),
		ApplLaunch("links1.png", "links2.png", "background16.png", CreateIconGroup(2)),
		ApplLaunch("apps1.png", "apps2.png", "background18.png", CreateIconGroup(3)),
				
		// 1 to disable stealing non-media keys.
		TaskManager(1, $smallTaskManagerFont, $bigTaskManagerFont),
		
		// Modular chat.  Currently only module is IRC.  If Pidgin leak is ever fixed, I'll modify the pidgin script
		// to be compatible.
		
		/*ChatView(
			IRCChatService(list("irc.quakenet.org:6667", "anickname-", "#achannel", "#arandomchannel")),
		),*/
		

		// Doesn't add anything unless Pidgin is configured in the ini.
		//GetPidginView(),

		//MediaView(
			//VLCController(),
			//WinampController(),
			//MPCController(),
			//ItunesController(),
			// Really, really slow, probably because of DDE.
			// AMIPController(),
		//),

		CalculatorView($smallCalculatorFont, $bigCalculatorFont),
//		TextEditorView(),
//		ClipboardView($smallClipboardFont, $bigClipboardFont),

		// uTorrent.
		//DownloadView($smallUtorrentFont, $bigUtorrentFont),

		// SABnzbdView 0.1.7 by 4wd:
		// Parameters are: SABnzbdView(URL, header, subview),
		// Where: URL = "http://ip:port/sabnzbd/"
		//     header = 0 if you want the TIME/DATE header
		//              1 if you want IP:Port header
		//    subview = 0 if not using StatusView for multiple views
		//              1 if using StatusView (allows Pause/Resume/Stop)
		//
		// For SABnzbd+ 0.4.9 or higher the URL must be:
		//        URL = "http://ip:port/sabnzbd/api?apikey=<insert key>"
		//
		// If you are only interested in one SABnzbd+ daemon, you can leave the
		// URL empty and place it in the LCDMisc.ini under [URLs].  See the example.
		//SABnzbdView(, 0, 0),

		// The sample below shows using StatusView to monitor more than one SABnzbd+
		// Switching between screens is done by pressing the LCDMisc 'OK' button,
		// (third LCD button).  URL is required when using StatusView.
		// You can mix URLs for SABnzbd+ versions.
		/*
			StatusView("Images\SABnzbd.png",
			  SABnzbdView("http://1.2.3.4:8080/sabnzbd/",1, 1),
			  SABnzbdView("http://5.6.7.8:8080/sabnzbd/api?apikey=77e45017",1, 1)
			),
		*/


		/* Parameters aren't needed, but replace the 0 with a 1 to display location
		 * instead of time at the top, over the header.  No room for both,
		 * unfortunately.  Could squeeze the location and either the date or time on
		 * together for most location names, but not all, so sticking with either-or
		 * for now.  May switch to a smaller forecast ticker to squeeze everything
		 * on, or separate current weather/forecast screens.
		 */
		Clock(),

		//WeatherView(,0),

		// Doesn't work yet...  Some day...
		// May already exist, so don't call constructor.
		// GetConfigView(),

		// Sample showing how to use a StatusView to let you switch between
		// weather screens.  Note that the first is in Fahrenheit and second
		// is in Celsuis.  Just uncomment (Delete  the "/*") and change/add
		// urls as needed.
		
		//StatusView("Images\Weather.png",
		//	WeatherView("http://xml.weather.yahoo.com/forecastrss?p=usma0066&u=f", 1),
		//	WeatherView("http://xml.weather.yahoo.com/forecastrss?p=CAXX0487&u=c", 1)
		//),
		

		//CommandView(),

		// Specify urls like the next line, or modify ini:
		// RSSView(RSS_DISPLAY_TITLE | RSS_DISPLAY_DESCRIPTION,RSS_DISPLAY_TITLE | RSS_DISPLAY_DESCRIPTION,"http://news.google.com/?rss=&output=rss"),

		/*RSSView(
			RSS_DISPLAY_TITLE | RSS_DISPLAY_DESCRIPTION, // Channel title flags
			RSS_DISPLAY_TITLE | RSS_DISPLAY_DESCRIPTION, // Item text flags
			, // URL to use.  Can either be a string or a list of strings.  If left null, uses one in ini.
			, // Channels to display.  Currently, can only specify one per entry in previous parameter.
			$smallRssFont,
			$smallRssTitleFont,
			$bigRssFont,
			$bigRssTitleFont
		)*/
		,

	);

	// Turn off lights during screensaver.  Delete the "//" to enable.
	//ScreenSaverLightToggle(eventHandler);

	/* Smoothes out system volume changes.  Note that it doesn't steal the
	 * volume wheel.  Instead, it monitors the system volume setting.
	 * It will partially break the Windows Master volume slider.
	 * Intended to rectify the default ~4% adjustment per tick of the
	 * volume wheel, which is much too coarse if you use a lower setting.
	 * Experimental, breaks dragging on master volume control.  Vista
	 * already uses smaller increments, so not useful there.
	 */
	//VolumeSmoother(eventHandler);

	// Really shouldn't be here.  Left for compatibility with older scripts.
	//emailImage = LoadImage("images\email.png");

	// Tray icons.  Uncomment whichever ones you like.
	//AddCpuIcon();
	AddTotalCpuIcon();
	//AddNetIcon();
	/*while(1) {
		GetDeviceState();
	}//*/
	// MontorPower(2);
}

// #import <util/shortcut.c>


