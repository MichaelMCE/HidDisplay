#pragma once
#ifndef _DRAWOPS_H_
#define _DRAWOPS_H_





#define	DRAWOP_INVALID  0				// something went wrong - not an op
#define	DRAWOP_POINT  1
#define	DRAWOP_LINE  2
#define	DRAWOP_RECT  3
#define	DRAWOP_CIRCLE  4
#define	DRAWOP_ELLIPSE  5
#define	DRAWOP_ARC  6
#define	DRAWOP_TRIANGLE  7
#define	DRAWOP_POLYLINE  8
#define	DRAWOP_COPY  9
#define	DRAWOP_IMAGE  10
#define	DRAWOP_STRING   11
#define	DRAWOPS_TOTAL  12				// number of op commands+1. used for array size calcs'



#define DRAWOP_FLAG_FILLED		0x01
#define DRAWOP_FLAG_INVERT		0x02
#define DRAWOP_FLAG_DOTTED		0x04
#define DRAWOP_FLAG_CLOSEPLY	0x08	// close polygon (connect last to first point)
#define DRAWOP_FLAG_CENTERED	0x10	// center image over x/y


typedef struct _ctable {
	uint16_t total;			// look up table capacity (COLi_TABLESIZE)
	uint16_t refId;

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
}op_point_t;

typedef struct {
	draw_op_t op;
	
	rectFlt_t rt;		// x1 y1, x2 y2
}op_line_t;

typedef struct {
	draw_op_t op;		// var1:total polys. 2 lines equates to 3 points. Maximum number of points is 255
	
	pointFlt_t *pts;	// x y
	uint32_t ptsSize;
}op_poly_t;

typedef struct {
	draw_op_t op;
	
	rectFlt_t rt;		// x1 y1, x2 y2
}op_rect_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x y
	float radius;
}op_circle_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x  y
	pointFlt_t pt2;		// r1 r2
}op_ellipse_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x  y
	pointFlt_t pt2;		// r1 r2
	pointFlt_t pt3;		// a1 a2
}op_arc_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// x1  y1
	pointFlt_t pt2;		// x2  y2
	pointFlt_t pt3;		// x3  y3
}op_triangle_t;

typedef struct {
	draw_op_t op;

	rectFlt_t rt;		// source x1 y1, x2 y2
	pointFlt_t pt1;		// des x y
}op_copy_t;

typedef struct {
	draw_op_t op;
	
	pointFlt_t pt1;		// des x y
	void *pixels;
	uint32_t imageSize;
	uint16_t width;
	uint16_t height;
}op_image_t;

typedef struct {
	draw_op_t op;

	uint32_t length;	// string buffer including NUL
	uint8_t *buffer;
	uint32_t fontId;
	pointFlt_t pt1;		// x y
}op_string_t;

typedef union {
	draw_op_t     *op;

	op_point_t    *pt;
	op_line_t     *line;
	op_poly_t     *poly;
	op_rect_t     *rect;
	op_circle_t   *cir;
	op_ellipse_t  *elp;
	op_arc_t      *arc;
	op_triangle_t *tri;
	op_copy_t     *copy;
	op_image_t	  *image;
	op_string_t   *str;
}op_base_t;


typedef struct draw_ops_t {
	void *storage;
	uint32_t length;
	
	uint16_t total;
	uint16_t refId;
	
	draw_ops_t *next;
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


int drawOps_begin (void *surface, const int totalOps, const int refId);
void drawOps_dispatch (const op_base_t *const base, const int type, const int width, const int height, const int refId);
void drawOps_end (void *surface, const int totalOps, const int refId);

size_t drawOp_size (const uint8_t drawOp);
void drawOps_palette (void *buffer, const int bufferSize, int palTotal, const int refId);



#endif

