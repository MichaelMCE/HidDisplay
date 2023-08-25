
struct Graph {
	var %data, %type, %points, %lastUpdate, %min, %max, %sides;

	function Graph($_type, $_min, $_max, $_points, $_sides) {
		%type = $_type;
		%points = $_points;
		if (!$_points) $_points = DHEIGHT;
		%data = list();
		%data[$_points-1] = 0;
		%min = $_min;
		%max = $_max;
		%sides = $_sides;

	}

	function Update($val) {
		pop(%data, 0);
		if ($val < %min) $val = %min;
		if ($val > %max) $val = %max;
		push_back(%data, $val);
	}

	function Draw($x1, $y1, $x2, $y2) {
		$dx = ($x1 - $x2) / abs($x1 - $x2);

		if (%sides & 2) {
			DrawRect($x1, $y2, $x2, $y2);
			$y2++;
		}
		if (%sides & 1) {
			DrawRect($x1, $y1, $x1, $y2);
			DrawRect($x2, $y1, $x2, $y2);
			$x1 -= $dx;
			$x2 += $dx;
		}

		$range = ($y2-$y1)*1.0/(%max-%min);

		// For rounding.  Rounding is done by removing decimal, I believe.
		$y15 = $y1 + 0.5;

		$x2 -= $dx;

		$v = %points;
		while ($x1 != $x2) {
			$v--;
			$x2 += $dx;
			$p = (%data[$v]-%min) * $range + $y15;
			DrawRect($x2, $p, $x2, $y1);
		}
	}
};

function DrawMultiGraph ($graphs, $x1, $y1, $x2, $y2, $bgColor, $padding)
{
	$colors = list(
			RGB(0,255,0),     RGB(255,0,0),   RGB(0,255,255),	RGB(255,255,0), RGB(255,0,255), RGB(255,255,255), RGB(244,100,50),	RGB(0,0,255), 
			RGB(127,127,127), RGB(127,127,0), RGB(0,127,127),	RGB(127,0,127), RGB(127,0,0),   RGB(0,0,127),     RGB(0,127,0),   	RGB(32,127,255),
			
			RGB(0,255,127),   RGB(255,0,127), RGB(127,255,255),	RGB(255,255,0), RGB(255,64,200),RGB(64,255,64),  RGB(50,100,224), RGB(255,100,64), 
			RGB(127,255,127), RGB(127,255,0), RGB(0,255,127),	RGB(127,0,127), RGB(127,0,0),   RGB(0,0,127),    RGB(0,127,0),   	RGB(32,127,255)
	);
	
		
	/*$colors = list(
	  0xff00ff00,
	  0xffffff00,
	  0xFFffffff,
	  0xFF1d2b53,
	  0xFF7e2553,
	  0xFF008751,
	  0xFFab5236,
	  0xFF5f574f,
	  0xFFc2c3c7,
	  0xFFfff1e8,
	  0xFFff004d,
	  0xFFffa300,
	  0xFFffec27,
	  0xFF00e436,
	  0xFF29adff,
	  0xFF83769c,
	  0xFFff77a8,
	  0xFFffccaa,
	  0xFF291814,
	  0xFF111d35,
	  0xFF422136,
	  0xFF125359,
	  0xFF742f29,
	  0xFF49333b,
	  0xFFa28879,
	  0xFFf3ef7d,
	  0xFFbe1250,
	  0xFFff6c24,
	  0xFFa8e72e,
	  0xFF00b543,
	  0xFF065ab5,
	  0xFF754665,
	  0xFFff6e59,
	  0xFFff9d81
	);*/
				
			
	$dx = ($x1 - $x2) / abs($x1 - $x2);
	
	if ($bgColor){
		//$bgColor = RGB(0,0,96);
		ColorRect($x1, $y1, $x2, $y2, $bgColor);
	}

	// For rounding.  Rounding is done by removing decimal, I believe.
	$y15 = $y1 + 0.5;

	if ($y1 < $y2) {
		$y1 += $padding[1];
		$y2 -= $padding[3];
	}
	else {
		$y2 += $padding[1];
		$y1 -= $padding[3];
	}
	if ($dx < 0) {
		$x1 += $padding[0];
		$x2 -= $padding[2];
	}
	else {
		$x2 += $padding[0];
		$x1 -= $padding[2];
	}


	$origDrawColor = SetDrawColor();

	
	for ($i=0; $i<size($graphs); $i++) {
		//WriteLogLn($i +s " - " +s FormatValue($i%size($colors)) ,1);
		SetDrawColor($colors[ $i%size($colors) ]);
		
		$graph = $graphs[$i];
		$min = $graph.min;
		$max = $graph.max;
		$v = $graph.points-1;
		$data = $graph.data;
		$range = ($y2-$y1)*1.0/($max-$min);

		$p = ($data[$v]-$min) * $range + $y15;
		$x2old = $x2;

		while ($x1 != $x2) {
			$v--;
			DrawLine($x2, $p, $x2 += $dx, $p = ($data[$v]-$min) * $range + $y15);
		}

		$x2 = $x2old;
	}
	SetDrawColor($origDrawColor);
};


function DrawGraph ($graph, $x1, $y1, $x2, $y2, $bgColor, $color, $padding) {
	
	$dx = ($x1 - $x2) / abs($x1 - $x2);
	if ($bgColor){
		//$bgColor = RGB(0,0,96);
		ColorRect($x1, $y1, $x2, $y2, $bgColor);
	}

	// For rounding.  Rounding is done by removing decimal, I believe.
	$y15 = $y1 + 0.5;

	if ($y1 < $y2) {
		$y1 += $padding[1];
		$y2 -= $padding[3];
	}
	else {
		$y2 += $padding[1];
		$y1 -= $padding[3];
	}
	if ($dx < 0) {
		$x1 += $padding[0];
		$x2 -= $padding[2];
	}
	else {
		$x2 += $padding[0];
		$x1 -= $padding[2];
	}

	$origDrawColor = SetDrawColor();

		SetDrawColor($color);
		$min = $graph.min;
		$max = $graph.max;
		$v = $graph.points-1;
		$data = $graph.data;
		$range = ($y2-$y1)*1.0/($max-$min);

		$p = ($data[$v]-$min) * $range + $y15;
		$x2old = $x2;

		while ($x1 != $x2) {
			$v--;
			DrawLine($x2, $p, $x2 += $dx, $p = ($data[$v]-$min) * $range + $y15);
		}

		$x2 = $x2old;

	SetDrawColor($origDrawColor);
};


function DrawBarGraph ($graph, $x1, $y1, $x2, $y2, $bgColor, $color, $padding) {
	
	$dx = ($x1 - $x2) / abs($x1 - $x2);
	if ($bgColor){
		//$bgColor = RGB(0,0,96);
		ColorRect($x1, $y1, $x2, $y2, $bgColor);
	}

	// For rounding.  Rounding is done by removing decimal, I believe.
	$y15 = $y1 + 0.5;

	if ($y1 < $y2) {
		$y1 += $padding[1];
		$y2 -= $padding[3];
	}
	else {
		$y2 += $padding[1];
		$y1 -= $padding[3];
	}
	if ($dx < 0) {
		$x1 += $padding[0];
		$x2 -= $padding[2];
	}
	else {
		$x2 += $padding[0];
		$x1 -= $padding[2];
	}
	
	$x2 -= $barWidth;
	//$dx -= $barWidth;

	$origDrawColor = SetDrawColor();

		SetDrawColor($color);
		$min = $graph.min;
		$max = $graph.max;
		$v = $graph.points-1;
		$data = $graph.data;
		$range = ($y2-$y1)*1.0/($max-$min);

		$p = ($data[$v]-$min) * $range + $y15;
		$x2old = $x2;

		//$v--;
		$p = ($data[$v]-$min) * $range + $y15;

		ColorRect($x1-$dx, $y1, $x2+$dx, $p, $color);

		$x2 = $x2old;

	SetDrawColor($origDrawColor);
};

