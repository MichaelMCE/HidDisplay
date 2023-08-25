#include <Windows.h>

void __stdcall DrawStuff (HDC hDC, size_t width, size_t height, size_t bitsPerPixel, int deviceId) {
	char *text = "Look, Mom, I'm on TV!";
	TextOut(hDC, 0, 0, text, strlen(text));

	// Have to call this when done drawing, for safety.
	GdiFlush();
}

int __stdcall ButtonDown(int mask) {
	if (mask == 8) return 1;
	return 0;
}

void __stdcall Init () {
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, void* lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInstance);
	}
    return 1;
}
