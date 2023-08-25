// Very slow.  May make a non-scripted equivalent, at some point, but
// having it here allows customization.  May add support for <pre> at
// some point.

function StripHTML($html) {
	$pos = 0;
	while ($pos < size($html)) {
		$pos = strstr($html, "<", $pos);
		if (IsNull($pos)) break;
		$e = $pos;
		while (1) {
			$e = RegExp($html, "([>'|"])", $e); //" Needed to get coloring right with editor.
			if (IsNull($e)) break;
			if ($e[0][0] ==s ">") break;
			else {
				$e = RegExp($html, "(" +s $e[0][0] +s ")", $e[0][1]+1);
				if (IsNull($e)) break;
				$e = $e[0][1]+1;
			}
		}
		if (IsNull($e)) break;
		$e = $e[0][1];
		$sub = "";
		$substr = substring($html, $pos, $e+1);
		$incr = 0;
		if (!IsNull(RegExp($substr, "^<[bB][rR][ |t|r|n>/]"))) {
			$sub = "<BR>";
		}
		else if (!IsNull(RegExp($substr, "^<[pP][ |t|r|n>/]"))) {
			$sub = "<BR><BR>";
		}
		else if (!IsNull(RegExp($substr, "^<|/?[tT][aA][bB][lL][eE][ |t|r|n>/]"))) {
			$sub = "<BR>";
		}
		else if (!IsNull(RegExp($substr, "^<|/[tT][rR][ |t|r|n>/]"))) {
			$sub = "<BR>";
		}
		else if (!IsNull(RegExp($substr, "^<|/[tT][dD][ |t|r|n>/]"))) {
			$sub = " ";
		}
		$html = strswap($html, $sub, $pos, $e+1)[0];
		$pos += size($sub);
	}
	$html = strreplace($html, "|r", "|n");
	$html = strreplace($html, "|n", " ");
	$html = strreplace($html, "|t", " ");
	$s = 0;
	while ($s != size($html)) {
		$s = size($html);
		$html = strreplace($html, "  ", " ");
	}
	$s = 0;
	$html = strreplace($html, "&nbsp;", " ");
	$html = strreplace($html, "<BR>", "|n");
	while ($s != size($html)) {
		$s = size($html);
		$html = strreplace($html, "|n |n", "|n|n");
		$html = strreplace($html, "|n|n|n", "|n|n");
	}
	$substr = RegExp($html, "^([ |t|r|n]*)");
	$pre = size($substr[0][0]);
	$substr = RegExp($html, "([ |t|r|n]*)$");
	$post = size($substr[0][0]);
	// Handles escape codes other than "&nbsp;", which doesn't
	// exist in XML.
	
	return ParseXML(substring($html, $pre, size($html)-$post))[1];
	
	// Remove funky punctuation that my favorite LCD fonts lack.
	/*$html = ParseXML(substring($html, $pre, size($html)-$post))[1];
	$html = strreplace($html, "’", "'");
	$html = strreplace($html, "“", "|""); //"
	$html = strreplace($html, "”", "|""); //"
	return $html;*/
}

