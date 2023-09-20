

#include "drawops.h"



ops_clut_t *clutActive = NULL;




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
	printf("pal_setActive: %i\r\n", clut->refId);
	clutActive = clut;
}

ops_clut_t *pal_create (const uint16_t total, const uint16_t refId)
{
	ops_clut_t *clut = (ops_clut_t*)uf_calloc(1, sizeof(ops_clut_t));
	if (clut){
		clut->colourTable16 = (uint16_t*)uf_calloc(total, sizeof(uint16_t));
		if (clut->colourTable16){
			clut->total = total;
			clut->refId = refId;
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
	if (colIdx < clut->total)
		clut->colourTable16[colIdx] = colour;
	return colIdx;
}

uint16_t pal_lookup (ops_clut_t *clut, const uint8_t colIdx)
{
	if (colIdx < clut->total)
		return clut->colourTable16[colIdx];
	else
		return 0;
}


