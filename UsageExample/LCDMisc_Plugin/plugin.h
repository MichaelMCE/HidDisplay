#ifndef PLUGIN_H
#define PLUGIN_H

// Currently exists separately from dlls that support scripted functions.
// May merge in the future.


// Current version of all plugin types.  Don't plan on adding more, but you never know.
#define LCD_PLUGIN_VERSION     1


/******************************
 * Functions for LCD plugins. *
 ******************************/


typedef struct LcdInfo LcdInfo;

// Return 0 on fail.  Function specified by LCDInfo structure.
// Info contains what lcdEnum returned.  Not required.
// Currently always called on each device right after enumerating
// all devices, but may allow for more advanved behavior later.
typedef int (CALLBACK *lcdStart) (LcdInfo *info);

// Function specified by LCDInfo structure.
// Info contains what lcdEnum returned.  Not required.
// Currently always called on each device right before destroying
// it, but may allow for more advanved behavior later.
typedef void (CALLBACK * lcdStop)(LcdInfo *info);

// Function specified by LCDInfo structure.
// Info contains what lcdEnum returned.
// Final function called on info.  Not required.
typedef void (CALLBACK * lcdDestroy)(LcdInfo *info);

typedef struct {
	int version;
	// Bpp is 8 (for 1 or 8 bpp lcds) or 32 (for 24 or 32 bpp lcds).
	int width, height, bpp;
	// Points to a 32-bpp or 8-bpp array.
	unsigned char *bitmap;
	// 32-bit color hDC for a device independent bitmap.  Mostly so I don't
	// have to navigate through the nightmare of creating a compatible DC,
	// creating a DIB bitmap for it, copying the bits over, and then drawing
	// it to the screen in the sample code.
	HDC hDC;
}BitmapInfo;

// Function specified by LCDInfo structure.
// Info contains what lcdEnum returned.  Required function for each LCD.
typedef void (CALLBACK * lcdUpdate)(LcdInfo *info, BitmapInfo *bmp);

// Plugin returns a pointed to a populated data structure in response to lcdEnum.
struct LcdInfo {
	// Version of the interface the plugin supports.
	int version;

	// Both must be >= 1;
	int width;
	int height;

	// Must be 1, 8, 24, or 32.
	// 1 and 8 images are send as 8 bpp structures, and 24 and 32 are passed as 32.
	int bpp;

	// String identifying the LCD.  Ideally, should be unique.
	// Can be used to identify the display in scripts,
	// though currently not used much/at all.  Must be in UTF8.
	char *name;

	// At the moment, mostly not used...
	int refreshRate;

	lcdStart Start;
	lcdStop Stop;
	lcdUpdate Update;
	lcdDestroy Destroy;
};

// Callback functions passed to LCD plugins.  Thread safe,
// so if needed, dll can start its own monitoring thread.

// When called, will stop all lcds the dll is managing, enumerate
// them again, and then start up all returned devices.  Note that
// Other events may be queued, so may not happen before the next
// draw event.  If called when enumerating devices, enumeration will
// continue and then restart later.
typedef void (CALLBACK * lcdDeviceChange)(int id);

// Triggers event, with the specified string as a paramter.  Strings are assumed to be in UTF8.
// If need more than one parameter, can use a delimited list, and scripts can split it themselves.
// If no parameter is needed, param can be null.
typedef void (CALLBACK * lcdTriggerEvent)(int id, char *eventName, char *param);

typedef struct{
	// For simplicity, same as LCDInfo version (LCD_PLUGIN_VERSION).
	int version;

	// Id assigned to plugin for use in callback functions.
	int id;

	lcdDeviceChange DeviceChange;
	lcdTriggerEvent TriggerEvent;
}LcdCallbacks;

#ifdef LCD_TYPEDEFS
/* Exported functions:
 */

// This is called first.
// Return 0 on failure, no other functions will be called.
// Not required, but if present, must return 1 when called or
// no other calls will ever occur.

// Callbacks contains all callback functions.  Callback structure
// will not be modified, so no need to make your own copy.
typedef int (CALLBACK * lcdInit)(LcdCallbacks *callbacks);

// Called when done.  No other commands will be called unless lcdInit()
// is called first.
// Not required.
typedef void (CALLBACK * lcdUninit)();

// Required.  Plugin must allocate and populate info itself.
// Returns null on fail or if there are no devices.
// Returned structure must stay valid until its destroy function is called on itself.
// It will be be called until it returns null.
// All devices will be stopped and then destroyed before enumerating devices again.
typedef LcdInfo * (CALLBACK * lcdEnum)();
#endif




#endif
