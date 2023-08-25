// Not funcitonal, due to SSL vileness.

#import <Views\View.c>
#requires <framework\header.c>
#requires <util\XML.c>
#import <constants.h>

#define JABBER_DISCONNECTED		0
#define JABBER_CONNECTING		1
#define JABBER_NEGOTIATING		2
#define JABBER_NEGOTIATING2		3
#define JABBER_START_TLS		4
#define JABBER_REALLY_STARTING_TLS 5
#define JABBER_HAPPY		   10

struct JabberView extends View {
	var %url, %username, %domain, %socket, %error, %status, %lastTry, %buffer, %lastHappy;
	function JabberView($_username, $_pass) {
		%noDrawOnCounterUpdate = 1;
		%noDrawOnAudioChange = 1;
		if (!IsString($_username)) {
			$_username = GetString("Jabber", "username");
		}
		if (!IsString($_pass)) {
			$_pass = GetString("Jabber", "password");
		}
		$temp = RegExp($_username, "^(.*)@(.*)$");
		%username = $temp[0][0];
		%domain = $temp[1][0];
		if (!size(%username)) {
			%error = "Invalid username";
		}
		else if (!size(%domain)) {
			%error = "Invalid domain";
		}
		else {
			%error = "Connecting";
			CreateTimer("CheckConnection", 60, 1, $this);
		}
	}

	function CheckConnection() {
		$t = Time();
		/* Second check imposes a 3 minute wait for receiving valid XML from start to end.
		 */
		if (%status == JABBER_HAPPY && !size(%buffer)) {
			%lastHappy = Time();
			/* Keepalive. Only send once fully negotiated so as not to mess up negotiation.
			 * If disconnected, will eventually trigger an FD_CONNECT response.
			 */
			%socket.Send(" ");
		}
		else {
			/* Time limit to connect to server.  Could do things a bit more intelligently,
			 * but not worth the effort.
			 */
			if (%status != JABBER_DISCONNECTED && $t-%lastHappy >= 3*60) {
				%status = JABBER_DISCONNECTED;
				%error = "Connection attempt failed";
				%socket = 0;
			}
			if (%status == JABBER_DISCONNECTED && $t-%lastTry >= 5*60) {
				%buffer = null;
				%lastTry = $t;
				%status = JABBER_CONNECTING;
				%error = "Connecting to " +s %domain;
				NeedRedraw();
				/*$_ips = IPAddrWait(%domain);
				if (size($_ips) == 0) {
					%error = "Can't resolve domain " +s %domain;
					%status = JABBER_DISCONNECTED;
					return;
				}//*/
				%socket = ConnectWait(%domain, 5222, IPPROTO_TCP, "HandleSocketMessage", $this);
				if (IsNull(%socket)) {
					%status = JABBER_DISCONNECTED;
					%error = "No connection";
				}
				else {
					%socket.Send("<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' to='" +s %domain +s "' version='1.0'>");
					%status = JABBER_NEGOTIATING;
					%error = "Handshaking";
					%lastHappy = Time();
				}
				NeedRedraw();
			}
		}
	}

	function Hangup($_error) {
		%error = $_error;
		if (IsObject(%socket) && %status >= JABBER_NEGOTIATING) {
			%socket.Send("</stream:stream>");
		}
		%status = JABBER_DISCONECTED;
		%socket = 0;
		%buffer = null;
		%CheckConnection();
		NeedRedraw();
	}

	function HandleSocketMessage($junk, $junk2, $msg, $_socket) {
		if ($msg == FD_CLOSE) {
			%status = JABBER_DISCONNECTED;
			%error = "Connection closed.";
			NeedRedraw();
			%CheckConnection();
			return;
		}
		else if ($msg == FD_READ) {
			while (1) {
				%buffer +=s %socket.Read();
				if (%status == JABBER_NEGOTIATING) {
					// %status = JABBER_NEGOTIATING2;
					$test = PartialParseXML(%buffer, XML_SINGLE_TAG);
					if ($test[1] != XML_HAPPY) return;
					$xml = $test[0];
					if ($xml[0] !=S "stream:stream") {
						%Hangup("Bad server response");
						return;
					}
					%buffer = substring(%buffer, $test[2], -1);
					$xml = $xml[1][1];
					$xmlns = SimpleXML($xml, list("xmlns"));
					$xmlnsStream = SimpleXML($xml, list("xmlns:stream"));
					if ($xmlnsStream !=s "http://etherx.jabber.org/streams" ||
						$xmlns !=s "jabber:client") {
						%Hangup("Bad server response");
						return;
					}
					$version = SimpleXML($xml, list("version")) + 0;
					if ($version < 1 || $version >= 2) {
						%Hangup("Unsupported server version");
						return;
					}
					// Don't think I care.
					$from = SimpleXML($xml, list("from"));
					$id = SimpleXML($xml, list("id"));
					NeedRedraw();
					%status = JABBER_NEGOTIATING2;
				}
				$test = PartialParseXML(%buffer, XML_TAG_AND_BODY);
				%buffer = substring(%buffer, $test[2], -1);
				if ($test[1] != XML_HAPPY) {
					if ($test[2]) {
						%lastHappy = Time();
					}
					return;
				}
				$xml = $test[0];
				if (%status == JABBER_NEGOTIATING2) {
					if ($xml[0] !=s "stream:features") {
						%Hangup("Bad server response");
						return;
					}
					$xml = $xml[1];
					if (SimpleXML($xml, list("starttls",,"xmlns")) !=s "urn:ietf:params:xml:ns:xmpp-tls") {
						%Hangup("TLS not supported");
						return;
					}
					%status = JABBER_START_TLS;
					%socket.Send("<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>");
					//%socket.Send("<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' to='" +s %domain +s "version='1.0'>");
					//%status == JABBER_NEGOTIATING2;
				}
				else if (%status == JABBER_START_TLS) {
					if ($xml[0] ==s "failure") {
						%Hangup("TLS failure");
						return;
					}
					if ($xml[0] !=s "proceed") {
						// ?? Just ignore.  Not sure if need special handling here for
						// messages I need to handle.
						continue;
					}
					if (SimpleXML($xml[1][1], list("xmlns")) !=s "urn:ietf:params:xml:ns:xmpp-tls") {
						%Hangup("Server is clinically insane");
						return;
					}
					%socket.StartTLS();
					%status = JABBER_REALLY_STARTING_TLS;
				}
				if (!size(%buffer)) {
					%lastHappy = Time();
					break;
				}
			}
		}
	}

	function Draw() {
		ClearScreen();
		if (%error != JABBER_HAPPY) {
			DisplayText(FormatText(%error, 160));
		}
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