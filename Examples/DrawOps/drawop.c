
//  Copyright (c) Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include <stdio.h>
#include <conio.h>
#include <wchar.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libHidDisplay.h>
#include "drawop.h"


static ops_clut_t *clutActive = NULL;



static inline void *uf_malloc (size_t size)
{
	return malloc(size);
}

static inline void *uf_calloc (size_t nelem, size_t elsize)
{
	return calloc(nelem, elsize);
}

static inline void uf_free (void *ptr)
{
	free(ptr);
}

static inline void *uf_realloc (void *ptr, size_t elsize)
{
	return realloc(ptr, elsize);
}

void pal_setActive (ops_clut_t *clut)
{
	clutActive = clut;
}

ops_clut_t *pal_create (const uint16_t total, const uint16_t refId)
{
	ops_clut_t *clut = uf_calloc(1, sizeof(ops_clut_t));
	if (clut){
		clut->colourTable16 = uf_calloc(total, sizeof(uint16_t));
		if (clut->colourTable16){
			clut->total = total;
			clut->refId = refId;
			
			if (!clutActive)
				clutActive = clut;
			return clut;
		}else{
			uf_free(clut);
		}
	}
	return NULL;
}

void pal_free (ops_clut_t *clut)
{
	if (clut){
		if (clut->colourTable16)
			uf_free(clut->colourTable16);
		uf_free(clut);
	}
}

uint8_t pal_add (ops_clut_t *clut, const uint8_t colIdx, const uint16_t colour)
{
	if (colIdx < clut->total){
		clut->colourTable16[colIdx] = colour;
		clut->ct++;
	}
	return colIdx;
}

uint16_t pal_total (ops_clut_t *clut)
{
	return clut->ct;
}

uint16_t pal_lookup (ops_clut_t *clut, const uint8_t colIdx)
{
	if (colIdx < clut->total){
		return clut->colourTable16[colIdx];
	}else{
		printf("pal_lookup: invalid colour index referenced %i, maximum == %i\n", colIdx, clut->total-1);
		return 0;
	}
}

uint16_t pal_lookupA (const uint8_t colIdx)
{
	if (colIdx < clutActive->total)
		return clutActive->colourTable16[colIdx];
	else
		return 0;
}

uint16_t pal_commit (ops_clut_t *clut, const uint32_t flags, teensyRawHidcxt_t *ctx)
{
	return libHidDisplay_DrawClutCommit(ctx, clut->colourTable16, clut->ct, clut->total*sizeof(uint16_t), clut->refId);
}

size_t drawop_size (const uint8_t drawOp)
{
	static uint8_t opsize[DRAWOPS_TOTAL];

	if (!opsize[DRAWOP_POINT]){
		opsize[DRAWOP_POINT] = sizeof(drawop_point_t);
		opsize[DRAWOP_LINE] = sizeof(drawop_line_t);
		opsize[DRAWOP_RECT] = sizeof(drawop_rect_t);
		opsize[DRAWOP_CIRCLE] = sizeof(drawop_circle_t);
		opsize[DRAWOP_ELLIPSE] = sizeof(drawop_ellipse_t);
		opsize[DRAWOP_ARC] = sizeof(drawop_arc_t);
		opsize[DRAWOP_TRIANGLE] = sizeof(drawop_triangle_t);
		opsize[DRAWOP_POLYLINE] = sizeof(drawop_poly_t);
		opsize[DRAWOP_COPY] = sizeof(drawop_copy_t);
		opsize[DRAWOP_IMAGE] = sizeof(drawop_image_t);
		opsize[DRAWOP_STRING] = sizeof(drawop_string_t);
		
	}
	return opsize[drawOp];
}

void *drawop_new (const uint8_t drawOp)
{
	if (!drawOp || drawOp >=  DRAWOPS_TOTAL)
		return NULL;

	draw_op_t *op = uf_calloc(1, drawop_size(drawOp));
	if (op)
		op->type = drawOp;
	else
		printf("drawop_new alloc failed for %i\n", drawOp);
	
	return op;	
}

void drawop_release (drawop_base_t *base)
{
	if (base->op->type == DRAWOP_POLYLINE)
		uf_free(base->poly->pts);
	else if (base->op->type == DRAWOP_STRING)
		uf_free(base->str->buffer);

	uf_free(base->op);
}

draw_ops_t *drawops_create (const uint32_t total, const uint16_t refId)
{
	draw_ops_t *ops = uf_calloc(1, sizeof(draw_ops_t));
	if (ops){
		ops->list = uf_calloc(total, sizeof(drawop_base_t*));
		if (ops->list){
			ops->total = total;
			ops->ct = 0;
			ops->length = sizeof(strm_hdr_t);
			ops->refId = refId;
			return ops;
		}
		uf_free(ops);
	}
	return NULL;
}

void drawops_free (draw_ops_t *ops)
{
	for (int i = 0; i < ops->ct; i++)
		drawop_release(&ops->list[i]);

	uf_free(ops->list);
	uf_free(ops);
}

int drawops_total (draw_ops_t *ops)
{
	return ops->total;
}

static int drawop_add (draw_ops_t *ops, void *op)
{
	if (ops->ct+1 >= ops->total){
		int total = ops->total + 32;
		ops->list = uf_realloc(ops->list, (total*sizeof(drawop_base_t*)));
		if (!ops->list)
			return 0;
		else
			ops->total = total;
	}

 	drawop_base_t *base = &ops->list[ops->ct];
 	base->op = op;

	size_t length = drawop_size(base->op->type);
	if (base->op->type == DRAWOP_IMAGE)
		length += base->image->imageSize;
	else if (base->op->type == DRAWOP_POLYLINE)
		length += base->poly->ptsSize;
	else if (base->op->type == DRAWOP_STRING)
		length += base->str->length;

	ops->length += sizeof(strm_op_t);
	ops->length += length;

	return ++ops->ct;
}

int drawop_Point (draw_ops_t *ops, const float x, const float y, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_point_t *pt = drawop_new(DRAWOP_POINT);
	if (pt){
		pt->pt1.x = x;
		pt->pt1.y = y;
		pt->op.colourIdx = colIdx;
		pt->op.flags = 0;

		total = drawop_add(ops, pt);
	}
	return total;
}

int drawop_Line (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const uint8_t isDotted, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_line_t *line = drawop_new(DRAWOP_LINE);
	if (line){
		line->rt.x1 = x1;
		line->rt.y1 = y1;
		line->rt.x2 = x2;
		line->rt.y2 = y2;
		line->op.colourIdx = colIdx;	
		line->op.flags |= (DRAWOP_FLAG_DOTTED*(isDotted&0x01));
	
		total = drawop_add(ops, line);
	}
	return total;
}

int drawop_Rectangle (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const uint8_t isFilled, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_rect_t *rect = drawop_new(DRAWOP_RECT);
	if (rect){
		rect->rt.x1 = x1;
		rect->rt.y1 = y1;
		rect->rt.x2 = x2;
		rect->rt.y2 = y2;
		rect->op.colourIdx = colIdx;	
		rect->op.flags |= (DRAWOP_FLAG_FILLED*(isFilled&0x01));
	
		total = drawop_add(ops, rect);
	}
	return total;
}

int drawop_Circle (draw_ops_t *ops, const float xc, const float yc, const float radius, const uint8_t isFilled, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_circle_t *circle = drawop_new(DRAWOP_CIRCLE);
	if (circle){
		circle->pt1.x = xc;
		circle->pt1.y = yc;
		circle->radius = radius;
		circle->op.colourIdx = colIdx;	
		circle->op.flags |= (DRAWOP_FLAG_FILLED*(isFilled&0x01));

		total = drawop_add(ops, circle);
	}
	return total;
}

int drawop_Ellipse (draw_ops_t *ops, const float xc, const float yc, const float r1, const float r2, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_ellipse_t *ellipse = drawop_new(DRAWOP_ELLIPSE);
	if (ellipse){
		ellipse->pt1.x = xc;
		ellipse->pt1.y = yc;
		ellipse->pt2.x = r1;
		ellipse->pt2.y = r2;
		ellipse->op.colourIdx = colIdx;	
		ellipse->op.flags = 0;	

		total = drawop_add(ops, ellipse);
	}
	return total;
}

int drawop_Poly (draw_ops_t *ops, pointFlt_t *pts, const uint8_t tPts, const uint8_t close, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_poly_t *poly = drawop_new(DRAWOP_POLYLINE);
	if (poly){
		poly->pts = uf_calloc(tPts, sizeof(pointFlt_t));
		for (int i = 0; i < tPts; i++)
			poly->pts[i] = pts[i];
		
		poly->ptsSize = sizeof(pointFlt_t) * tPts;
		poly->op.var1 = tPts;
		poly->op.colourIdx = colIdx;	
		poly->op.flags |= (DRAWOP_FLAG_CLOSEPLY*(close&0x01));

		total = drawop_add(ops, poly);
	}
	return total;
}

int drawop_Arc (draw_ops_t *ops, const float x, const float y, const float r1, const float r2, const float a1, const float a2, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_arc_t *arc = drawop_new(DRAWOP_ARC);
	if (arc){
		arc->pt1.x = x;
		arc->pt1.y = y;
		arc->pt2.x = r1;
		arc->pt2.y = r2;
		arc->pt3.x = a1;
		arc->pt3.y = a2;
		arc->op.colourIdx = colIdx;	
		arc->op.flags = 0;	

		total = drawop_add(ops, arc);
	}
	return total;
}

int drawop_Triangle (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const float x3, const float y3, const uint8_t isFilled, const uint8_t colIdx)
{
	int total = 0;
	
	drawop_triangle_t *triangle = drawop_new(DRAWOP_TRIANGLE);
	if (triangle){
		triangle->pt1.x = x1;
		triangle->pt1.y = y1;
		triangle->pt2.x = x2;
		triangle->pt2.y = y2;
		triangle->pt3.x = x3;
		triangle->pt3.y = y3;
		triangle->op.colourIdx = colIdx;	
		triangle->op.flags |= (DRAWOP_FLAG_FILLED*(isFilled&0x01));

		total = drawop_add(ops, triangle);
	}
	return total;
}

int drawop_Copy (draw_ops_t *ops, const float x1, const float y1, const float x2, const float y2, const float des_x, const float des_y)
{
	int total = 0;
	
	drawop_copy_t *copy = drawop_new(DRAWOP_COPY);
	if (copy){
		copy->rt.x1 = x1;
		copy->rt.y1 = y1;
		copy->rt.x2 = x2;
		copy->rt.y2 = y2;
		copy->pt1.x = des_x;
		copy->pt1.y = des_y;
		copy->op.colourIdx = 0;
		copy->op.flags = 0;	

		total = drawop_add(ops, copy);
	}
	return total;
}

int drawop_String (draw_ops_t *ops, void *text8, const int strLen, const uint8_t flags, const uint32_t fontId, const uint16_t colIdx, const float x, const float y)
{
	int total = 0;
	
	drawop_string_t *str = drawop_new(DRAWOP_STRING);
	if (str){
		str->pt1.x = x;
		str->pt1.y = y;
		str->op.colourIdx = colIdx;
		str->op.flags = flags;
		str->fontId = fontId;
		str->length = strLen+1;
		str->buffer = uf_calloc(1, str->length);
		memcpy(str->buffer, text8, str->length);

		total = drawop_add(ops, str);
	}
	return total;
}

int drawop_Image (draw_ops_t *ops, void *pixels, const int imageSize, const uint16_t width, const uint16_t height, const int isCentered, const float des_x, const float des_y)
{
	int total = 0;
	
	drawop_image_t *img = drawop_new(DRAWOP_IMAGE);
	if (img){
		img->pixels = pixels;
		img->imageSize = imageSize;
		img->width = width;
		img->height = height;
		img->pt1.x = des_x;
		img->pt1.y = des_y;
		img->op.colourIdx = 0;
		img->op.flags |= (DRAWOP_FLAG_CENTERED*(isCentered&0x01));

		total = drawop_add(ops, img);
	}
	return total;
}

int drawops_execute (teensyRawHidcxt_t *ctx, const uint16_t refID)
{
	printf("ops_execute: refID:%i\n", refID);

	return libHidDisplay_DrawOpsExecute(ctx, refID);
}

static const uint32_t calcCrc (const uint8_t *buffer, const uint32_t blen)
{
	uint32_t crc = *buffer;

	for (int i = 0; i < blen; i++){
		crc <<= 1;
		crc ^= buffer[i];
	}
	return crc;
}

static void *drawops_build (draw_ops_t *ops, const uint32_t flags, uint8_t *pbuffer, const uint32_t length)
{
	for (int i = 0; i < ops->ct; i++){
		drawop_base_t *base = &ops->list[i];
		const uint16_t type = base->op->type;
		const uint32_t len = drawop_size(type);

		strm_op_t strm;
		strm.len = len;
		strm.type = type;
		strm.idx = i;

		if (type == DRAWOP_IMAGE){
			strm.len += base->image->imageSize;
			memcpy(pbuffer, &strm, sizeof(strm));
			pbuffer += sizeof(strm);
			
			memcpy(pbuffer, base->image, len);
			pbuffer += len;

			memcpy(pbuffer, base->image->pixels, base->image->imageSize);
			pbuffer += base->image->imageSize;

		}else if (type == DRAWOP_POLYLINE){
			strm.len += base->poly->ptsSize;
			memcpy(pbuffer, &strm, sizeof(strm));
			pbuffer += sizeof(strm);
			
			memcpy(pbuffer, base->poly, len);
			pbuffer += len;
			
			memcpy(pbuffer, base->poly->pts, base->poly->ptsSize);
			pbuffer += base->poly->ptsSize;
			
		}else if (type == DRAWOP_STRING){
			strm.len += base->str->length;
			memcpy(pbuffer, &strm, sizeof(strm));
			pbuffer += sizeof(strm);
			
			memcpy(pbuffer, base->str, len);
			pbuffer += len;
			
			memcpy(pbuffer, base->str->buffer, base->str->length);
			pbuffer += base->str->length;
			
		}else{
			memcpy(pbuffer, &strm, sizeof(strm));
			pbuffer += sizeof(strm);
			
			memcpy(pbuffer, base->op, len);
			pbuffer += len;
		}
	}
	return pbuffer;
}

int drawops_commit (draw_ops_t *ops, const uint32_t flags, teensyRawHidcxt_t *ctx)
{
	int success = 0;
	
	void *buffer = uf_calloc(1, ops->length);
	if (!buffer){
		printf("emotional damage\n");
		return success;
	}
	uint8_t *pbuffer = buffer;
	
	strm_hdr_t hdr;
	hdr.totalOps = ops->ct;
	hdr.refId = ops->refId;
	hdr.crc = 0;
	memcpy(pbuffer, &hdr, sizeof(hdr));
	pbuffer += sizeof(hdr);

	pbuffer = drawops_build(ops, flags, pbuffer, ops->length);

	if ((pbuffer - (uint8_t*)buffer) == ops->length){
		printf("commit size: %i\n", (int)ops->length);

		hdr.totalOps = ops->ct;
		hdr.refId = ops->refId;
		hdr.crc = calcCrc((uint8_t*)(buffer+sizeof(hdr)), ops->length-sizeof(hdr));
		memcpy(buffer, &hdr, sizeof(hdr));
		
		printf("sending: total:%i, crc:0x%X\n", ops->ct, hdr.crc);
		success = libHidDisplay_DrawOpsCommit(ctx, buffer, ops->length, ops->ct, flags&0xFF, ops->refId);
		if (success)
			printf("committed\n");
		else
			printf("commit failed to send\n");
	}

	uf_free(buffer);
	return success;
}

