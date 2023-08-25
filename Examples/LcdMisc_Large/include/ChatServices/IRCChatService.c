#import <ChatServices\ChatService.c>
#import <constants.h>
#requires <util\Time.c>

#define IRC_DISCONNECTED	0
// Only set durung call to ConnectWait(), to prevent synchronization issues..
#define IRC_CONNECTING		1
#define IRC_HANDSHAKING		2
#define IRC_CONNECTED		3

#define CHANNEL_OPERATOR	1
#define CHANNEL_HALF_OP		2
#define CHANNEL_VOICED		4

function IRCStrip($text) {
	while (1) {
		$temp = RegExp($text, "(|3##?,##?)");
		if (IsNull($temp)) break;
		$text = strreplace($text, $temp[0][0], "", 1, $temp[0][1]);
	}
	while (1) {
		$temp = RegExp($text, "(|3##?)");
		if (IsNull($temp)) break;
		$text = strreplace($text, $temp[0][0], "", 1, $temp[0][1]);
	}
	//return strreplace(strreplace(strreplace(strreplace($text, "|2", ""), "|x16", ""), "|x1F", ""), "|3", "");
	return strreplace(strreplace(strreplace($text, "|x16", ""), "|x1F", ""), "|3", "");
}

struct IRCServer extends Channel {
	var %nick, %DNS;

	var %status, %lastMessageTime;

	var %socket, %buffer;

	var %retryTimer;
	function IRCServer($_DNS, $_nick) {
		%lastMessageTime = -1;
		if (IsNull($_DNS)) {
			$_DNS = "No Server";
		}
		%chat = Chat();
		%SetDNS($_DNS);
		if (IsNull($_nick)) {
			$_nick = "NoName";
		}
		%SetNick($_nick);
		%indent = 1;
	}

	function SetDNS($_DNS) {
		%DNS = $_DNS;
		%SetID();
	}

	function SetNick($_nick) {
		%chat.self = %nick = $_nick;
		%SetID();
	}

	function SetID() {
		%chat.id = %name = %DNS +s " (" +s %nick +s ")";
	}

	function AddText($text) {
		%chat.AddText(IRCStrip($text));
		NeedRedraw();
	}
};

// Set only when parted.  Not when offline, used for rejoining.
#define CHANNEL_NOT_CONNECTED		0

#define CHANNEL_JOINING				1
// Set once have names list.
#define CHANNEL_JOINED				2

// Used for PMs in addition to channels.
struct IRCChannel extends Channel {
	var %server, %nick, %channel, %joined;
	function IRCChannel($_server, $_name, $_nick) {
		%server = $_server;
		%chat = Chat($_name);
		%name = $_name;
		%SetNick($_nick);
		%indent = 2;
		if ($_name[0] ==s "#" || $_name[0] == "&")
			%channel = 1;
	}

	function SetNick($_nick) {
		%chat.self = %nick = $_nick;
	}

	function AddText($text) {
		%chat.AddText(IRCStrip($text));
		NeedRedraw();
	}
};

struct IRCChatService extends ChatService {
	var %servers, %lastNick;
	function IRCChatService() {
		$root = Channel();
		$root.chat = Chat("IRC");
		$root.name = "IRC";
		%servers = list();
		%channels = list($root);
		if (size($)) {
			for ($i=0; $i<size($); $i++) {
				$s = $[$i];
				%Server($s[0], $s[1]);
				%lastNick = $s[1];
				$server = %servers[size(%servers)-1];
				for ($j=2; $j<size($s); $j++) {
					$channel = %AddChannel($server, $s[$j]);
					$channel.joined = CHANNEL_NOT_CONNECTED;
				}
			}
		}
		else {
			%servers[0] = %channels[0] = IRCServer();
		}
		CreateTimer("CheckConnections", 40, 0, $this);
	}

	function Server($dns, $nick) {
		$server = IRCServer($dns, $nick);
		%channels[size(%channels)] = %servers[size(%servers)] = $server;
		SpawnThread("Connect", $this, list($server));
		for ($i=0; $i<size(%channels); $i++) {
			if (%channels[$i].visible) {
				%channels[$i].visible = $server;
				break;
			}
		}
	}

	function %Connect($server, $DNS) {
		//$ip = GetLocalIP();
		$ip = "127.0.0.1";
		if (IsNull($DNS)) $DNS = $server.DNS;
		$port = 6667;
		if ($server.status != IRC_DISCONNECTED) {
			if ($DNS ==s $server.DNS || $server.status == IRC_CONNECTING) return;
		}
		if ($server.retryTimer > 0) {
			KillTimer($server.retryTimer);
			$server.retryTimer = -1;
		}
		$server.socket = null;
		%Echo($server, "Connecting...");
		$server.DNS = $DNS;

		$temp = RegExp($DNS, "^([^:]*):(#*)$");
		if (!IsNull($temp)) {
			$DNS = $temp[0][0];
			$port = $temp[1][0];
		}

		$server.status == IRC_CONNECTING;
		$IPAddr = IPAddrWait($DNS);
		if (IsNull($IPAddr)) {
			$error = "Couldn't find DNS";
		}
		else {
			$server.socket = ConnectWait($IPAddr[0], $port, IPPROTO_TCP, "HandleServerSocketMessage", $this);
			if (IsNull($server.socket)) {
				$error = "Can't connect";
			}
			else {
				$server.status = IRC_HANDSHAKING;
				%Send($server, "NICK", $server.nick);
				%Send($server, "USER", $server.nick, $ip, $DNS, $server.nick);
				return;
			}
		}
		$server.status = IRC_DISCONNECTED;
		%Echo($server, $error);
		%Echo($server, "Retrying in 60");
		$server.retryTimer = CreateTimer("Retry", 60, 0, $this);
	}

	function CheckConnections() {
		$t = Time();
		for ($i = size(%servers)-1; $i>=0; $i--) {
			if ($t - %servers[$i].lastMessageTime >= 180) {
				if (%servers[$i].status == IRC_CONNECTED) {
					%servers[$i].lastMessageTime = $t;
					%Send(%servers[$i], "PING", "check");
				}
				// No clue what to do otherwise..Just wait, I guess.
			}
		}
	}

	function Retry($id) {
		KillTimer($id);
		for ($i = size(%servers)-1; $i>=0; $i--) {
			if (%servers[$i].retryTimer == $id) {
				%servers[$i].retryTimer = -1;
				%Connect(%servers[$i]);
				break;
			}
		}
	}

	function Send($server) {
		if ($server.status == IRC_DISCONNECTED || $server.status == IRC_CONNECTING) {
			%EchoBest($server, 1, , , "Not connected");
			return;
		}
		$text = ToUpper($[1]);
		$i = 2;
		while ($i < size($)-1) {
			$text +=s " " +s $[$i];
			$i++;
		}
		if ($i == size($) - 1)
			$text +=s " :" +s $[$i];
		$server.socket.Send($text +s "|r|n");
		$server.lastMessageTime = Time();
	}

	// Doesn't add the colon to the last argument.
	function %SendMode($server) {
		$text = ToUpper($[1]);
		$i = 2;
		while ($i < size($)) {
			$text +=s " " +s $[$i];
			$i++;
		}
		$server.socket.Send($text +s "|r|n");
		$server.lastMessageTime = Time();
	}

	function AddrSplit($addr) {
		$temp = RegExp($addr, "^(.*?)!(.*?)@(.*)$");
		if (!IsNull($temp)) {
			return list($temp[0][0], $temp[1][0], $temp[2][0]);
		}
		else {
			return list($addr);
		}
	}

	function HandleServerSocketMessage($reserved, $error, $msg, $socket) {
		while (1) {
			if ($i == size(%servers)) return;
			if (Equals($socket, %servers[$i].socket)) {
				$server = %servers[$i];
				break;
			}
			$i++;
		}
		if ($msg != FD_READ) {

			// Should always be true, but just in case...
			if ($server.status != IRC_CONNECTING) {
				$server.socket = null;
				for ($i=0; $i<size(%channels); $i++) {
					if (Equals(%channels[$i].server, $server)) {
						$chan = %channels[$i];
						%LeaveChannel(%channels[$i], "Disconnected");
					}
				}
				if (!size($error))
					$server.AddText("Disconnected");
				else
					$server.AddText("Disconnected (" +s $error +s ")");

				// Don't reconnect on voluntary disconnect.
				if ($server.status != IRC_DISCONNECTED) {
					$server.status = IRC_DISCONNECTED;
					%Connect($server);
				}
			}
		}
		else {
			$buffer = $server.buffer +s $socket.Read();
			$server.lastMessageTime = Time();
			while (1) {
				$temp = RegExp($buffer, "^([^\n]*)(\n\n?)");
				if (IsNull($temp)) {
					$server.buffer = $buffer;
					return;
				}
				$buffer = substring($buffer, $temp[1][1] + size($temp[1][0]), -1);
				$text = $temp[0][0];
				$from = null;
				$temp  = RegExp($text, "^:([^\w]*)\w");
				if (!IsNull($temp)) {
					$from = $temp[0][0];
					$text = substring($text, 2+size($from), -1);
				}
				$temp = RegExp($text, "\w:(.*)$");
				$last = null;
				if (!IsNull($temp)) {
					$last = $temp[0][0];
					$text = substring($text, 0, $temp[0][1]-2);
				}
				// Allowed tabs earlier, but tabs technically not allowed.
				$vals = strsplit($text, " ");
				if (IsString($last)) {
					$vals[size($vals)] = $last;
				}
				$command = pop($vals,0);

				if (!IsNull(RegExp($command, "^#+$"))) {
					%RAW($server, $from, $command|0, @$vals);
				}
				else {
					$this.call($command, list($server, %AddrSplit($from), @$vals));
				}
			}
		}
	}

	function FindChannel($server, $name) {
		for ($i=0; $i<size(%channels); $i++) {
			if (%channels[$i].name ==s $name && Equals(%channels[$i].server, $server)) return %channels[$i];
		}
	}

	function AddChannel($server, $name) {
		$out = %FindChannel($server, $name);
		if (!IsNull($out)) return $out;
		for ($i=0; $i<size(%channels); $i++) {
			if (Equals(%channels[$i], $server)) break;
		}
		for ($j=$i+1; $j < size(%channels); $j++) {
			if (%channels[$j].indent < 2) break;
			if (%channels[$j].name >s $name) break;
		}
		insert(%channels, $j, IRCChannel($server, $name, $nick));
		return (%channels[$j]);
	}

	function EchoBest($server, $useActive, $from, $to, $msg) {
		if (size($to)) {
			if ($to !=s $nick) {
				for ($i=0; $i<size(%channels); $i++) {
					if (%channels[$i].name ==s $to && Equals(%channels[$i].server, $server)) {
						%Echo(%channels[$i], $msg);
						$found = 1;
					}
				}
			}
			if ($found) return 3;
		}
		if (size($from)) {
			for ($i=0; $i<size(%channels); $i++) {
				if (%channels[$i].name ==s $from && Equals(%channels[$i].server, $server)) {
					%Echo(%channels[$i], $msg);
					$found = 1;
				}
			}
			if ($found) return 2;
		}
		if ($useActive) {
			for ($i=0; $i<size(%channels); $i++) {
				if (%channels[$i].visible) {
					if (Equals(%channels[$i].server, $server)) {
						%Echo(%channels[$i], $msg);
						return 1;
					}
					break;
				}
			}
		}
		%Echo($server, $msg);
		return 0;
	}

	function NOTICE($server, $from, $to, $msg) {
		if (size($from[0])) {
			$text = "-" +s $from[0] +s "- " +s $msg;
			$res = %EchoBest($server, , , $to, $text);
			if (!$res) {
				for ($i=0; $i<size(%channels); $i++) {
					$channel = %channels[$i];
					if (Equals($channel.server, $server)) {
						$channel.AddText($text);
					}
				}
			}
		}
		else {
			%Echo($server, "-from server- " +s $msg);
		}
	}

	function TOPIC($server, $by, $channel, $text) {
		%EchoBest($server, , $channel, , $by[0] +s " sets topic to |"" +s $text +s "|"");
	}

	function INVITE($server, $by, $nick, $channel) {
		%EchoBest($server, 1, , , "! " +s $by[0] +s " is inviting you to " +s $channel);
	}

	function LeaveChannel($channel, $message) {
		if ($channel.joined != CHANNEL_NOT_CONNECTED) {
			$channel.joined = CHANNEL_NOT_CONNECTED;
			$channel.AddText($message);
			if ($channel.server.status == IRC_CONNECTED) {
				if ($channel.channel) {
					%ExecCommand($channel.server, "JOIN", $channel.name);
					$channel.AddText("Rejoining...");
				}
			}
		}
	}

	function PART($server, $person, $channel, $text) {
		if (size($text)) $text = " (" +s $text +s ")";
		$chan = %FindChannel($server, $channel);
		if ($person[0] ==s $server.nick) {
			$msg = "You part " +s $text;
			%LeaveChannel($chan, $msg);
		}
		else {
			$msg = $person[0] +s " parts " +s $text;
			$chan.chat.RemoveName($person[0]);
			$chan.AddText($msg);
		}
	}


	function KICK($server, $person, $channel, $target, $text) {
		$msg = $person[0] +s " has kicked " +s $target;
		if (size($text)) $msg += " (" +s $text +s ")";
		$chan = %FindChannel($server, $channel);
		if ($target ==s $server.nick) {
			%LeaveChannel($chan, $msg);
		}
		else {
			$chan.chat.RemoveName($target);
			$chan.AddText($msg);
		}
	}

	function MODE($server, $person, $channel) {
		$chan = %FindChannel($server, $channel);
		$text = $person[0] +s " has set mode: " +s $[3];
		for ($i=4; $i<size($); $i++) {
			$text +=s " " +s $[$i];
		}
		$chan.AddText($text);
		$params = $[3];
		$pos = 4;
		for ($i=0;$i < size($params);$i++) {
			$c = $params[$i];
			$powerChange = 0;
			if ($c ==S "+") $sub = 0;
			else if ($c ==S "-") $sub = 1;
			else if ($c ==S "v") {
				$powerChange = CHANNEL_VOICED;
			}
			else if ($c ==S "o") {
				$powerChange = CHANNEL_OPERATOR;
			}
			else if ($c ==S "h") {
				$powerChange = CHANNEL_HALF_OP;
			}
			else if ($c ==S "k") {
				$pos++;
			}
			else if ($c ==S "l") {
				$pos++;
			}
			else if ($c ==S "b") {
				$pos++;
			}
			else if ($c ==S "e") {
				$pos++;
			}
			else if ($c ==S "I") {
				$pos++;
			}
			if ($powerChange) {
				$user = $chan.chat.FindName($[$pos]);
				if ($sub) $user[3] &= ~$powerChange;
				else $user[3] |= $powerChange;
				$pos ++;
			}
		}
	}

	function QUIT($server, $person, $text) {
		$msg = $person[0] +s " quits";
		if (size($text)) {
			$msg +=s " (" +s $text +s ")";
		}
		for ($i=0; $i<size(%channels); $i++) {
			if (Equals(%channels[$i].server, $server)) {
				$channel = %channels[$i];
				$user = $channel.chat.RemoveName($person[0]);
				if (!IsNull($user)) {
					$channel.AddText($msg);
				}
			}
		}
	}

	function NICK($server, $person, $name) {
		if ($person[0] !=s $server.nick) {
			$msg = $person[0] +s " is now known as " +s $name;
		}
		else {
			%SetNick($server, $name);
			$msg = "You are now known as " +s $name;
			$server.AddText($msg);
			%lastNick = $name;
		}
		for ($i=0; $i<size(%channels); $i++) {
			if (Equals(%channels[$i].server, $server)) {
				$channel = %channels[$i];
				$user = $channel.chat.FindName($person[0]);
				if (!IsNull($user)) {
					$user[0] = $name;
					$user[1] = $person[1];
					$user[2] = $person[2];
					$channel.AddText($msg);
					$channel.chat.SortNames();
				}
			}
		}
	}

	function PRIVMSG($server, $from, $to, $text) {
		$CTCP = RegExp($text, "^|1(.*?)|1?$")[0][0];
		if ($to[0] ==S "#" || $to[0] ==S "&") {
			$channel = %AddChannel($server, $to);
		}
		if (IsNull($CTCP)) {
			$name = $from[0];
			if (!IsNull($channel)) {
				$user = $channel.chat.FindName($name);
				$power = $user[3];
				if ($power & CHANNEL_OPERATOR) {
					$name = "@" +s $name;
				}
				else if ($power & CHANNEL_HALF_OP) {
					$name = "%" +s $name;
				}
				else if ($power & CHANNEL_VOICED) {
					$name = "+" +s $name;
				}
			}
			$text = "<" +s $name +s "> "  +s $text;
		}
		else {
			$temp = RegExp($CTCP, "^\w*([^\w]+)\w*(.*)$");
			if (IsNull($temp)) return;
			$command = $temp[0][0];
			$args = $temp[1][0];
			if ($command ==s "ACTION") {
				if (substring($args, 0, 3) ==s "'s ") {
					$text = "* " +s $from[0] +s $args;
				}
				else
					$text = "* " +s $from[0] +s " " +s $args;
			}
			else {
				$text = "[" +s $from[0] +s "] CTCP " +s $command;
				if (size($args)) {
					$text +=s ": " +s $args;
				}
				// Currently ignore pings and versions, and no CTCP chat and DCC support.
				$server.AddText($text);
				return;
			}
		}
		if (!IsNull($channel)) {
			$channel.AddText($text);
		}
		else {
			$temp = $from[0];
			if ($temp ==s $server.nick) {
				$temp = $to;
			}
			$channel = %AddChannel($server, $temp);
			$channel.AddText($text);
			$channel.chat.ClearNames();
			$channel.chat.AddName($from);
			$channel.chat.AddName(list($server.nick));
		}
	}

	function JOIN($server, $from, $name) {
		if ($from[0] ==s $server.nick) {
			$channel = %FindChannel($server, $name);
			if (IsNull($channel)) {
				$channel = %AddChannel($server, $name);
				for ($i=0; $i<size(%channels); $i++) {
					if (%channels[$i].visible) {
						%channels[$i].visible = $channel;
						break;
					}
				}
			}
			$channel.joined = CHANNEL_JOINING;
			$channel.chat.ClearNames();
		}
		else {
			$channel = %FindChannel($server, $name);
			$channel.chat.AddName($from);
		}
		$channel.AddText("** " +s $from[0] +s " has joined " +s $name);
	}

	// Only useful for server objects.  Doesn't affect lists in Chat objects.
	function SetNick($server, $nick) {
		$server.SetNick($nick);
		for ($i=0; $i<size(%channels); $i++) {
			if (Equals(%channels[$i].server, $server))
				%channels[$i].SetNick($nick);
		}
	}

	function RAW($server, $from, $raw) {
		pop_range($, 0, 2);
		$merged = $[1];
		for ($i=2; $i<size($); $i++) {
			$merged +=s " " +s $[$i];
		}
		//if (size($merged)) {
			if (($raw < 200) || ($raw >= 251 && $raw <= 255) || $raw == 422) {
				if ($raw == 1) {
					if (size($[0])) {
						%SetNick($server, $[0]);
						$server.status = IRC_CONNECTED;
						%EchoRaw($server, $merged);
						for ($i=size(%channels)-1; $i>=0; $i--) {
							$channel = %channels[$i];
							if (Equals($channel.server, $server) && $channel.channel) {
								%ExecCommand($server, "JOIN", $channel.name);
								$channel.AddText("Joining...");
							}
						}
						return;
					}
				}
				%EchoRaw($server, $merged);
				return;
			}
			if ($raw == 353) {
				$channel = %FindChannel($server, $[2]);
				if ($channel.joined == CHANNEL_JOINING) {
					$n = strsplit($[3], " ");
					for ($i=0; $i<size($n); $i++) {
						$temp = RegExp($n[$i], "^([%+@]?)(.*)");
						if ($temp[0][0] ==s "@") $power = CHANNEL_OPERATOR;
						else if ($temp[0][0] ==s "+") $power = CHANNEL_VOICED;
						else if ($temp[0][0] ==s "%") $power = CHANNEL_HALF_OP;
						$channel.chat.AddNameUnsorted(list($temp[1][0], 0, 0, $power));
					}
					return;
				}
			}
			else if ($raw == 366) {
				$channel = %FindChannel($server, $[1]);
				if ($channel.joined == CHANNEL_JOINING) {
					$channel.chat.SortNames();
					$channel.joined = CHANNEL_JOINED;
					return;
				}
			}

			// whois/whowas/who
			if (($raw >= 311 && $raw <= 319) || $raw == 369 || $raw == 352) {
				$msg = $merged;
				if ($raw == 311 || $raw == 314) {
					$msg = "Whois";
					if ($raw == 314)
						$msg = "Whowas";
					$msg +=s " reply for " +s $[5] +s "(" +s $[2] +s "@" +s $[3] +s ")";
				}
				else if ($raw == 312) {
					$msg = "Server: " +s $[2] +s " (" +s $[3] +s ")";
				}
				else if ($raw == 313) {
					$msg = $[1] +s " " +s $[2];
				}
				else if ($raw == 317) {
					$msg = "Idle time: " +s FormatDuration($[2]);
					if (IsString($[4])) {
						%EchoBest($server, 1, , , "? " +s $msg);
						$msg = "On since: " +s FormatTime("ddd M-DD-YY, hh:NN:SS tt", $[3]);
					}
				}
				else if ($raw == 319) {
					$msg = "Channels: " +s $msg;
				}
				else if ($raw == 318) {
					$msg = "End of whois for " +s $[1];
				}
				else if ($raw == 369) {
					$msg = "End of whowas for " +s $[1];
				}
				else if ($raw == 315) {
					$msg = "End of who for " +s $[1];
				}
				else if ($raw == 352) {
					$msg = "Who: " +s $[5] +s "!" +s $[2] +s "@" +s $[3] +s " is on " +s $[4] +s " (" +s $[1] +s " " +s $[6] +s " " +s $[7] +s ")";
				}
				%EchoBest($server, 1, , , "? " +s $msg);
				return;
			}

			// Channel info
			if ($raw >= 324  && $raw <= 333) {
				$msg = "";
				$start = "? " +s $[1];
				if ($raw == 324) {
					$msg = $start +s " modes: " +s substring($merged, size($[1])+1, -1);
				}
				if ($raw == 328) {
					$msg = $start +s " url: " +s $[2];
				}
				if ($raw == 329) {
					$msg = $start +s " creation time: " +s FormatTime("ddd M-DD-YY, hh:NN:SS tt", $[2]);
				}
				if ($raw == 331) {
					$msg = $start +s " has no topic";
				}
				if ($raw == 332) {
					$msg = $start +s " topic: " +s $[2];
				}
				if ($raw == 333) {
					$msg = "? Set by " +s $[2] +s " on " +s FormatTime("ddd M-DD-YY, hh:NN:SS tt", $[3]);
				}
				if (size($msg)) {
					%EchoBest($server, 1, $[1], , $msg);
					return;
				}
			}

			if ($raw >= 471  && $raw <= 475 && $raw != 472) {
				%EchoBest($server, 1, $[1], , "! " +s strreplace($[2], "channel", $[1]));
				return;
			}

			if ($raw == 221) {
				if (!size($[1])) %EchoBest($server, 1, , , "No modes set for " +s $[0]);
				else %EchoBest($server, 1, , , "Modes for " +s $[0] +s ": " +s $[1]);
				return;
			}
			%EchoBest($server, 1, , , "! " +s $merged);
		//}
	}

	function PING($server, $from, $param) {
		%Send($server, "PONG", $param);
	}

	function Echo($channel, $text) {
		$channel.AddText($text);
	}

	function EchoRaw($channel, $text) {
		%Echo($channel, "! " +s $text);
	}

	function KeyDown($channel, $event, $param, $modifiers, $vk, $string) {
		if ($vk != VK_RETURN) {
			if ($modifiers & 2) {
				if ($vk == VK_B) {
					$modifiers = 0;
					$string = "|2";
				}
				else if ($vk == VK_W) {
					// Root window.
					if (Equals($channel, %channels[0])) return 1;

					NeedRedraw();
					if (!IsNull($channel.server)) {
						if ($channel.joined != CHANNEL_NOT_CONNECTED) {
							%Send($channel.server, "PART", $channel.name);
							$channel.joined = CHANNEL_NOT_CONNECTED;
						}
					}
					else {
						%Send($channel, "QUIT");
					}
					for (; $i<size(%channels); $i++) {
						if (Equals(%channels[$i], $channel) || Equals(%channels[$i].server, $channel)) {
							pop(%channels, $i);
							$i--;
						}
					}
					return 1;
				}
			}
			return $channel.chat.KeyDown($event, $param, $modifiers, $vk, $string);
		}

		$text = $channel.chat.editor.GetText();
		if (!size($text)) return 1;

		$channel.chat.ClearEditor();
		NeedRedraw();
		$command = RegExp($text, "^\w*\\([^\w]*)\w*(.*)");
		$t = "\"; //"
		if (IsNull($command)) {
			$command = RegExp($text, "^\w*/([^\w]*)\w*(.*)");
			$t = "/";
		}
		if (!IsNull($command)) {
			if ($command[0][0] ==s "SAY") {
				$text = $command[1][0];
			}
			else if ($command[0][0] ==s "ME") {
				$text = "|1ACTION " +s $command[1][0] +s "|1";
			}
			else {
				if ($command[0][0][0] !=S $t) {
					%ExecCommand($channel, $command[0][0], $command[1][0]);
					return 1;
				}
				$text = strreplace($text, $t, "", 1);
			}
		}
		if (type($channel) ==S "IRCServer" || type($channel) ==S "Channel") {
			%Echo($channel, "Note to self: " +s $text);
		}
		else {
			if ($channel.joined == CHANNEL_JOINED) {
				%ExecCommand($channel, "PRIVMSG", $channel.name +s " " +s $text);
			}
			else {
				$channel.AddText("Not on channel");
			}
		}
		return 1;
	}

	function ExecCommand($channel, $command, $args) {
		$server = $channel;
		if ($channel.chat.ExecCommand($command, $args)) return;
		if ($command ==s "SERVER") {
			$temp = RegExp($args, "^([^\w]+)\w*(.*)$");
			if (IsNull($temp)) {
				$channel.AddText("! SERVER: Not enough parameters");
			}
			else {
				$DNS = $temp[0][0];
				$nick = $temp[1][0];
				$i = size(%channels);
				if (!size($nick)) {
					$nick = %lastNick;
					while (!size($nick) && $i) {
						$nick = %channels[--$i].nick;
					}
					if (!size($nick))
						$nick = "Goat" +s rand(0, 1000000);
				}
				%Server($DNS, $nick);
			}
			return;
		}
		if (type($channel) ==S "Channel") {
			$channel.AddText("! Root window.  Only /server works");
			return;
		}
		if (type($server) !=S "IRCServer") {
			$server = $channel.server;
			$IsChannel = 1;
		}
		if ($command ==s "CONNECT") {
			if ($server.status == IRC_DISCONNECTED) {
				%Connect($server);
			}
			else if ($server.status == IRC_CONNECED) {
				$channel.AddText("Already connected");
			}
			else {
				$channel.AddText("Already trying to connect");
			}
			return;
		}
		if ($command ==s "MSG") $command = "PRIVMSG";
		// 2 parameter commands, including those whose second parameter is optional.
		// If it's not there, will fall through to the default 0-1 parameter code.
		if ($command ==s "PRIVMSG" ||
			$command ==s "NOTICE" ||
			$command ==s "TOPIC" ||
			$command ==s "INVITE" ||
			$command ==s "JOIN" ||
			$command ==s "PART") {

				$temp = RegExp($args, "^([^\w]+)\w+([^\w].+)$");
				if (!IsNull($temp)) {
					if ($command ==s "PRIVMSG") {
						%PRIVMSG($server, list($server.nick), $temp[0][0], $temp[1][0]);
					}
					%Send($server, $command, $temp[0][0], $temp[1][0]);
					return;
				}
		}
		if ($command ==s "DISCONNECT") {
			$command = "QUIT";
		}
		if ($command ==s "MODE") {
			%SendMode($server, $command, $args);
		}
		else {
			%Send($server, $command, $args);
		}
		if ($command ==s "QUIT") {
			$server.status = IRC_DISCONNECTED;
		}
	}
};

