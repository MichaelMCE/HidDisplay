#ifndef _DRAWOP_H
#define _DRAWOP_H



enum _ops {

	DRAWOP_INVALID = 0,				// something went wrong - not an op
	DRAWOP_POINT = 1,
	DRAWOP_LINE,
	DRAWOP_RECT,
	DRAWOP_CIRCLE,
	DRAWOP_ELLIPSE,
	DRAWOP_ARC,
	DRAWOP_TRIANGLE,
	DRAWOP_POLYLINE,
	DRAWOP_COPY,
	DRAWOP_IMAGE,
	DRAWOP_STRING,
	DRAWOPS_TOTAL				// number of op commands+1. used for array size calcs'
};

#define DRAWOP_FLAG_FILLED		0x01
#define DRAWOP_FLAG_INVERT		0x02
#define DRAWOP_FLAG_DOTTED		0x04
#define DRAWOP_FLAG_CLOSEPLY	0x08	// close polygon (connect last to first point)
#define DRAWOP_FLAG_CENTERED	0x10	// center image over x/y




// RGB24 to RGB565
#define COLOUR_24TO16(c)		((uint16_t)(((((c)>>16)&0xF8)<<8) | ((((c)>>8)&0xFC)<<3) | (((c)&0xF8)>>3)))

// 4bit gray scale to RGB565
#define COLOUR_G4TO16(c)		((uint16_t)((((c)&0x0F)<<12) | (((c)&0x0F)<<7) | ((c)&0x0F)<<1))

// 8bit gray scale to RGB24
#define COLOUR_G8TO24(c)		((uint32_t)(((c)<<16) | ((c)<<8) | ((c)&0xFF)))

#define COLOUR_RGB8TO16(r,g,b)	((((r)&0xF8)<<8) | (((g)&0xFC)<<3) | (((b)&0xF8)>>3))


// RGB 565
#define RGB_16_RED			(0xF800)
#define RGB_16_GREEN		(0x07E0)
#define RGB_16_BLUE			(0x001F)
#define RGB_16_WHITE		(RGB_16_RED|RGB_16_GREEN|RGB_16_BLUE)
#define RGB_16_BLACK		(0x0000)
#define RGB_16_MAGENTA		(RGB_16_RED|RGB_16_BLUE)
#define RGB_16_YELLOW		(RGB_16_RED|RGB_16_GREEN)
#define RGB_16_CYAN			(RGB_16_GREEN|RGB_16_BLUE)

// RGB 888
#define RGB_24_RED			(0xFF0000)
#define RGB_24_GREEN		(0x00FF00)
#define RGB_24_BLUE			(0x0000FF)	
#define RGB_24_WHITE		(RGB_24_RED|RGB_24_GREEN|RGB_24_BLUE)
#define RGB_24_BLACK		(0x000000)
#define RGB_24_MAGENTA		(RGB_24_RED|RGB_24_BLUE)
#define RGB_24_YELLOW		(RGB_24_RED|RGB_24_GREEN)
#define RGB_24_CYAN			(RGB_24_GREEN|RGB_24_BLUE)



#define COLOUR_RED			RGB_16_RED
#define COLOUR_GREEN		RGB_16_GREEN
#define COLOUR_BLUE			RGB_16_BLUE
#define COLOUR_WHITE		RGB_16_WHITE
#define COLOUR_BLACK		RGB_16_BLACK
#define COLOUR_MAGENTA		RGB_16_MAGENTA
#define COLOUR_YELLOW		RGB_16_YELLOW
#define COLOUR_CYAN			RGB_16_CYAN
#define COLOUR_CREAM		COLOUR_24TO16(0xEEE7D0)
#define COLOUR_GREY			COLOUR_24TO16(0x111111 * 7)
#define COLOUR_BROWN		COLOUR_24TO16(0x964B00)


#define DRAW_FILLED			1
#define DRAW_INVERT			1
#define DRAW_DOTTED			1
#define DRAW_PLY_CLOSE		1
#define DRAW_IMG_CENTERED	1


typedef struct _ctable {
	uint16_t total;			// look up table capacity (COLi_TABLESIZE)
	uint16_t refId;
	
	uint16_t ct;
	uint16_t unused;

	uint16_t *colourTable16;
}ops_clut_t;


typedef struct {
	uint8_t type;		// which draw op			(DRAWOP_xxxx)
	uint8_t flags;		// flags local to draw op	(DRAWOP_FLAG_xxxx)
	uint8_t colourIdx;	// index in to colour table
	uint8_t var1;
}draw_op_t;

typedef struct {
	float x;
	float y;
}pointFlt_t;

typedef struct {
	float x1;
	float y1;
	float x2;
	float y2;
}rectFlt_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x y
}drawop_point_t;

typedef struct {
	draw_op_t op;
	
	rectFlt_t rt;		// x1 y1, x2 y2
}drawop_line_t;

typedef struct {
	draw_op_t op;		// var1:total polys. 2 lines equates to 3 points. Maximum number of points is 255
	
	pointFlt_t *pts;	// x y
	uint32_t ptsSize;
}drawop_poly_t;

typedef struct {
	draw_op_t op;
	
	rectFlt_t rt;		// x1 y1, x2 y2
}drawop_rect_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x y
	float radius;
}drawop_circle_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x  y
	pointFlt_t pt2;		// r1 r2
}drawop_ellipse_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x  y
	pointFlt_t pt2;		// r1 r2
	pointFlt_t pt3;		// a1 a2
}drawop_arc_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x1  y1
	pointFlt_t pt2;		// x2  y2
	pointFlt_t pt3;		// x3  y3
}drawop_triangle_t;

typedef struct {
	draw_op_t op;

	rectFlt_t rt;		// source x1 y1, x2 y2
	pointFlt_t pt1;		// des x y
}drawop_copy_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// des x y
	void *pixels;
	uint32_t imageSize;
	uint16_t width;
	uint16_t height;
}drawop_image_t;

typedef struct {
	draw_op_t op;

	uint32_t length;	// string buffer including NUL
	uint8_t *buffer;
	uint32_t fontId;
	pointFlt_t pt1;		// x y
}drawop_string_t;

typedef union {
	draw_op_t     *op;

	drawop_point_t    *pt;
	drawop_line_t     *line;
	drawop_poly_t     *poly;
	drawop_rect_t     *rect;
	drawop_circle_t   *cir;
	drawop_ellipse_t  *elp;
	drawop_arc_t      *arc;
	drawop_triangle_t *tri;
	drawop_copy_t     *copy;
	drawop_image_t    *image;
	drawop_string_t   *str;
}drawop_base_t;


typedef struct {
	uint16_t  total;	// max possible ops
	uint16_t  ct;		// total ops added (size of queue)
	
	uint32_t length;	// sum length meta data (size of points, bitmaps, etc..)
	
	uint16_t refId;
	uint8_t unused1;
	uint8_t unused2;

	drawop_base_t *list;
}draw_ops_t;


typedef struct{
	uint16_t totalOps;
	uint16_t refId;
	uint32_t crc;
}strm_hdr_t;

typedef struct{
	uint32_t len;
	uint16_t type;
	uint16_t idx;
}strm_op_t;


draw_ops_t *drawops_create (const uint32_t total, const uint16_t refId);
void drawops_free (draw_ops_t *ops);
int  drawops_total (draw_ops_t *ops);

int drawops_commit (draw_ops_t *ops, const uint32_t flags, teensyRawHidcxt_t *ctx);	// send to display
int drawops_execute (teensyRawHidcxt_t *ctx, const uint16_t refID);				// run


int drawop_Point (draw_ops_t *ops, const float x, const float y, const uint8_t colIdx);
int drawop_Line (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const uint8_t isDotted, const uint8_t colIdx);
int drawop_Rectangle (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const uint8_t isFilled, const uint8_t colIdx);
int drawop_Circle (draw_ops_t *ops, const float xc, const float yc, const float radius, const uint8_t isFilled, const uint8_t colIdx);
int drawop_Ellipse (draw_ops_t *ops, const float xc, const float yc, const float r1, const float r2, const uint8_t colIdx);
int drawop_Poly (draw_ops_t *ops, pointFlt_t *pts, const uint8_t total, const uint8_t close, const uint8_t colIdx);
int drawop_Arc (draw_ops_t *ops, const float x, const float y, const float r1, const float r2, const float a1, const float a2, const uint8_t colIdx);
int drawop_Triangle (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const uint8_t isFilled, const uint8_t colIdx);
int drawop_Image (draw_ops_t *ops, void *pixels, const int imageSize, const uint16_t width, const uint16_t height, const int isCentered, const float des_x, const float des_y);
int drawop_Copy (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const float des_x, const float des_y);
int drawop_String (draw_ops_t *ops, void *str, const int strLen, const uint8_t flags, const uint32_t fontID, const uint16_t colIdx, const float x, const float y);


ops_clut_t *pal_create (const uint16_t total, const uint16_t refId);
void pal_free (ops_clut_t *clut);
uint16_t pal_total (ops_clut_t *clut);

uint16_t pal_commit (ops_clut_t *clut, const uint32_t flags, teensyRawHidcxt_t *ctx);

void pal_setActive (ops_clut_t *clut);	// set the colour table drawing ops are calculated against, client side
uint8_t pal_add (ops_clut_t *clut, const uint8_t colIdx, const uint16_t colour);
uint16_t pal_lookup (ops_clut_t *clut, const uint8_t colIdx);





#endif



