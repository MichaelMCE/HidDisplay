
/* 

Intended for compiling with: https://github.com/MichaelMCE/FlexDisplay
To compile: In Arduino Tools menu, select USB Type 'Raw HID 512'
Timings intended for and tested with CPU speed @ 720mhz


USB Detail:
  VID: 0x16C0
  PID: 0x0486
  Interface: 0
  Receive Endpoint: 0x04
  Transmit Endpoint: 0x83

  Use libusb inf-wizard to install a driver for the 'RawHid interface 0' device.

*/


#include <Arduino.h>
#include "config.h"
#include "libHidDisplayDesc.h"
#include "hid/usb_hid.h"
#include "hid/usb_hid.c"
#include "Teensy.h"

#if ENABLE_OP_PRIMATIVES
#include "draw/drawops.h"
static draw_ops_t drawOpsRoot;
#endif

#if USE_STARTUP_IMAGE
#include "startup_256x142_16.h"
#endif

#if ENABLE_TOUCH_FT5216
static touchCtx_t touchCtx;
#endif



static config_t config;
static usb_rawhid_classex rawHid;
static uint8_t *recvBuffer = NULL;
static uint8_t tmpBuffer[8];




static inline int usb_recv2 (void **buffer)
{
	int ret = rawHid.recv2(buffer, 9);
	recvBuffer = (uint8_t*)*buffer;
	return ret;
}

static inline int usb_recv (void *buffer)
{
	return rawHid.recv(buffer, 9);
}

static inline int usb_send (void *buffer, const size_t size)
{
	return rawHid.send(buffer, size);
}

int usb_read (uint8_t *buffer, const size_t len)
{
	int totalReads = len / PACKET_SIZE;
	if (!totalReads) totalReads = 1;
		
	while (totalReads--){
		int bytesIn = usb_recv(buffer);
		if (bytesIn != PACKET_SIZE)
			return 0;
		buffer += PACKET_SIZE;
	}

	int finalRead = len % PACKET_SIZE;
	if (finalRead){
		int bytesIn = usb_recv2((void**)&recvBuffer);
		if (bytesIn > 0 && bytesIn == PACKET_SIZE)
			memcpy(buffer, recvBuffer, finalRead);
		else
			return 0;
	}
	return 1;
}

static int usb_flush (const size_t len)
{
	int totalReads = (len / PACKET_SIZE);
	totalReads += ((len % PACKET_SIZE) > 0);
	if (!totalReads) totalReads = 1;
		
	while (totalReads--){
		int bytesIn = usb_recv2((void**)&recvBuffer);
		if (bytesIn != PACKET_SIZE)
			return 0;
	}
	return 1;
}

static inline uint32_t calcWriteCrc (rawhid_header_t *desc)
{
	uint32_t crc = desc->op ^ desc->flags;

	for (int i = 0; i < 8; i++)
		crc ^= desc->u.crc.val[i];
		
	return crc;
}

static inline uint32_t decodeOp (rawhid_header_t *desc)
{
	uint32_t crc = calcWriteCrc(desc);
	if (crc != desc->crc){
		//printf("header crc mismatch\r\n");
		return 0;
	}
	return desc->op;
}

static inline uint32_t decodeHeader (rawhid_header_t *header, uint16_t *x1, uint16_t *y1, uint16_t *x2, uint16_t *y2, uint32_t *len)
{
	if ((header->u.write.x2 <= header->u.write.x1) || (header->u.write.y2 <= header->u.write.y1)){
		//printf("header area invalid\r\n");
		return 0;
	}

	if (((header->u.write.x2 - header->u.write.x1)+1) * ((header->u.write.y2 - header->u.write.y1)+1) * sizeof(uint16_t) != header->u.write.len){
		//printf("header size mismatch\r\n");
		return 0;
	}
	
	if (x1) *x1  = header->u.write.x1;
	if (y1) *y1  = header->u.write.y1;
	if (x2) *x2  = header->u.write.x2;
	if (y2) *y2  = header->u.write.y2;
	if (len)*len = header->u.write.len;

	return 1;
}

#if USE_STARTUP_IMAGE
static void setStartupImage ()
{
	const int img_w = 256;
	const int img_h = 142;
	
	int x1 = (TFT_WIDTH - img_w) / 2;
	if (x1 < 0) x1 = 0;
	int y1 = (TFT_HEIGHT - img_h) / 2;
	if (y1 < 0) y1 = 0;

	int x2 = x1 + img_w-1;
	if (x2 > TFT_WIDTH-1) x2 = TFT_WIDTH-1;
	int y2 = y1 + img_h-1;
	if (y2 > TFT_HEIGHT-1) y2 = TFT_HEIGHT-1;
	
	for (int y = y1, row = 0; y <= y2; y++, row++)
		tft_update_array((uint16_t*)frame256x142[row], x1, y, x2, y);
}
#endif

#if ENABLE_TOUCH_FT5216
static void touch_init ()
{
	if (touchCtx.enabled == TOUCH_REPORTS_HALT)
		touch_start(FT5216_INT);
	
	touchCtx.rotate = TOUCH_ROTATION;
	touchCtx.pressed = 0;
	touchCtx.t0 = 0;
}
#endif

void setup ()
{
	Serial.begin(115200);
//	while (!Serial);
	printf("Name: " CFG_STRING "\r\n");
	printf("Serial: " DEVICE_SERIAL_STR "\r\n");
	printf("\r\n");
	
	recvBuffer = tmpBuffer;
	
	// not needed here as is called from within teensy4/usb.c
	//usb_rawhid_configure();
	
	tft_init();
	tft_clear(0x0000);		// some displays require two initial clears
	tft_clear(0x0000);		// first is somethings corrupted

#if USE_STARTUP_IMAGE
	setStartupImage();
	setStartupImage();
#endif
}

static void opSetWriteCfg (rawhid_header_t *desc)
{
	if (desc->flags|RAWHID_OP_FLAG_WINDOW){
		config.tiles.window.x1 = desc->u.config.tiles.window.x1;
		config.tiles.window.y1 = desc->u.config.tiles.window.y1;
		config.tiles.window.x2 = desc->u.config.tiles.window.x2;
		config.tiles.window.y2 = desc->u.config.tiles.window.y2;
	}
	
	config.tiles.across = desc->u.config.tiles.across;
	config.tiles.down = desc->u.config.tiles.down;
	config.tiles.width = desc->u.config.tiles.width;
	config.tiles.height = desc->u.config.tiles.height;
}

int recvArea (recvDataCtx_t *dataCtx, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t *dbuffer = (uint16_t*)tft_getBuffer();
	const int pitch = ((x2 - x1) + 1);
	const int height = (y2 - y1) + 1;
	
	for (int y = 0; y < height; y++){
		uint16_t *pixels = &dbuffer[y*pitch];
			
		for (int x = x1; x <= x2; x++){
			if (dataCtx->inCt >= PACKET_SIZE){
				dataCtx->inCt = 0;
				if (usb_recv2((void**)&dataCtx->readIn) != PACKET_SIZE){
					return 0;
				}
			}
			*(pixels)++ = *((uint16_t*)&dataCtx->readIn[dataCtx->inCt]);
			dataCtx->inCt += 2;
		}
	}
	return 1;
}

static inline uint16_t tileToX (const uint8_t tileX)
{
	return tileX * (TFT_WIDTH/config.tiles.across);
}

static inline uint16_t tileToY (const uint8_t tileY)
{
	return tileY * (TFT_HEIGHT/config.tiles.down);
}

void opWriteTiles (rawhid_header_t *indesc)
{
	rawhid_header_t desc;
	memcpy(&desc, indesc, sizeof(desc));
	
	const int updateDisplay = desc.flags&RAWHID_OP_FLAG_UPDATE;
	const uint16_t x1 = tileToX(config.tiles.window.x1);
	const uint16_t y1 = tileToY(config.tiles.window.y1);
	const uint16_t x2 = tileToX(config.tiles.window.x2);
	const uint16_t y2 = tileToY(config.tiles.window.y2);
	const uint16_t width = config.tiles.width;
	const uint16_t height = config.tiles.height; 
		
	uint16_t x = tileToX(desc.u.tiles.x);
	uint16_t y = tileToY(desc.u.tiles.y);
	int tTiles = desc.u.tiles.total;
	
	recvDataCtx_t dataCtx;
	dataCtx.readIn = (uint8_t*)recvBuffer;
	dataCtx.inCt = PACKET_SIZE+1;

	
	while (tTiles-- > 0){
		recvArea(&dataCtx, x, y, x+width-1, y+height-1);
		
		x += width;
		if (x > x2){
			x = x1;
			y += height;
			if (y > y2){
				y = y1;
				x = x1;
			}
		}
	};
	
	if (updateDisplay)
		tft_update();
}

static void opSendDeviceSerial (rawhid_header_t *desc)
{
	memset(desc, 0, sizeof(*desc));
	
	desc->op = RAWHID_OP_SERIAL;
	desc->u.serialid.u32 = DEVICE_SERIAL_NUM;
	strncpy((char*)desc->u.serialid.str, DEVICE_SERIAL_STR, sizeof(desc->u.serialid.str));

	desc->crc = calcWriteCrc(desc);
	usb_send(desc, sizeof(*desc));
}

static void opSendDeviceCfg (rawhid_header_t *desc)
{
	memset(desc, 0, sizeof(*desc));
	
	desc->op = RAWHID_OP_GETCFG;
	desc->u.cfg.supports = 0;			// todo
	desc->u.cfg.bpp = RAWHID_BPP_16;
	desc->u.cfg.width = TFT_WIDTH;
	desc->u.cfg.height = TFT_HEIGHT;
	desc->u.cfg.pitch = desc->u.cfg.width * sizeof(uint16_t);

#ifdef COL_CLAMP_MAX
	desc->flags |= RAWHID_OP_FLAG_CCLAMP;
	desc->u.cfg.rgbMin = COL_CLAMP_MIN;
	desc->u.cfg.rgbMax = COL_CLAMP_MAX;
#else
	desc->u.cfg.rgbMin = 0;
	desc->u.cfg.rgbMax = 0;
#endif

#if USE_BUFFER_LAYERS
	desc->flags |= RAWHID_OP_FLAG_LAYERS;
	desc->u.cfg.layersActive = tft_getWriteLayer();
	desc->u.cfg.layersTotal = BUFFER_LAYERS_TOTAL;
#endif

#if USE_STRIP_RENDERER
	desc->flags |= RAWHID_OP_FLAG_SRENDR;
	desc->u.cfg.stripHeight = STRIP_RENDERER_HEIGHT;
#endif

	memset(desc->u.cfg.string, 32, sizeof(desc->u.cfg.string));
	strncpy((char*)desc->u.cfg.string, CFG_STRING, sizeof(desc->u.cfg.string));

	desc->crc = calcWriteCrc(desc);
	usb_send(desc, sizeof(*desc));
}

void opGfx (rawhid_header_t *desc)
{
	const int op = desc->u.gfxop.op;
	if (op == RAWHID_GFX_CLEAR){
		tft_clear(desc->u.gfxop.var16[0]);
		if (desc->flags&RAWHID_OP_FLAG_UPDATE)
			tft_update();

	}else if (op == RAWHID_GFX_SCROLL){
		// todo 

	}else if (op == RAWHID_GFX_BACKLIGHT){
		tft_backlight(desc->u.gfxop.var8[0]);
		
	}else if (op == RAWHID_GFX_ROTATE){
		tft_rotate(desc->u.gfxop.var8[0]);
	}
}

#if USE_STRIP_RENDERER
void opRecvImage (rawhid_header_t *header)
{
	const int updateDisplay = header->flags&RAWHID_OP_FLAG_UPDATE;
	
	recvDataCtx_t dataCtx;
	dataCtx.readIn = (uint8_t*)recvBuffer;
	dataCtx.inCt = PACKET_SIZE+1;
	
	int trows = STRIP_RENDERER_HEIGHT;
	const int iter = TFT_HEIGHT / STRIP_RENDERER_HEIGHT;
	
	for (int y = 0; y < iter; y++){
		if (!recvArea(&dataCtx, 0, (y*trows), TFT_WIDTH-1, (y*trows)+trows-1))
			return;
		if (updateDisplay)
			tft_update_area(0, (y*trows), TFT_WIDTH-1, (y*trows)+trows-1);
	}


	const int remaining = TFT_HEIGHT % STRIP_RENDERER_HEIGHT;
	if (remaining){
		if (!recvArea(&dataCtx, 0, (iter*trows), TFT_WIDTH-1, (iter*trows)+remaining-1))
			return;
		if (updateDisplay)
			tft_update_area(0, (iter*trows), TFT_WIDTH-1, (iter*trows)+remaining-1);
	}
}
#else	
void opRecvImage (rawhid_header_t *header)
{
	const int updateDisplay = header->flags&RAWHID_OP_FLAG_UPDATE;
	uint32_t len = 0;
	
	if (decodeHeader(header, NULL, NULL, NULL, NULL, &len)){
		usb_read((uint8_t*)tft_getBuffer(), len);

		if (updateDisplay)
			tft_update();
	}
}
#endif

void opRecvImageArea (rawhid_header_t *header)
{
	const int updateDisplay = header->flags&RAWHID_OP_FLAG_UPDATE;

	uint16_t x1 = 0;
	uint16_t y1 = 0;
	uint16_t x2 = 0;
	uint16_t y2 = 0;

	uint8_t *dbuffer = (uint8_t*)tft_getBuffer();
	uint8_t *readIn = (uint8_t*)recvBuffer;
	uint32_t len = 0;

	if (decodeHeader(header, &x1, &y1, &x2, &y2, &len)){
		uint32_t tReads = len / PACKET_SIZE;
		for (uint32_t i = 0; i < tReads; i++){
			if (usb_recv2((void**)&readIn) != PACKET_SIZE)
				return;

			memcpy(dbuffer, readIn, PACKET_SIZE);
			dbuffer += PACKET_SIZE;
		}
		
		uint32_t remaining = len % PACKET_SIZE;
		if (remaining){
			if (usb_recv2((void**)&readIn) != PACKET_SIZE)
				return;

			memcpy(dbuffer, readIn, remaining);
		}
	}
	if (updateDisplay)
		tft_update_area(x1, y1, x2, y2);
}


#if ENABLE_TOUCH_FT5216
static void opTouchCtrl (touchCtx_t *ctx, rawhid_header_t *header)
{
	if (header->flags&RAWHID_OP_FLAG_REPORTSON){
		if (!ctx->enabled) touch_init();
		ctx->enabled = TOUCH_REPORTS_ON;
	}else if (header->flags&RAWHID_OP_FLAG_REPORTSOFF){
		ctx->enabled = TOUCH_REPORTS_OFF;
	}
	
	if (header->flags&RAWHID_OP_FLAG_TOUCHDIR)
		ctx->rotate = header->u.touch.direction;
		if (ctx->rotate == TOUCH_DIR_DEFAULT)
			ctx->rotate = TOUCH_ROTATION;
}

static void opSendTouch (touchCtx_t *ctx, touch_t *touch, const int isReleased)
{
	rawhid_header_t *desc = &ctx->header;
	memset(desc, 0, sizeof(*desc));

	desc->op = RAWHID_OP_TOUCH;
	desc->u.touch = *touch;
	desc->u.touch.time = ctx->t0;
	
	if (!isReleased){
		desc->u.touch.flags = RAWHID_OP_TOUCH_POINTS;
	}else{
		desc->u.touch.flags = RAWHID_OP_TOUCH_RELEASE;
		ctx->t0 = 0;
	}
	
	desc->crc = calcWriteCrc(desc);
	usb_send(desc, sizeof(*desc));
}
	
static void do_touch (touchCtx_t *ctx, touch_t *touch)
{
	static int noTouchCt = 0;
	
	if (ctx->enabled == TOUCH_REPORTS_HALT)
		return;

	if (touch_isPressed()){
		memset(touch, 0, sizeof(touch_t));
		touch->direction = ctx->rotate;
				
		if (touch_process(touch)){
			ctx->pressed = 1;
			for (int i = 0; i < touch->tPoints; i++)
				printf("Touch %i: %i %i\r\n", i+1, touch->points[i].x, touch->points[i].y);
			if (ctx->enabled == TOUCH_REPORTS_ON)
				opSendTouch(ctx, touch, 0);
		}
		noTouchCt = 1;
	}else if (ctx->pressed){
		if (++noTouchCt == 3){	// don't be so eagar to send a touch release, allow to check again to be sure
			ctx->pressed = 0;
			printf("Touch released\r\n");
			if (ctx->enabled == TOUCH_REPORTS_ON)
				opSendTouch(ctx, touch, 1);
		}
	}
}
#endif


#if ENABLE_OP_PRIMATIVES

static const inline uint32_t calcCrc (const uint8_t *buffer, const uint32_t blen)
{
	uint32_t crc = *buffer;

	for (uint32_t i = 0; i < blen; i++){
		crc <<= 1;
		crc ^= buffer[i];
	}
	return crc;
}

void drawOps_decode (uint8_t *buffer, int bufferSize, int opTotal, const int refId)
{
	uint8_t *pbuffer = buffer;
	
	strm_hdr_t hdr;
	memcpy(&hdr, pbuffer, sizeof(hdr));
	pbuffer += sizeof(hdr);
	
	uint32_t crc = calcCrc(pbuffer, bufferSize-sizeof(hdr));
	if (crc != hdr.crc){
		printf("ops decode: crc mismatch for %i: 0x%X != 0x%X\r\n", (int)hdr.refId, (int)hdr.crc, (int)crc);
		return;
	}

	for (int i = 0; i < hdr.totalOps; i++){
		strm_op_t strm;
		memcpy(&strm, pbuffer, sizeof(strm));
		pbuffer += sizeof(strm);
		if (strm.idx != i){
			printf("ops decode: op code mismatch: %i != %i\r\n", strm.idx, i);
			return;
		}

		uint8_t *array = (uint8_t*)calloc(1, strm.len);
		if (!array) continue;

		op_base_t base = {.op=(draw_op_t*)array};
		memcpy(base.op, pbuffer, strm.len);
			
		const int opLength = drawOp_size(strm.type);
		if (strm.type == DRAWOP_IMAGE)
			base.image->pixels = (void*)(array + opLength);
			
		else if (strm.type == DRAWOP_POLYLINE)
			base.poly->pts = (pointFlt_t*)(array + opLength);
			
		else if (strm.type == DRAWOP_STRING)
			base.str->buffer = (uint8_t*)(array + opLength);
		
		drawOps_dispatch(&base, strm.type, TFT_WIDTH, TFT_HEIGHT, refId);
		pbuffer += strm.len;
		
		free(array);
	}
}

static void drawOps_add (draw_ops_t *root, draw_ops_t *ops)
{
	// first try adding in to an empty slot
	draw_ops_t *head = root->next;
	while (head){
		if (!head->storage){
			head->storage = ops->storage;
			head->total = ops->total;
			head->length = ops->length;
			head->refId = ops->refId;
			extmem_free(ops);
			return;
		}
		head = head->next;
	}

	// no empty slots exist, add to end
	head = root;
	while (head->next)
		head = head->next;
		
	head->next = ops;
}

void drawOps_remove (draw_ops_t *head, const uint16_t refId)
{
	while (head){
		if (head->refId == refId){
			if (head->storage)
				extmem_free(head->storage);
			head->storage = NULL;
			head->refId = 0;
			return;
		}
		head = head->next;
	}
}

draw_ops_t *drawOps_find (draw_ops_t *head, const uint16_t refId)
{
	while (head){
		if (head->refId == refId)
			return head;
		head = head->next;
	};
	return NULL;
}

void drawOps_execute (draw_ops_t *ops)
{
	if (drawOps_begin(NULL, ops->total, ops->refId)){
		drawOps_decode((uint8_t*)ops->storage, ops->length, ops->total, ops->refId);
		drawOps_end(NULL, ops->total, ops->refId);
	}
}
#endif

void opPalette (rawhid_header_t *desc)
{
	int totalSize = desc->u.pal.length;
	
#if ENABLE_OP_PRIMATIVES
	const uint16_t totalCol = desc->u.pal.total;
	const uint16_t refId = desc->u.pal.refId;

	int readLength = totalSize;
	if (readLength < PACKET_SIZE) readLength = PACKET_SIZE;

	uint8_t *buffer = (uint8_t*)extmem_calloc(1, readLength);
	if (buffer){
		if (usb_read(buffer, readLength)){
			if (totalSize < readLength)
				buffer = (uint8_t*)extmem_realloc((void*)buffer, totalSize);

			// default behaviour for a pal is to search and remove a preexisting palette upon reference clash
			if (refId)
				drawOps_remove(&drawOpsRoot, refId);
			
			draw_ops_t *ops = (draw_ops_t*)extmem_calloc(1, sizeof(draw_ops_t));
			if (ops){
				ops->storage = buffer;
				ops->total = totalCol;
				ops->length = totalSize;
				ops->refId = refId;
				ops->next = NULL;

				if (refId)
					drawOps_add(&drawOpsRoot, ops);
				drawOps_palette(buffer, totalSize, totalCol, refId);
			}
		}else{
			extmem_free(buffer);
		}
	}else
		// memory allocation has failed
		// remove from the usb pipe whatever was to be processed	
#endif
	{
		usb_flush(totalSize);
	}
}

void opPrimatives (rawhid_header_t *desc)
{
	int totalSize = desc->u.drawop.length;

#if ENABLE_OP_PRIMATIVES
	const uint16_t totalOps = desc->u.drawop.total;
	const uint16_t refId = desc->u.drawop.refId;
	const uint8_t  flags = desc->u.drawop.flags; 				// RAWHID_DRAW_STORE | RAWHID_DRAW_REFID | RAWHID_DRAW_EXECUTE;

	int readLength = totalSize;
	if (readLength < PACKET_SIZE) readLength = PACKET_SIZE;

	if ((flags&RAWHID_DRAW_EXECUTE) && !totalOps && refId){		// execute an existing command set
		draw_ops_t *ops = drawOps_find(&drawOpsRoot, refId);
		if (ops)
			drawOps_execute(ops);
		return;
	}

	uint8_t *buffer = (uint8_t*)extmem_calloc(1, readLength);
	if (buffer){
		if (usb_read(buffer, readLength)){
			if (totalSize < readLength)							// data length is less than minimum read size (packet_size), so reclaim that memory
				buffer = (uint8_t*)extmem_realloc((void*)buffer, totalSize);

			if (flags&RAWHID_DRAW_STORE){
				if (flags&RAWHID_DRAW_OVERWRITE)
					drawOps_remove(&drawOpsRoot, refId);

				if (!drawOps_find(&drawOpsRoot, refId)){
					draw_ops_t *ops = (draw_ops_t*)extmem_calloc(1, sizeof(draw_ops_t));
					if (ops){
						ops->storage = buffer;
						ops->total = totalOps;
						ops->length = totalSize;
						ops->refId = refId;
						ops->next = NULL;

						drawOps_add(&drawOpsRoot, ops);
					}
				}
			}

			if (flags&RAWHID_DRAW_EXECUTE){
				if (drawOps_begin(NULL, totalOps, refId)){
					drawOps_decode(buffer, totalSize, totalOps, refId);
					drawOps_end(NULL, totalOps, refId);
				}
			}
		}else{
			extmem_free(buffer);
		}
	}else
		// memory allocation has failed
		// remove from the usb pipe whatever was to be processed
#endif
	{
		usb_flush(totalSize);
	}
}

void opReset (rawhid_header_t *desc)
{
	if (desc->flags&RAWHID_OP_FLAG_RESET)
		SCB_AIRCR = 0x05FA0004;
}

void loop ()
{
	rawhid_header_t *desc = (rawhid_header_t*)recvBuffer;

	while (1){
#if ENABLE_TOUCH_FT5216
	  do_touch(&touchCtx, &touchCtx.touch);
#endif

	  int bytesIn = usb_recv2((void**)&desc);
	  if (bytesIn != PACKET_SIZE) return;

	  const int op = decodeOp(desc);
	  if (op == RAWHID_OP_WRITEAREA){
		  opRecvImageArea(desc);
		
	  }else if (op == RAWHID_OP_WRITEIMAGE){
		  opRecvImage(desc);

	  }else if (op == RAWHID_OP_PRIMATIVE){
		  opPrimatives(desc);

	  }else if (op == RAWHID_OP_PALETTE){
		  opPalette(desc);

	  }else if (op == RAWHID_OP_GFXOP){
		  opGfx(desc);

	  }else if (op == RAWHID_OP_WRITETILE){
		  opWriteTiles(desc);
	
	  }else if (op == RAWHID_OP_GETCFG){
		  opSendDeviceCfg(desc);
		
	  }else if (op == RAWHID_OP_SETCFG){
		  opSetWriteCfg(desc);

	  }else if (op == RAWHID_OP_SERIAL){
		  opSendDeviceSerial(desc);
		  
#if ENABLE_TOUCH_FT5216
	  }else if (op == RAWHID_OP_TOUCH){
		  opTouchCtrl(&touchCtx, desc);
#endif
	  }else if (op == RAWHID_OP_RESET){
	  	  opReset(desc);
	  }
	}
}
