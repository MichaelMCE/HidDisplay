// Not funcitonal, due to SSL vileness.

#import <Views\View.c>
#requires <framework\header.c>
#requires <util\XML.c>
#import <constants.h>

dll32 SampleDll "SampleDll.dll";
dll64 SampleDll "SampleDll64.dll";
dll SampleDll void SampleDrawStuff "DrawStuff" (int, int, int, int, int);
dll SampleDll int  SampleButtonDown "ButtonDown" (int mask);
dll SampleDll void SampleInit "Init" ();

struct DllSampleView extends View {
	
	function DllSampleView() {
		SampleInit();
		%toolbarImage = LoadImage("Images\Computer.png");
	}

	function G15ButtonDown($event, $param, $buttons) {
		if ($buttons & 0x3F) {
			$state = G15GetButtonsState();
			if ($buttons & 3) {
				if (!($state & ($buttons^3))) {
					$res = SampleDrawStuff($buttons);
				}
			}
			else if ($buttons & 0x30) {
				if (!($state & ($buttons^0x30))) {
					$res = SampleDrawStuff($buttons);
				}
			}
			else {
				$res = SampleDrawStuff($buttons);
			}
			if ($res) {
				%hasFocus = 0;
			}
			NeedRedraw();
			return 1;
		}
	}

	function Draw($event, $param, $name, $res) {
		ClearScreen();
		SampleDrawStuff(GetHDC(), @$res);
	}
}


/*<?xml version='1.0' encoding='UTF-8'?>

<stream:stream
xmlns:stream="http://etherx.jabber.org/streams"
xmlns="jabber:client"
from="supergoat"
id="84b1b26f"
xml:lang="en"
version="1.0">

<stream:features>

<starttls xmlns="urn:ietf:params:xml:ns:xmpp-tls">
</starttls>

<mechanisms xmlns="urn:ietf:params:xml:ns:xmpp-sasl">
	<mechanism>DIGEST-MD5</mechanism>
	<mechanism>PLAIN</mechanism>
	<mechanism>ANONYMOUS</mechanism>
	<mechanism>CRAM-MD5</mechanism>
</mechanisms>

<compression xmlns="http://jabber.org/features/compress">
	<method>zlib</method>
</compression>

<auth xmlns="http://jabber.org/features/iq-auth"/>
<register xmlns="http://jabber.org/features/iq-register"/>
</stream:features>
//*/