// This file should probably be in util, but still here for legacy reasons.

/* Takes a list of lists to use as argument to $fxn, returns list of results. */
function Map($fxn, $lst) {
	$out = list();
	$out[size($lst)-1] = 0;
	for (;$i<size($lst); $i++) {
		$out[$i] = call($fxn, $lst[$i]);
	}
	return $out;
}

/* Simpler than map - takes in a list of values for a single argument function */
function MapSingle($fxn, $lst) {
	$out = list();
	$out[size($lst)-1] = 0;
	for (;$i<size($lst); $i++) {
		$out[$i] = call($fxn, list($lst[$i]));
	}
	return $out;
}

function RemoveNulls($lst) {
	for ($i = size($lst)-1; $i>=0; $i--) {
		if(IsNull($lst[$i])) {
			pop($lst, $i);
		}
	}
	return $lst;
}

// Find the first element that Equals() the given value.  Returns -1 on fail.
function listFind($list, $val) {
	while ($i < size($list)) {
		if (Equals($list[$i], $val))
			return $i;
		else
			$i++;
	}
	return -1;
}

// Find the first element that == the given value.  Returns -1 on fail.
function listFindInt($list, $val) {
	while ($i < size($list)) {
		if ($list[$i] == $val)
			return $i;
		else
			$i++;
	}
	return -1;
}

// flips $list.
function listFlip($list) {
	$len = size($list)-1;
	for ($i = $len/2; $i>=0; $i--) {
		$temp = $list[$i];
		$list[$i] = $list[$len-$i];
		$list[$len-$i] = $temp;
	}
}

// Remove all objects from $list Equal() to $val.
function listRemove($list, $val) {
	while ($i < size($list)) {
		if (Equals($list[$i], $val))
			pop($list, $i);
		else
			$i++;
	}
}

function IndexedStringSort($list) {
	$out = list(0);
	for ($i = 1; $i<size($list); $i++) {
		for ($j = $i; $j > 0; ) {
			if ($list[$out[$j-1]] <=s $list[$i]) break;
			$out[$j] = $out[$j-1];
			$j--;
		}
		$out[$j] = $i;
	}
	return $out;
}

// Simple list wrapper class.
struct ObjectList {
	var objs;
	function ObjectList() {
		%objs = list();
	}

	function Insert($index/*, obj1, obj2, etc*/) {
		insert(%objs, @$);
	}

	function Remove(/*obj1, obj2, etc*/) {
		for ($i=0; $i<size($); $i++) {
			listRemove(%objs, $[$i]);
		}
	}
};


// Priority list.  Elements are of the form (object, priority).
// Low priority items first.  Still needs to be debugged, so
// don't use it, yet.
struct PriotityObjectList {
	var objs;
	function PriotityObjectList() {
		%objs = list();
	}

	function Insert($object, $priority) {
		$end = size(%objs);
		while ($i < $end) {
			if (%objs[$i][1] > $priority) {
				break;
			}
			$i++;
		}
		// Insert argument list ($object, $priority).
		insert(%objs, $i, $);
	}

	function ChangePriority($object, $priority) {
		%Remove($object);
		%Insert(@$);
	}

	function Remove(/*obj1, obj2, etc*/) {
		for ($i=0; $i<size($); $i++) {
			for ($j=0; $j<size(%objs);) {
				if (Equals(%objs[$j], $[$i])) {
					pop(%objs, $i);
				}
				else {
					$i++;
				}
			}
		}
	}
};

