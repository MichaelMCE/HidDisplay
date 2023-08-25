
#import <constants.h>


dll64 utillsDll "utills_64.dll" "Init";
dll utillsDll int GetNumberOfCPUCores "GetNumberOfCPUCores" ();
dll utillsDll int vlcGetWindowHandle "vlcGetWindowHandle" ();
dll utillsDll int hrmGetValue "hrmGetValue" (wstring name);
dll utillsDll int hrmGetValue64 "hrmGetValueQ" (wstring name);
dll utillsDll function hrmGetInfo "hrmGetInfo";
dll utillsDll int trackGetMetaInt "trackGetMetaInt" (int meta);
dll utillsDll string trackGetMetaStr "trackGetMetaStr" (int meta);
dll utillsDll function trackGetMetaInfo "trackGetMetaInfo";

//dll64 razersbDll "razersb_64.dll" "Init";
//dll razersbDll void razersb_SelectDisplayPad "razersb_SelectDisplayPad" ();
//dll razersbDll void razersb_SelectDisplayKeys "razersb_SelectDisplayKeys" ();
//dll razersbDll int razersb_GetSelectedDisplay "razersb_GetSelectedDisplay" ();

//dll razersbDll void razersb_ClearDisplayPad "razersb_ClearDisplayPad" ();
//dll razersbDll void razersb_ClearDisplayKeys "razersb_ClearDisplayKeys" ();
//dll razersbDll function razer_test "razer_test" ();



/*
$info = trackGetMetaInfo();
$info[MTAG_Artist]: track artist
$info[MTAG_Album]: etc.. refer to constants.h for complete tag list
note that not all fields are filled


$info = hrmGetInfo();
$info[0]: current bpm
$info[1]: average
$info[2]: mode
$info[3]: lowest bpm read
$info[4]: highest bpm read
$info[5]: previous bpm
$info[6]: time (GetTickCount()) when last updated 

*/


