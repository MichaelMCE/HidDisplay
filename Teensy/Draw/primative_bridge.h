#pragma once
#ifndef _PRIMATIVE_BRIDGE_H_
#define _PRIMATIVE_BRIDGE_H_



int bridge_begin (void *surface, const int totalOps, const int refId);
void bridge_end (void *surface, const int totalOps, const int refId);

void bridge_drawLine (void *surface, const int x1, const int y1, const int x2, const int y2, const uint8_t isDotted, const uint16_t colour);
void bridge_drawRectangle (void *surface, const int x1, const int y1, const int x2, const int y2, const uint8_t isFilled, const uint16_t colour);
void bridge_drawPoly (void *surface, pointFlt_t *pts, const uint8_t total, const uint8_t close, const uint16_t colour);
void bridge_drawEllipse (void *surface, const int x, const int y, const int r1, const int r2, const uint16_t colour);
void bridge_drawCopy (void *surface, const int x1, const int y1, const int x2, const int y2, const int des_x, const int des_y);
void bridge_drawCircle (void *surface, const int xc, const int yc, const int radius, const uint8_t isFilled, const uint16_t colour);
void bridge_drawArc (void *surface, const int x, const int y, const int r1, const int r2, const float a1, const float a2, const uint16_t colour);
void bridge_drawTriangle (void *surface, const int x1, const int y1, const int x2, const int y2, const int x3, const int y3, const uint8_t isFilled, const uint16_t colour);
void bridge_drawPoint (void *surface, const int x, const int y, const uint16_t colour);
void bridge_drawImage (void *surface, void *image, const int isCentered, const int cx, const int cy);
void bridge_drawString (void *surface, void *text, const int x, const int y, const uint32_t fontId, const uint16_t flags, const uint16_t colour);

int bridge_palette_begin (const uint16_t *palette, const int total, const int refId);
void bridge_palette_end (ops_clut_t *pal, const int refId);

#endif
