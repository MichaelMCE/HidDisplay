// Can significantly reduce CPU usage.  Currently only works with single argument functions that take a string or integer as an argument.

function CachedCall ($fxn, $arg, $time) {
	$data = cachedValues[$fxn +s $arg];
	if (IsNull($data)) {
		if (IsNull(cachedValues)) {
			cachedValues = dict();
		}
		$data = list(0);
		cachedValues[$fxn +s $arg] = $data;
	}
	if ($data[0] + $time < Time()) {
		$data[0] = Time();
		$data[1] = call($fxn, list($arg));
	}
	return $data[1];
}

function SafeSmart($drive) {
	if (IsNull(SmartTests[$drive])) {
		if (IsNull(SmartTests)) SmartTests = dict();
		$res = GetDiskSmartInfo($drive);
		if (IsList($res) && size($res)) {
			SmartTests[$drive] = 1;
			return $res;
		}
		SmartTests[$drive] = 0;
		return null;
	}
	else if (SmartTests[$drive] == 0) return null;
	else return GetDiskSmartInfo($drive);
}

