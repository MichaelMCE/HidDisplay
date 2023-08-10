
#include <stdio.h>
#include <conio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "HidApi/include/hidapi.h"


#define HID_VID 0x16C0
#define HID_PID 0x0486



int main (int argc, char *argv[])
{   
	if (hid_init()){
		printf("hid_init() failed\n");
		return 0;
	}

	printf("Searching for VID:0x%X PID:0x%X...\n", HID_VID, HID_PID);
	
	struct hid_device_info *deviceInfo = hid_enumerate(HID_VID, HID_PID);
	if (deviceInfo){
		if (deviceInfo->interface_number == 0)	// HidDisplay serial port is on interface 1
			deviceInfo = deviceInfo->next;
	}
	
	if (!deviceInfo){
		printf("Device not found\n");

	}else if (deviceInfo){
		printf("Found:\nInterface: %i\nPath: %s\n", deviceInfo->interface_number, deviceInfo->path);
		printf("Serial #: %ls\n\n", deviceInfo->serial_number);
		
		hid_device *device = hid_open_path(deviceInfo->path);
		if (device){
			printf("Ready\n--------\n");
			uint8_t data[1024];
			
			while (!kbhit()){
				data[0] = 0;
				int ret = hid_read_timeout(device, data, sizeof(data), 500);
				if (ret == 64){
					for (int i = 0; i < ret && data[i]; i++)
						printf("%c", data[i]);
				}
			}
			hid_close(device);
		}
		hid_free_enumeration(deviceInfo);
	}

	hid_exit();
	return 1;

}

