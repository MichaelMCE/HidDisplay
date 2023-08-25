
function DisplayHeader ($res) {
	$width = $res[0];
	
	if (IsNull($font))
		$font = Font("76London", 80, 0,0,0, CLEARTYPE_QUALITY);
	
	//UseFont(bigHeaderFont);
	UseFont($font);
		
	//if (IsNull(titleBarImage))
	//	titleBarImage = ImageLoad("titlebar.png");

	//ColorRect(0, 0, $width, 20, colorHighlightBg);
	//DrawImage(titleBarImage);
	//SetDrawColor(RGB(210,210,210));

	DisplayTextShadowed(FormatTime("HH:NN:SS"), 6, -5, 0xB328F612, RGBA(255,255,255,240));
	DisplayTextRightShadowed(FormatTime("Ddd DD.MM.YY"), $width-8, -5, 0xB328C672, RGBA(255,255,255,240));
		
	//WriteLogLn(FormatTime("HH:NN:SS"), 1);
	
	/*if (GetEmailCount()) {
		DrawImage(emailImage, $width/2-8, 4);
		DisplayText(GetEmailCount(), $width/2+2);
	}//*/
	SetDrawColor(colorText);

}
