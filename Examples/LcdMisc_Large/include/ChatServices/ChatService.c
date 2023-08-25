#requires <Modules\Chat.c>

// Doesn't extend chat so I can easily nuke chat object for inactive connections, clearing history, etc.
struct Channel {
	// Visible many not really mean visible, when the chat screen isn't up front.  Used for
	// prioritizing messages that could be displayed in multiple screens, but don't need to be
	// displayed on all of them (like IRC /whois replies).
	var %chat, %indent, %name, %visible;

	function AddText($text) {
		%chat.AddText($text);
		if (%visible) NeedRedraw();
	}

	function Channel() {
	}
};

struct ChatService {
	var %channels;
	function ChatService() {
	}
};

