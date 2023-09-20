#pragma once
#ifndef _PALETTE_H_
#define _PALETTE_H_


ops_clut_t *pal_create (const uint16_t total, const uint16_t refId);
void pal_free (ops_clut_t *clut);

void pal_setActive (ops_clut_t *clut);

uint8_t pal_add (ops_clut_t *clut, const uint8_t colIdx, const uint16_t colour);
uint16_t pal_lookup (ops_clut_t *clut, const uint8_t colIdx);


#endif

