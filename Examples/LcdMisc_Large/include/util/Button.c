

#requires <util\Graphics.c>

struct Button {
	var %x, %y;
	var %image;
	var %cmd, %args;
	var %width, %height;
	var %type;
	var %scale;
	var %aniTimerId;
	var %aniDir;
	var %label;
	
	function Button ($X, $Y, $filename, $CMD, $ARGS, $Label){
		%x = $X;
		%y = $Y;
		%cmd = $CMD;
		%args = $ARGS;
		%label = $Label;
		%image = LoadImage32($filename);
		%width = %image.Size()[0];
		%height = %image.Size()[1];
		%type = 0;	// 0 = executable, 1 = event, 2 = do nothing but animate, 3 = same as 1 without animation.
		%scale = 1.0;
		%aniTimerId = 0;
		%aniDir = 0;	// 0 = off, 1 = inwards, 2 = outwards
	}
	function drawBorder ($colour){
		DrawRectangle(%x-1, %y-1, %x+%width, %y+%height, $colour);
	}
	function draw (){
		if (%scale == 1.0 || %type == 2){
			DrawImage(%image, %x, %y);

			$h = %image.Size()[1];
			DisplayTextShadowed(%label, %x, %y+$h+1, 0xB328C672, RGBA(255,255,255,240));
			
		}else{
			$w = %image.Size()[0]+0.0;
			$h = %image.Size()[1]+0.0;
			DrawRotatedScaledImage(%image, %x+(($w/2)*(1.0-%scale)), %y+(($h/2)*(1.0-%scale)), 0, %scale, %scale);
		}
	}
	function isButtonTouched ($X, $Y){
		if ($X >= %x && $X <= %x+%width){
			if ($Y >= %y && $Y <= %y+%height){	
				return 1;
			}
		}
		return 0;
	}

	function doAnimate (){
		if (%scale < 0.1)
			%aniDir = 2;

		if (%aniDir == 1)
			%scale -= 0.1;
		else
			%scale += 0.1;
	
		if (%scale >= 1.0){
			KillTimer(%aniTimerId);
			%scale = 1.0;
			%aniTimerId = 0;
			%aniDir = 0;
		}
		NeedRedraw();
	}

	function startIconAnimation (){
		if (%aniTimerId == 0 && %type != 3){
			%scale = 1.0;
			%aniDir = 1;
			%aniTimerId = CreateFastTimer("doAnimate", 30, 1, $this);
		}
	}

	function run (){
		%startIconAnimation();
		//Wait(75);

		if (%type == 0){
			if (!Run(%cmd, %args, 2))
				RunSimple(%cmd, %args, 2);
			return 1;
		}else if (%type == 1 || %type == 3){
			$alist = list();
			$alist[0] = %args;
			PostEvent(%cmd, 1, $alist);
			return 1;
		}else if (%type == 2){	// do nothing
			return 0;
		}
	}
};


struct Icons extends Button {
	var %buttons, %tbuttons;
	
	function Icons (){
		%buttons = list();
		%tbuttons = 0;
	}
	
	function addIcon ($TYPE, $X, $Y, $filename, $CMD, $ARGS, $Label){
		%buttons[%tbuttons] = Button($X, $Y, IMAGE_PATH +s $filename, $CMD, $ARGS, $Label);
		%buttons[%tbuttons].type = $TYPE;
		%tbuttons++;
	}
	function drawIcon ($i){
		%buttons[$i].draw();
	}
	function drawIcons (){
		for ($i = 0; $i < size(%buttons); $i++){
			%buttons[$i].draw();
		}
	}
	function drawIconsWithBorder ($colour){
		for ($i = 0; $i < size(%buttons); $i++){
			%buttons[$i].draw();
			%buttons[$i].drawBorder($colour);
		}
	}	
	function runTouchedIcon ($X, $Y){
		$button = %isIconTouched($X, $Y);
		if (!IsNull($button)){
			return $button.run();
		}else{
			return 0;
		}
	}
	function isIconTouched ($X, $Y){
		for ($i = 0; $i < size(%buttons); $i++){
			if (%buttons[$i].isButtonTouched($X, $Y)){
				return %buttons[$i];
			}
		}
		return NULL;
	}
};

