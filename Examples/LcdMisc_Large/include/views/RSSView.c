#import <Views\View.c>
#requires <list.c>
#requires <util\html.c>
#requires <util\XML.c>
#requires <util\Graphics.c>

#define RSS_DISPLAY_TITLE 1
#define RSS_DISPLAY_DESCRIPTION 2

// Note that channel info is also stored as an ItemInfo.
struct ItemInfo {
	var %title, %desc, %url, %formattedTitle, %titleHeight, %descHeight;
	function ItemInfo($flags, $item, $font1, $font2, $width) {
		UseFont($font1);
		%title = StripHTML(SimpleXML($item, list("title"))[3]);
		%formattedTitle = FormatText(%title, $width);
		%titleHeight = TextSize(%formattedTitle)[1];
		%url = SimpleXML($item, list("link"))[3];
		if ($flags & RSS_DISPLAY_DESCRIPTION) {
			UseFont($font2);
			%desc = FormatText(StripHTML(SimpleXML($item, list("description"))[3]), $width);
			%descHeight = TextSize(%desc)[1];
		}
	}
};

struct RSSView extends View {
	var %channelFlags,
		%itemFlags,
		%feeds,
		%channels,
		%nextUpdates,
		%nextUpdate,
		%channelItems,
		%items,
		%nextItems,
		%offset,
		%scrollTimer,
		%smallFont,
		%smallTitleFont,
		%bigFont,
		%bigTitleFont,
		%baseFontHeight,
		%top,
		%prevTop,
		%delay,
		%formatWidth,
		%updating;

	function RSSView ($_channelFlags, $_itemFlags, $_feeds, $_channels, $smallRssFont, $smallRssTitleFont, $bigRssFont, $bigRssTitleFont) {
		%delay = 150;
		%baseFontHeight = 10;

		%nextUpdate = Time();
		if (size($_feeds) == 0) {
			$_feeds = GetString("URLs", "RSS");
		}
		%toolbarImage = LoadImage32("Images\rss1.png");
		%toolbarImageL = LoadImage32("Images\rss2.png");
		%channelFlags = $_channelFlags;
		%itemFlags = $_itemFlags;
		if (!IsList($_feeds))
			%feeds = list($_feeds);
		else
			%feeds = $_feeds;
		if (!IsList($_channels))
			%channels = list($_channels);
		else
			%channels = $_channels;
		%nextUpdate = Time();
		%nextUpdates = list(%nextUpdate);
		if (IsList($_feeds)) {
			%feeds = $_feeds;
			%channels = $_channels;
			for ($i = size(%feeds);$i>0;)
				%nextUpdates[--$i] = %nextUpdate;
		}

		%channelItems = list();

		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
		%items = list();

		%smallFont = $smallRssFont;
		%smallTitleFont = $smallRssTitleFont;
		%bigFont = $bigRssFont;
		%bigTitleFont = $bigRssTitleFont;
	}

	function Update() {
		%updating = 1;
		$t = Time();
		$newFormatWidth = 480;
		
		if ($newFormatWidth == 480) {
			$font = %bigFont;
			$titleFont = %bigTitleFont;
			%delay = 125;
		}else {
			$font = %smallFont;
			$titleFont = %smallTitleFont;
			%delay = 250;
		}
		if (%scrollTimer) ModifyFastTimer(%scrollTimer, %delay);
		UseFont($font);
		%baseFontHeight = GetFontHeight();
		for (; $i<size(%feeds); $i++) {
			if ($t >= %nextUpdates[$i] || %formatWidth != $newFormatWidth) {
				$xml = HttpGetWait(%feeds[$i]);
				if (IsNull($xml) || IsNull($xml = ParseXML($xml))) {
					%nextUpdates[$i] = Time() + 5*60;
					continue;
				}
				$itemList = list();

				// rss 2.0
				$channelList = SimpleXML($xml, list("rss"));
				if (IsNull($channelList)) {
					// rss 1.0
					$channelList = SimpleXML($xml, list("rdf:RDF"));
					if (IsNull($channelList)) {
						// rss 1.1.  Don't ask me why the heck they can't even all
						// agree on where to keep the list of channels.  Difference
						// here is completely arbitrary.
						$channelList = $xml;
					}
				}

				if (!IsNull($channelList))
					for ($j = 0; $j < size($channelList); $j+=2) {
						if ($channelList[$j] !=s "channel") {
							if ($channelList[$j] ==s "item") {
								// rss 1.0
								if (%itemFlags & RSS_DISPLAY_TITLE) {
									$item = ItemInfo(%itemFlags, $channelList[$j+1], $titleFont, $font, $newFormatWidth);
									if (size($item.title)) {
										$itemList[size($itemList)] = $item;
									}
								}
							}
							continue;
						}
						$channel = $channelList[$j+1];

						$ttl2 = SimpleXML($item, list("ttl"))[3];
						if (IsString ($ttl2) && ($ttl==0 || ($ttl2 < $ttl && $ttl2 > 0))) {
							$ttl = $ttl2+0;
						}

						$item = ItemInfo(%channelFlags, $channel, $titleFont, $font, $newFormatWidth);
						if (size(%channels[$i]) && %channels[$i] !=s $item.title) {
							// Skip channel if name specified and doesn't match.
							continue;
						}
						if (%channelFlags & RSS_DISPLAY_TITLE && size($item.title)) {
							$itemList[size($itemList)] = $item;
						}

						// rss 2.0
						if (%itemFlags & RSS_DISPLAY_TITLE) {

							$channel2 = SimpleXML($channel, list("items"));
							if (!IsNull($channel2)) {
								// rss 1.1
								$channel = $channel2;
							}

							for ($k = 0; $k < size($channel); $k+=2) {
								if ($channel[$k] !=S "item") continue;
								$item = ItemInfo(%itemFlags, $channel[$k+1], $titleFont, $font, $newFormatWidth);
								if (size($item.title)) {
									$itemList[size($itemList)] = $item;
								}
							}
						}
					}

				if ($ttl <= 0) $ttl = 60;
				else if ($ttl <= 5) $ttl = 5;
				else if ($ttl > 180) $ttl = 180;
				%nextUpdates[$i] = Time() + $ttl*60;


				%channelItems[$i] = $itemList;
				$last = $itemList[size($itemList)-1];
				if (size($last.desc))
					$item.descHeight += 12;
				else
					$item.titleHeight += 12;
			}
		}
		%nextItems = list();
		for ($i=0; $i<size(%feeds); $i++) {
			%nextItems = list(@%nextItems, @%channelItems[$i]);
		}
		/* Update instantly if recently switched to this view or
		 * nothing's showing.  Otherwise, will update at end of
		 * current cycle.
		 */
		if (!size(%items) || %offset >= 0) {
			%items = %nextItems;
			%top = 0;
		}
		if (%formatWidth != $newFormatWidth) {
			%formatWidth = $newFormatWidth;
			%items = %nextItems;
		}
		%updating = 0;
	}

	function Focus() {
		$url = %items[%top].url;
		if (size($url))
			Run($url,,2);
	}

	function Show() {
		if (!%scrollTimer) {
			%offset = 43;
			%items = %nextItems;
			%top = 0;
			%prevTop = 0;
			%scrollTimer = CreateFastTimer("Bump", %delay,,$this);
		}
	}

	function Hide() {
		if (%scrollTimer) {
			KillTimer(%scrollTimer);
			%scrollTimer = 0;
		}
	}

	function Bump() {
		%offset--;
		NeedRedraw();
	}


	function Draw ($event, $param, $name, $res) {
		$w = $res[0];
		$h = $res[1]-1;

		// Force update when new G19 detected.
		if (%nextUpdate <= Time() || %formatWidth < $w) {
			%nextUpdate = Time()+60;
			SpawnThread("Update", $this);
		}

		if ($w <= 160) {
			$font = %smallFont;
			$titleFont = %smallTitleFont;
		}
		else {
			$font = %bigFont;
			$titleFont = %bigTitleFont;
		}

		ClearScreen();
		UseFont($font);
		$fh = GetFontHeight();
		$pos = %offset*$fh/%baseFontHeight;
		UseFont($titleFont);
		$lst = %items;

		if (!size($lst)) {
			if (%updating) {
				DisplayText("Updating data...");
			}
			else {
				DisplayText("Update failed...");
			}
			return;
		}

		$topOffset = 0;
		while ($pos < $h) {
			if ($i == size($lst)) {
				if ($pos < $fh) {
					$wrap = 1;
					$wrapTo = $pos;
				}
				$lst = %nextItems;
				$i = 0;
			}
			$s1 = $lst[$i].titleHeight*$fh/%baseFontHeight;
			if ($pos + $s1 >= $fh) {
				DisplayText($lst[$i].formattedTitle,0,$pos);
			}
			if ($pos < $fh) {
				$topTitle = $lst[$i].title;
				$topOffset = $pos;
				%top = $i;
			}
			$pos += $s1;
			if (!size($lst[$i].desc)) {
				$pos += $fh;
				$i++;
				continue;
			}
			UseFont($font);

			$pos += $fh/2;
			$s2 = $lst[$i].descHeight*$fh/%baseFontHeight;
			if ($pos + $s2 > 0) {
				DisplayText($lst[$i].desc,0,$pos);
			}
			$pos += $s2;
			$pos += $fh;
			$i++;
			UseFont($titleFont);
		}

		if (%hasFocus || $h < 100) {
			//SetDrawColor(colorHighlightBg);
			DrawRectangleFilled(0,0,$w,$fh,colorHighlightBg);
			//SetDrawColor(colorHighlightText);
		}else {
			//SetDrawColor(colorHighlightUnfocusedBg);
			DrawRectangleFilled(0,0,$w,$fh,RGBA(70,70,70,200));
			//SetDrawColor(colorHighlightUnfocusedText);
		}

		if ($topOffset <= 0) {
			DisplayText($lst[%top].title,0,0);
			%prevTop = %top;
		}
		else {
			// Scroll out old text.
			DisplayText($lst[%prevTop].title, 0, $topOffset-$fh);
			DrawClippedText($topTitle, 0, $topOffset, $w, $fh+1);
		}
			SetDrawColor(colorText);

		//DrawRect(0,$fh-1,$w,$fh-1);

		if ($wrap) {
			%offset = $wrapTo*%baseFontHeight/$fh;
			%items = %nextItems;
		}
	}
};

