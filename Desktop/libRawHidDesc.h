
#ifndef _LIBRAWHIDDESC_H
#define _LIBRAWHIDDESC_H


#define RAWHID_OP_INVALID		0		// should never happen
#define RAWHID_OP_GETCFG		1
#define RAWHID_OP_WRITEIMAGE	2
#define RAWHID_OP_GFXOP			3		// clear screen, scroll n pixels v/h, etc..
#define RAWHID_OP_WRITEAREA		4
#define RAWHID_OP_SETCFG		5
#define RAWHID_OP_WRITETILE		6

#define RAWHID_GFX_CLEAR		1		// clear internal buffer with colour n, don't update display. .var16[0]/16bit colour, .var32[0]/24/32bit colour
#define RAWHID_GFX_SCROLL		2		// hardware scroll buffer n rows/cols. only if display support scroll
#define RAWHID_GFX_ROTATE		3		// hardware rotate. .var8[0]/rotation (0 to 3)

#define RAWHID_BPP_1			1
#define RAWHID_BPP_4			2
#define RAWHID_BPP_8			3
#define RAWHID_BPP_16			4
#define RAWHID_BPP_24			5
#define RAWHID_BPP_32			6


#define RAWHID_OP_FLAG_UPDATE	0x01	// set to auto update display after Op completion
#define RAWHID_OP_FLAG_WINDOW	0x02	// use to set tile write window
#define RAWHID_OP_FLAG_CCLAMP	0x04	// colour must be clamped according to .rgbMin/Max
#define RAWHID_OP_FLAG_SRENDR	0x08	// strip renderer enabled

typedef struct {
	struct {
		struct {
			uint8_t x1;			// configured write window, in tiles
			uint8_t y1;
			uint8_t x2;
			uint8_t y2;
		}window;
			
		uint8_t across;		// total tiles along the horizontal axis
		uint8_t down;		// total tiles along the vertical axis
		uint8_t width;		// width of s single tile, pixels
		uint8_t height;		// height of a single tile, pixels
	}tiles;
}config_t;

// Synchronize this with Teensy.ino
typedef struct _header_t {
	uint8_t op;						// RAWHID_OP_
	uint8_t flags;					// RAWHID_OP_FLAGS_			// general op specific flags
	uint8_t unused2;
	uint8_t unused3;

	union {
		struct {
			uint8_t supports;		// hardware scrolling, primatives, etc..
			uint8_t bpp;			// RAWHID_BPP_n
			uint16_t pitch;			// row length in bytes (stride)
			
			uint16_t width;
			uint16_t height;
			
			uint8_t rgbMin;			// sets colour component range.
			uint8_t rgbMax;			// no individual component should fall outside this range
			
			uint8_t stripHeight;
			
			uint8_t string[37];		// excludes NUL
		}cfg;
	
		struct {
			uint16_t x1;
			uint16_t y1;
			uint16_t x2;
			uint16_t y2;

			uint32_t len;			
		}write;

		struct {
			uint8_t  op;
			uint8_t  var8[7];
			uint16_t var16[8];
			uint32_t var32[8];
		}gfxop;

		struct {
			uint32_t val[8];		// to enable checksum (.op ^ .val)
		}crc;

		struct {
			uint8_t  x;				// begin tile write from x
			uint8_t  y;				// begin tile write from y
			uint16_t total;			// number of tiles to write. tile writes will wrap around the .window
		}tiles;

		config_t config;
	}u;

	uint32_t crc;
}rawhid_header_t;




#endif

