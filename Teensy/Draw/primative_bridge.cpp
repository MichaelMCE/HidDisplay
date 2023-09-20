

#include "palette.h"
#include "primative_bridge.h"




/*

This is where you attach a primative library

'surface' varible is currently NULL by default

*/


int bridge_begin (void *surface, const int totalOps, const int refId)
{
	printf("\r\n");
	printf("bridge_begin: total:%i, refId:%i\r\n", totalOps, refId);
	
	// return 1 to accept forthcoming drawing operation(s)
	// return 0 to cancel, losing operation(s)
	return 1;
}

void bridge_end (void *surface, const int totalOps, const int refId)
{
	printf("bridge_end: %i\r\n", refId);
}

void bridge_drawLine (void *surface, const int x1, const int y1, const int x2, const int y2, const uint8_t isDotted, const uint16_t colour)
{
	printf("bridge_drawLine: %i %i, %i %i, %.4X\r\n", x1, y1, x2, y2, colour);
}

void bridge_drawRectangle (void *surface, const int x1, const int y1, const int x2, const int y2, const uint8_t isFilled, const uint16_t colour)
{
	printf("bridge_drawRectangle: %i %i, %i %i\r\n", x1, y1, x2, y2);
}

void bridge_drawPoly (void *surface, pointFlt_t *pts, const uint8_t total, const uint8_t close, const uint16_t colour)
{
	printf("bridge_drawPoly: %i %i\r\n", total, close);

	for (int i = 0; i < total; i++, pts++){
		printf(" %i: %f %f\r\n", i+1, pts->x, pts->y);
		
		// drawLine()
	}
	if (close){
		// drawLine() from last to first
	}
}

void bridge_drawEllipse (void *surface, const int x, const int y, const int r1, const int r2, const uint16_t colour)
{
	printf("bridge_drawEllipse: %i %i, %i %i\r\n", x, y, r1, r2);
}

void bridge_drawCopy (void *surface, const int x1, const int y1, const int x2, const int y2, const int des_x, const int des_y)
{
	printf("bridge_drawCopy: %i %i, %i %i, %i %i\r\n", x1, y1, x2, y2, des_x, des_y);
}

void bridge_drawCircle (void *surface, const int xc, const int yc, const int radius, const uint8_t isFilled, const uint16_t colour)
{
	printf("bridge_drawCircle: %i %i, %i\r\n", xc, yc, radius);
}

void bridge_drawArc (void *surface, const int x, const int y, const int r1, const int r2, const float a1, const float a2, const uint16_t colour)
{
	printf("bridge_drawArc: %i %i, %i %i, %.3f %.3f\r\n", x, y, r1, r2, a1, a2);
}

void bridge_drawTriangle (void *surface, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const uint8_t isFilled, const uint16_t colour)
{
	printf("bridge_drawTriangle: %i %i, %i %i, %i %i\r\n", x1, y1, x2, y2, x3, y3);
}

void bridge_drawPoint (void *surface, const int x, const int y, const uint16_t colour)
{
	printf("bridge_drawPoint: %i %i, %.4X\r\n", x, y, (int)colour);
}

void bridge_drawImage (void *surface, void *pixels, const int isCentered, const int cx, const int cy)
{
	printf("bridge_drawImage: %i %i\r\n", cx, cy);
}

void bridge_drawString (void *surface, void *text, const int x, const int y, const uint32_t fontId, const uint16_t flags, const uint16_t colour)
{
	printf("bridge_drawString: %i: %i %i\r\n", (int)fontId, x, y);
	printf(" '%s'\r\n", (char*)text);
}

int bridge_palette_begin (const uint16_t *palette, const int total, const int refId)
{
	printf("bridge_palette_being: %i %i\r\n", total, refId);
	
	//for (int i = 0; i < total; i++)
	//	printf("%i: %.4X\r\n", i+1, palette[i]);

	// return 1 enables processing via palette.c, replace existing palette
	// return 0 to disable any further processing
	return 1;
}

void bridge_palette_end (ops_clut_t *pal, const int refId)
{
	printf("bridge_palette_end: %i %i\r\n", pal->total, refId);

	for (int i = 0; i < pal->total; i++)
		printf("%i: %.4X\r\n", i+1, pal_lookup(pal, i));
}
