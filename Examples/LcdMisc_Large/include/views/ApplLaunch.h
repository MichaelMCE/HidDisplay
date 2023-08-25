
function CreateIconGroup1 ()
{
	$ctrlApp = "C:\Program Files (x86)\vlcStream32\vlcsCtrl.exe";
	hSpace = 64;
	vSpace = 32;
	
	$_x = 152; $bw = 128 + hSpace;
	$_y = 64; $bh = 128 + hSpace;
	$x = $_x;
	$y = $_y;

	$iconGroup = Icons();
	$iconGroup.addIcon(0, $x, $y,				"media\play.png", $ctrlApp, "playpause");
	$x += $bw;
	$iconGroup.addIcon(0, $x, $y,				"media\stop.png", $ctrlApp, "stop");
	
	$x = $_x;
	$y += $bh;
	$iconGroup.addIcon(0, $x, $y,				"media\prev.png", $ctrlApp, "prev");
	$x = $_x + $bw;
	$iconGroup.addIcon(0, $x, $y,				"media\next.png", $ctrlApp, "next");

	$x = 544;
	$y = 127;
	$iconGroup.addIcon(0, $x, $y,				"media\up.png", $ctrlApp, "up");
	$y += ($bh/2) - 4;
	$iconGroup.addIcon(0, $x, $y,				"media\down.png", $ctrlApp, "down");

	return $iconGroup;
}

function CreateIconGroup2 ()
{
	

	$_x = 114; $bw = 150;
	$_y = 20; $bh = 150;
	
	$x = $_x;
	$y = $_y;
	
	//configure icons to appear in the Application Launcher
	
	// Button ($X, $Y, $filename, $CMD, $ARGS, $Label)
	
	$iconGroup = Icons();
	$iconGroup.addIcon(0, $x, $y,				"links\rt.png",			"https://www.rt.com/");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\youtube.png",	"https://www.youtube.com/feed/subscriptions?flow=1");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\amazon.png",		"https://www.amazon.co.uk/");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\sputnik.png",	"https://sputniknews.com/");


	$iconGroup.addIcon(0, $x=$_x, $y+=$bh,		"links\duckduckgo.png",	"https://duckduckgo.com/");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\thedonald.png",	"https://patriots.win/");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\reddit.png",		"https://old.reddit.com/");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\twitch.png",		"https://www.twitch.tv/");
	
	$iconGroup.addIcon(0, $x=$_x, $y+=$bh,		"links\twitter.png",	"https://nitter.net/DD_Geopolitics#m", "", "DD Geopolitics");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\twitter.png",	"https://twitter.com/RWApodcast", "", "RWApodcast");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\twitter.png",	"https://twitter.com/Cyberspec1", "", "CyberSpec");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"links\teleg.png",		"https://t.me/s/DDGeopolitics", "", "DD Geopolitics");
	
	return $iconGroup;
}


function CreateIconGroup3 ()
{
	$_x = 114; $bw = 150;
	$_y = 20; $bh = 150;
	
	$x = $_x;
	$y = $_y;

	$iconGroup = Icons();
	$iconGroup.addIcon(0, $x=$_x,$y+=$bh, 		"notepad2.png",		"notepad.exe");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"process.png",		"taskmgr.exe");
	$iconGroup.addIcon(0, $x+=$bw,$y,			"hdd.png",			"explorer", "::{20d04fe0-3aea-1069-a2d8-08002b30309d}"); // my computer
	
	$iconGroup.addIcon(0, $x=$_x,$y+=$bh,		"cmd.png",			"cmd.exe");
	$iconGroup.addIcon(0, $x+=$bw+64, $y,		"calculator.png",	"calc");

	return $iconGroup;
}

function CreateIconGroup ($group)
{
	if ($group == 1)
		return CreateIconGroup1();
	else if ($group == 2)
		return CreateIconGroup2();
	else if ($group == 3)
		return CreateIconGroup3();
}






