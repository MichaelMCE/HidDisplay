
function SimpleXML (xml, path) {
	while ($index < size($path)) {
		$index2 = FindNextXML($xml, $path[$index]);
		if ($index2 < 0) return;
		$index++;
		$xml = $xml[$index2 + 1];
	}
	return $xml;
}

