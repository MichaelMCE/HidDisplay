

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <process.h>
#include <windows.h>
#include "libTeensyRawHid.h"




int libTeensyRawHid_Init ()
{
	volatile static int initOnce = 0;		// init usb once only per process instance
	
	if (!initOnce){
		initOnce = 1;
		usb_init();
	}

    usb_find_busses();
    usb_find_devices();
	return 1;	
}

struct usb_device *find_TeensyRawHid (teensyRawHidcxt_t *ctx, int index, const int interface)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    
    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next) {
        for (dev = usb_bus->devices; dev; dev = dev->next) {
        	//printf("device: %.4X %.4X, %i\n", dev->descriptor.idVendor, dev->descriptor.idProduct, dev->config->interface->altsetting->bInterfaceNumber);
            if ((dev->descriptor.idVendor == LIBUSB_VENDERID) && (dev->descriptor.idProduct == LIBUSB_PRODUCTID)){
            	//printf("dev->config->interface->altsetting->bInterfaceNumber %i\n", dev->config->interface->altsetting->bInterfaceNumber);
				if (dev->config->interface->altsetting->bInterfaceNumber == interface){
            		if (index-- == 0){
						//printf("libTeensyRawHid: device found:  0x%X:0x%X '%s' {%i}\n", dev->descriptor.idVendor, dev->descriptor.idProduct, dev->filename, interface);
						strcpy(ctx->path, dev->filename);
                		return dev;
                	}
                }
			}
        }
    }
    return NULL;
}

int libTeensyRawHid_GetDeviceTotal (const int interface)
{
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    int ct = 0;
    
    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if ((dev->descriptor.idVendor == LIBUSB_VENDERID) && (dev->descriptor.idProduct == LIBUSB_PRODUCTID)){
				if (dev->config->interface->altsetting->bInterfaceNumber == interface){
					ct++;
                }
			}
        }
    }
    return ct;
}

int libTeensyRawHid_GetDisplayTotal ()
{
    return libTeensyRawHid_GetDeviceTotal(TEENSYRAWHID_INTERFACE);
}

int libTeensyRawHid_Open (teensyRawHidcxt_t *ctx, const uint32_t interface, uint32_t index)
{
	libTeensyRawHid_Init();

	ctx->usbdev = find_TeensyRawHid(ctx, index, interface);
	if (ctx->usbdev){
		ctx->usb_handle = usb_open(ctx->usbdev); 
		if (ctx->usb_handle == NULL){
			//printf("libTeensyRawHid_Open(): error: usb_open\n");
			return 0;
		}

		if (usb_set_configuration(ctx->usb_handle, 1) < 0){
			//printf("libTeensyRawHid_Open(): error: setting config 1 failed\n");
			//printf("%s\n", usb_strerror());
			usb_close(ctx->usb_handle);
			return 0;
		}

		if (usb_claim_interface(ctx->usb_handle, interface) < 0){
			//printf("libTeensyRawHid_Open(): error: claiming interface 0 failed\n");
			//printf("%s\n", usb_strerror());
			usb_close(ctx->usb_handle);
			return 0;
		}

		//usb_set_altinterface(ctx->usb_handle, 1);
		//printf("libTeensyRawHid_Open: interface %i open\n", interface);
		return 1;
	}
	return 0;
}


int libTeensyRawHid_OpenDisplay (teensyRawHidcxt_t *ctx, uint32_t index)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->interface = TEENSYRAWHID_INTERFACE;
			
	return libTeensyRawHid_Open(ctx, ctx->interface, 0);
}

int libTeensyRawHid_Close (teensyRawHidcxt_t *ctx)
{
	if (ctx->usb_handle){
		usb_release_interface(ctx->usb_handle, ctx->interface);
		usb_close(ctx->usb_handle);
		ctx->usb_handle = NULL;
		return 1;
	}
	return 0;
}

int libTeensyRawHid_CloseDisplay (teensyRawHidcxt_t *ctx)
{
	return libTeensyRawHid_Close(ctx);
}

static int libTeensyRawHid_WriteData (teensyRawHidcxt_t *ctx, void *data, const size_t size)
{
	return usb_bulk_write(ctx->usb_handle, LIBUSB_ENDPOINT, (char*)data, size, 1000);
}

int libTeensyRawHid_WriteImage (teensyRawHidcxt_t *ctx, header_t *idesc, void *pixelData)
{
	//printf("libTeensyRawHid_WriteImage\n");

	idesc->len = ((idesc->x2 - idesc->x1)+1) * ((idesc->y2 - idesc->y1)+1) * sizeof(uint16_t);
	idesc->crc = (idesc->x1 + idesc->y1 + idesc->x2 + idesc->y2) ^ idesc->len;

	char buffer[TEENSYRAWHID_PACKETSIZE];
	memcpy(buffer, idesc, sizeof(*idesc));

	if (libTeensyRawHid_WriteData(ctx, buffer, TEENSYRAWHID_PACKETSIZE) == TEENSYRAWHID_PACKETSIZE){
		// todo: check return, ensure everything was sent
		int ret = libTeensyRawHid_WriteData(ctx, pixelData, idesc->len);
		if (ret != idesc->len){
			//printf("libTeensyRawHid_WriteImage ret != len: len = %i, ret = %i\n", idesc->len, ret);
		}
		return (ret == idesc->len);
	}else{
		//printf("libTeensyRawHid_WriteImage failed: len = %i\n", idesc->len);
	}
	return 0;
}
