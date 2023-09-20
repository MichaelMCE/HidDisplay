

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <process.h>
#include <windows.h>
#include "libHidDisplay.h"





int libHidDisplay_Init ()
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
						//printf("libHidDisplay: device found:  0x%X:0x%X '%s' {%i}\n", dev->descriptor.idVendor, dev->descriptor.idProduct, dev->filename, interface);
						strcpy(ctx->path, dev->filename);
						ctx->wMaxPacketSize = dev->config->interface->altsetting->endpoint->wMaxPacketSize;
                		return dev;
                	}
                }
			}
        }
    }
    return NULL;
}

int libHidDisplay_GetDeviceTotal (const int interface)
{
	libHidDisplay_Init();
	
    struct usb_bus *usb_bus;
    struct usb_device *dev;
    struct usb_bus *busses = usb_get_busses();
    int ct = 0;
    
    for (usb_bus = busses; usb_bus; usb_bus = usb_bus->next){
        for (dev = usb_bus->devices; dev; dev = dev->next){
            if ((dev->descriptor.idVendor == LIBUSB_VENDERID) && (dev->descriptor.idProduct == LIBUSB_PRODUCTID)){
				ct += (dev->config->interface->altsetting->bInterfaceNumber == interface);
			}
        }
    }
    return ct;
}

int libHidDisplay_GetDisplayTotal ()
{
    return libHidDisplay_GetDeviceTotal(RAWHID_INTERFACE);
}

int libHidDisplay_Open (teensyRawHidcxt_t *ctx, const uint32_t interface, uint32_t deviceIndex)
{
	libHidDisplay_Init();

	ctx->usbdev = find_TeensyRawHid(ctx, deviceIndex, interface);
	if (ctx->usbdev){
		ctx->usb_handle = usb_open(ctx->usbdev); 
		if (ctx->usb_handle == NULL){
			//printf("libHidDisplay_Open(): error: usb_open\n");
			return 0;
		}

		if (usb_set_configuration(ctx->usb_handle, 1) < 0){
			//printf("libHidDisplay_Open(): error: setting config 1 failed\n");
			//printf("%s\n", usb_strerror());
			usb_close(ctx->usb_handle);
			return 0;
		}

		if (usb_claim_interface(ctx->usb_handle, interface) < 0){
			//printf("libHidDisplay_Open(): error: claiming interface 0 failed\n");
			//printf("%s\n", usb_strerror());
			usb_close(ctx->usb_handle);
			return 0;
		}

		//usb_set_altinterface(ctx->usb_handle, 1);
		//printf("libHidDisplay_Open: interface %i open\n", interface);
#if USE_WRITE_ASYNC
		usb_bulk_setup_async(ctx->usb_handle, &ctx->async.cxt, LIBUSB_ENDPOINT_WRITE);
		ctx->async.state = 0;
		ctx->async.threadState = 0;
#endif
		ctx->buffer = (uint8_t*)calloc(1, ctx->wMaxPacketSize);
		return (ctx->buffer != NULL);
	}
	return 0;
}

int libHidDisplay_OpenDisplay (teensyRawHidcxt_t *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->interface = RAWHID_INTERFACE;
			
	return libHidDisplay_Open(ctx, ctx->interface, 0);
}

int libHidDisplay_Close (teensyRawHidcxt_t *ctx)
{
	if (ctx->async.threadState)
		libHidDisplay_WriteImageAsyncStop(ctx);

	if (ctx->usb_handle){
		usb_release_interface(ctx->usb_handle, ctx->interface);
		if (ctx->async.cxt){
			usb_free_async(&ctx->async.cxt);
			ctx->async.cxt = NULL;
		}
		usb_close(ctx->usb_handle);
		ctx->usb_handle = NULL;
		if (ctx->buffer){
			free(ctx->buffer);
			ctx->buffer = NULL;
		}
		return 1;
	}
	return 0;
}

int libHidDisplay_CloseDisplay (teensyRawHidcxt_t *ctx)
{
	return libHidDisplay_Close(ctx);
}

static int libHidDisplay_ReadData (teensyRawHidcxt_t *ctx, void *data, const size_t size)
{
	return usb_bulk_read(ctx->usb_handle, LIBUSB_ENDPOINT_READ, (char*)data, size, 400);
}

static int libHidDisplay_WriteData (teensyRawHidcxt_t *ctx, void *data, const size_t size)
{
#if (!USE_WRITE_ASYNC)
	return usb_bulk_write(ctx->usb_handle, LIBUSB_ENDPOINT_WRITE, (char*)data, size, 1000);
#else
	 if (ctx->async.state){		// wait for previous call to complete, else strange things happen
	 	if (usb_reap_async(ctx->async.cxt, 1000) < 1){
	 		ctx->async.state = 0;
	 		return 0;
	 	}
	 }
	 if (usb_submit_async(ctx->async.cxt, (char*)data, (int)size) < 0){
	 	return 0;
	 }else{
	 	ctx->async.state = 1;
	 	return size;
	 }
#endif
}

static int libHidDisplay_WriteDataWait (teensyRawHidcxt_t *ctx, void *data, const size_t size)
{
	return usb_bulk_write(ctx->usb_handle, LIBUSB_ENDPOINT_WRITE, (char*)data, size, 1000);
}

static uint32_t calcWriteCrc (const rawhid_header_t *desc)
{
	uint32_t crc = desc->op ^ desc->flags;

	for (int i = 0; i < 8; i++)
		crc ^= desc->u.crc.val[i];
		
	return crc;
}

int libHidDisplay_DrawOpsCommit (teensyRawHidcxt_t *ctx, void *drawOps, const int drawOpsLen, const int totalOps, const uint32_t flags, const int refId)
{
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));
		
	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_PRIMATIVE;
	desc->u.drawop.total = totalOps;
	desc->u.drawop.length = drawOpsLen;
	desc->u.drawop.refId = refId;
	desc->u.drawop.flags = HIDD_DRAW_STORE | HIDD_DRAW_REFID;
	if (flags&HIDD_DRAW_EXECUTE)
		desc->u.drawop.flags |= HIDD_DRAW_EXECUTE;
	if (flags&HIDD_DRAW_OVERWRITE)
		desc->u.drawop.flags |= HIDD_DRAW_OVERWRITE;
	desc->crc = calcWriteCrc(desc);
	

	if (libHidDisplay_WriteDataWait(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		int ret = libHidDisplay_WriteDataWait(ctx, drawOps, desc->u.drawop.length);
		if (ret != (int)desc->u.drawop.length)
			printf("libHidDisplay_DrawCommit ret != len: len = %i, ret = %i\n", desc->u.drawop.length, ret);

		return (ret == (int)desc->u.drawop.length);
	}else{
		printf("libHidDisplay_DrawCommit failed: len = %i\n", desc->u.drawop.length);
		return 0;
	}
}

int libHidDisplay_DrawOpsExecute (teensyRawHidcxt_t *ctx, const int refId)
{
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));
		
	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_PRIMATIVE;
	desc->u.drawop.total = 0;
	desc->u.drawop.length = 0;
	desc->u.drawop.flags = HIDD_DRAW_REFID | HIDD_DRAW_EXECUTE;
	desc->u.drawop.refId = refId;
	desc->crc = calcWriteCrc(desc);
	

	if (libHidDisplay_WriteDataWait(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		//printf("libHidDisplay_DrawOpsExecute OK\n");
		return 1;
	}else{
		printf("libHidDisplay_DrawCommit failed: len = %i\n", desc->u.drawop.length);
		return 0;
	}
}

int libHidDisplay_DrawClutCommit (teensyRawHidcxt_t *ctx, void *colTable, const int colTotal, const size_t colTableSize, const uint16_t refId)
{
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));

	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_PALETTE;
	desc->u.pal.total = colTotal;
	desc->u.pal.length = colTableSize;
	desc->u.pal.refId = refId;
	desc->crc = calcWriteCrc(desc);
	

	if (libHidDisplay_WriteDataWait(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		int ret = libHidDisplay_WriteDataWait(ctx, colTable, colTableSize);
		if (ret != (int)colTableSize)
			printf("libHidDisplay_DrawClutCommit ret != len: len = %i, ret = %i\n", (int)colTableSize, ret);
		return (ret == (int)colTableSize);
	}else{
		printf("libHidDisplay_DrawClutCommit failed: len = %i\n", (int)colTableSize);
		return 0;
	}
}

int libHidDisplay_Reset (teensyRawHidcxt_t *ctx)
{
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));

	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_RESET;
	desc->flags = RAWHID_OP_FLAG_RESET;
	desc->crc = calcWriteCrc(desc);

	if (libHidDisplay_WriteDataWait(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		return 1;
	}else{
		return 0;
	}
}

int libHidDisplay_WriteImage (teensyRawHidcxt_t *ctx, void *pixelData)
{
	//printf("libHidDisplay_WriteImage\n");
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));
		
	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_WRITEIMAGE;
	desc->flags = RAWHID_OP_FLAG_UPDATE;
	desc->u.write.x1 = 0;
	desc->u.write.y1 = 0;
	desc->u.write.x2 = ctx->width-1;
	desc->u.write.y2 = ctx->height-1;
	desc->u.write.len = ((desc->u.write.x2 - desc->u.write.x1)+1) * ((desc->u.write.y2 - desc->u.write.y1)+1) * sizeof(uint16_t);
	desc->crc = calcWriteCrc(desc);

	if (libHidDisplay_WriteData(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		// todo: check return, ensure everything was sent
		int ret = libHidDisplay_WriteData(ctx, pixelData, desc->u.write.len);
		if (ret != (int)desc->u.write.len){
			//printf("libHidDisplay_WriteImage ret != len: len = %i, ret = %i\n", desc->u.write.len, ret);
		}
		return (ret == (int)desc->u.write.len);
	}else{
		//printf("libHidDisplay_WriteImage failed: len = %i\n", desc->u.write.len);
	}
	return 0;
}

int libHidDisplay_WriteImageEx (teensyRawHidcxt_t *ctx, uint16_t *pixelData, const uint8_t maxComponentValue)
{
	const int tpixels = ctx->width * ctx->height;
	
	uint8_t r, g, b;
	rgb16_t *p;
	
	for (int i = 0; i < tpixels; i++){
		p = (rgb16_t*)&pixelData[i];
		
		r = p->r << 3;
		if (r > maxComponentValue) r = maxComponentValue;
		p->r = r>>3;
				
		g = p->g << 2;
		if (g > maxComponentValue) g = maxComponentValue;
		p->g = g>>2;

		b = p->b << 3;
		if (b > maxComponentValue) b = maxComponentValue;
		p->b = b>>3;		
	}

	return libHidDisplay_WriteImage(ctx, pixelData);
}

int libHidDisplay_WriteArea (teensyRawHidcxt_t *ctx, void *pixelData, const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2)
{
	//printf("libHidDisplay_WriteImage\n");
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));
		
	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_WRITEAREA;
	desc->flags = RAWHID_OP_FLAG_UPDATE;
	desc->u.write.x1 = x1;
	desc->u.write.y1 = y1;
	desc->u.write.x2 = x2;
	desc->u.write.y2 = y2;
	desc->u.write.len = ((desc->u.write.x2 - desc->u.write.x1)+1) * ((desc->u.write.y2 - desc->u.write.y1)+1) * sizeof(uint16_t);
	desc->crc = calcWriteCrc(desc);

	if (libHidDisplay_WriteData(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		int ret = libHidDisplay_WriteData(ctx, pixelData, desc->u.write.len);
		if (ret != (int)desc->u.write.len){
			printf("libHidDisplay_WriteImage ret != len: len = %i, ret = %i\n", desc->u.write.len, ret);
		}
		return (ret == (int)desc->u.write.len);
	}
	return 0;
}

int libHidDisplay_GetConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc)
{
	memset(desc, 0, sizeof(*desc));
	desc->op = RAWHID_OP_GETCFG;
	desc->crc = calcWriteCrc(desc);	
	
	char buffer[ctx->wMaxPacketSize];
	memcpy(buffer, desc, sizeof(*desc));
	
	if (libHidDisplay_WriteData(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		if (libHidDisplay_ReadData(ctx, buffer, ctx->wMaxPacketSize) == (int)ctx->wMaxPacketSize){
			memcpy(desc, buffer, sizeof(*desc));
			desc->u.cfg.string[sizeof(desc->u.cfg.string)-1] = 0;
			//printf("read cfg:%i %i: '%s'\n", desc->u.cfg.width, desc->u.cfg.height, desc->u.cfg.string);
			return 1;
		}
	}
	return 0;
}

int libHidDisplay_GetReportWait (teensyRawHidcxt_t *ctx, touch_t *touch)
{
	int ret = libHidDisplay_ReadData(ctx, ctx->buffer, ctx->wMaxPacketSize);
	if (ret == (int)ctx->wMaxPacketSize){
		rawhid_header_t *header = (rawhid_header_t*)ctx->buffer;
		uint32_t crc = calcWriteCrc(header);
		if (crc == header->crc && header->op == RAWHID_OP_TOUCH){
			memcpy(touch, &header->u.touch, sizeof(*touch));
			//printf("libHidDisplay_GetReportWait: crc %X %X, %i\n", crc, header->crc, touch->tPoints);
			if (touch->tPoints)
				return touch->tPoints;
			else
				return -1;	// release
		}
	}
	return 0;
}

int libHidDisplay_ClearDisplay (teensyRawHidcxt_t *ctx, const uint16_t colour)
{
	char buffer[ctx->wMaxPacketSize];
	memset(buffer, 0, sizeof(buffer));
		
	rawhid_header_t *desc = (rawhid_header_t*)buffer;
	desc->op = RAWHID_OP_GFXOP;
	desc->flags = RAWHID_OP_FLAG_UPDATE;
	desc->u.gfxop.op = RAWHID_GFX_CLEAR;
	desc->u.gfxop.var16[0] = colour;
	desc->crc = calcWriteCrc(desc);	

	return (libHidDisplay_WriteData(ctx, buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t));
}

int libHidDisplay_TouchReportEnable (teensyRawHidcxt_t *ctx, const int state, const int applyRotation)
{
	memset(ctx->buffer, 0, ctx->wMaxPacketSize);
	rawhid_header_t *desc = (rawhid_header_t*)ctx->buffer;

	desc->op = RAWHID_OP_TOUCH;
	if (state)
		desc->flags = RAWHID_OP_FLAG_REPORTSON;
	else
		desc->flags = RAWHID_OP_FLAG_REPORTSOFF;
		
	if (applyRotation){
		desc->u.touch.direction = applyRotation;
		desc->flags |= RAWHID_OP_FLAG_TOUCHDIR;
	}
	desc->crc = calcWriteCrc(desc);	
	
	if (libHidDisplay_WriteData(ctx, desc, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t))
		return 1;
	
	return 0;
}

int libHidDisplay_SetTileConfig (teensyRawHidcxt_t *ctx, rawhid_header_t *desc)
{

	desc->op = RAWHID_OP_SETCFG;
	desc->flags |= RAWHID_OP_FLAG_WINDOW;
	desc->crc = calcWriteCrc(desc);

	memset(ctx->buffer, 0, ctx->wMaxPacketSize);
	memcpy(ctx->buffer, desc, sizeof(*desc));
	
	if (libHidDisplay_WriteData(ctx, ctx->buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t))
		return 1;
	
	return 0;
}

int libHidDisplay_WriteTiles (teensyRawHidcxt_t *ctx, void *pixels, const uint32_t size, const uint16_t x, const uint16_t y, const uint16_t tTiles)
{
	memset(ctx->buffer, 0, ctx->wMaxPacketSize);
		
	rawhid_header_t *desc = (rawhid_header_t*)ctx->buffer;
	desc->op = RAWHID_OP_WRITETILE;
	desc->flags = RAWHID_OP_FLAG_UPDATE;
	desc->u.tiles.x = x;
	desc->u.tiles.y = y;
	desc->u.tiles.total = tTiles;
	desc->crc = calcWriteCrc(desc);

	if (libHidDisplay_WriteData(ctx, ctx->buffer, sizeof(rawhid_header_t)) == sizeof(rawhid_header_t)){
		int ret = libHidDisplay_WriteData(ctx, pixels, size);
		if (ret != (int)size){
			//printf("libHidDisplay_WriteTile ret != len: len = %i, ret = %i\n", desc->u.write.len, ret);
		}
		return (ret == (int)size);
	}
	return 0;
}
