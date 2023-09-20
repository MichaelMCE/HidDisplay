

#include "drawops.h"
#include "palette.h"
#include "primative_bridge.h"


// palette.c
extern ops_clut_t *clutActive;



static inline uint16_t palLookup (const uint8_t colIdx)
{
	if (colIdx < clutActive->total)
		return clutActive->colourTable16[colIdx];
	else
		return 0;
}

void drawOps_palette (void *buffer, const int bufferSize, int palTotal, const int refId)
{
	uint16_t *palette = (uint16_t*)buffer;
	
	if (bridge_palette_begin(palette, palTotal, refId)){
		ops_clut_t *pal = pal_create(palTotal, refId);
		if (pal){
			for (int i = 0; i < palTotal; i++)
				pal_add(pal, i, palette[i]);

			if (clutActive) pal_free(clutActive);
			pal_setActive(pal);

			bridge_palette_end(pal, pal->refId);
		}
	}
}

int drawOps_begin (void *surface, const int totalOps, const int refId)
{
	return bridge_begin(surface, totalOps, refId);
}

void drawOps_end (void *surface, const int totalOps, const int refId)
{
	bridge_end(surface, totalOps, refId);
}

void drawOps_point (op_point_t *pt, const int width, const int height)
{
	bridge_drawPoint(NULL, (pt->pt1.x*width), (pt->pt1.y*height), palLookup(pt->op.colourIdx));
}

void drawOps_line (op_line_t *line, const int width, const int height)
{
	bridge_drawLine(NULL, (line->rt.x1*width), (line->rt.y1*height), (line->rt.x2*width), (line->rt.y2*height), line->op.flags&DRAWOP_FLAG_DOTTED, palLookup(line->op.colourIdx));
}

void drawOps_rect (op_rect_t *rect, const int width, const int height)
{
	bridge_drawRectangle(NULL, (rect->rt.x1*width), (rect->rt.y1*height), (rect->rt.x2*width), (rect->rt.y2*height), rect->op.flags&DRAWOP_FLAG_FILLED, palLookup(rect->op.colourIdx));
}

void drawOps_circle (op_circle_t *cir, const int width, const int height)
{
	bridge_drawCircle(NULL, (cir->pt1.x*width), (cir->pt1.y*height), cir->radius*height, cir->op.flags&DRAWOP_FLAG_FILLED, palLookup(cir->op.colourIdx));
}

void drawOps_ellipse (op_ellipse_t *elp, const int width, const int height)
{
	bridge_drawEllipse(NULL, (elp->pt1.x*width), (elp->pt1.y*height), (elp->pt2.x*width), (elp->pt2.y*height), palLookup(elp->op.colourIdx));
}

void drawOps_arc (op_arc_t *arc, const int width, const int height)
{
	bridge_drawArc(NULL, (arc->pt1.x*width), (arc->pt1.y*height), (arc->pt2.x*width), (arc->pt2.y*height), (arc->pt3.x*width), (arc->pt3.y*height), palLookup(arc->op.colourIdx));
}

void drawOps_tri (op_triangle_t *tri, const int width, const int height)
{
	bridge_drawTriangle(NULL, (tri->pt1.x*width), (tri->pt1.y*height), (tri->pt2.x*width), (tri->pt2.y*height), (tri->pt3.x*width), (tri->pt3.y*height), tri->op.flags&DRAWOP_FLAG_FILLED, palLookup(tri->op.colourIdx));
}

void drawOps_poly (op_poly_t *poly, const int width, const int height)
{
	bridge_drawPoly(NULL, poly->pts, poly->op.var1, poly->op.flags&DRAWOP_FLAG_CLOSEPLY, palLookup(poly->op.colourIdx));
}

void drawOps_copy (op_copy_t *copy, const int width, const int height)
{
	bridge_drawCopy(NULL, (copy->rt.x1*width), (copy->rt.y1*height), (copy->rt.x2*width), (copy->rt.y2*height), (copy->pt1.x*width), (copy->pt1.y*height));
}

void drawOps_image (op_image_t *image, const int width, const int height)
{
	bridge_drawImage(NULL, image->pixels, image->op.flags&DRAWOP_FLAG_CENTERED, (image->pt1.x*width), (image->pt1.y*height));
}

void drawOps_string (op_string_t *str, const int width, const int height)
{
	bridge_drawString(NULL, str->buffer, (str->pt1.x*width), (str->pt1.y*height), str->fontId, str->op.flags, palLookup(str->op.colourIdx));
}
		
void drawOps_dispatch (const op_base_t *const base, const int type, const int width, const int height, const int refId)
{
	switch (type){
	case DRAWOP_POINT:
		drawOps_point(base->pt, width, height);
		break;
	case DRAWOP_LINE:
		drawOps_line(base->line, width, height);
		break;
	case DRAWOP_RECT:
		drawOps_rect(base->rect, width, height);
		break;
	case DRAWOP_CIRCLE:
		drawOps_circle(base->cir, width, height);
		break;
	case DRAWOP_ELLIPSE:
		drawOps_ellipse(base->elp, width, height);
		break;
	case DRAWOP_ARC:
		drawOps_arc(base->arc, width, height);
		break;
	case DRAWOP_TRIANGLE:
		drawOps_tri(base->tri, width, height);
		break;
	case DRAWOP_POLYLINE:
		drawOps_poly(base->poly, width, height);
		break;
	case DRAWOP_COPY:
		drawOps_copy(base->copy, width, height);
		break;
	case DRAWOP_IMAGE:
		drawOps_image(base->image, width, height);
		break;
	case DRAWOP_STRING:
		drawOps_string(base->str, width, height);
		break;
	default:
	case DRAWOP_INVALID:
		printf("Op: invalid type %i\r\n", type);
	}
}

size_t drawOp_size (const uint8_t drawOp)
{
	static uint8_t opsize[DRAWOPS_TOTAL];

	if (!opsize[DRAWOP_POINT]){
		 opsize[DRAWOP_POINT] = sizeof(op_point_t);
		 opsize[DRAWOP_LINE] = sizeof(op_line_t);
		 opsize[DRAWOP_RECT] = sizeof(op_rect_t);
		 opsize[DRAWOP_CIRCLE] = sizeof(op_circle_t);
		 opsize[DRAWOP_ELLIPSE] = sizeof(op_ellipse_t);
		 opsize[DRAWOP_ARC] = sizeof(op_arc_t);
		 opsize[DRAWOP_TRIANGLE] = sizeof(op_triangle_t);
		 opsize[DRAWOP_POLYLINE] = sizeof(op_poly_t);
		 opsize[DRAWOP_COPY] = sizeof(op_copy_t);
		 opsize[DRAWOP_IMAGE] = sizeof(op_image_t);
		 opsize[DRAWOP_STRING] = sizeof(op_string_t);
	}
	return opsize[drawOp];
}
