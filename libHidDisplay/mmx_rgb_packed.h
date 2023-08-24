/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2010 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

/* Packed -> Packed conversion */

/*
 *  Needs the following macros:
 *  IN_TYPE:     Type of the input pointer
 *  OUT_TYPE:    Type of the output pointer
 *  IN_ADVANCE:  Input advance
 *  OUT_ADVANCE: Output advance
 *  FUNC_NAME:   Name of the function
 *  NUM_PIXELS:  The number of pixels the conversion processes at once
 *  CONVERT:     This macro takes no arguments and makes the appropriate conversion
 *               from <src> to <dst>
 *  INIT:        Variable declarations and initialization (Optional)
 *  CLEANUP:     Stuff at the end (Optional)
 */

#include "mmx.h"

static void (FUNC_NAME)(const void *_src, int width, const int height, void *_dst, const int sp, const int dp)
{
	int i,j;
	IN_TYPE *src = (IN_TYPE*)_src;
	OUT_TYPE *dst = (OUT_TYPE*)_dst;
  	const int jmax = (width / NUM_PIXELS)+1;

#ifdef INIT
	INIT
#endif

	for (i = 0; i < height; i++){
		src = (IN_TYPE*)_src;
		dst = (OUT_TYPE*)_dst;
		
		j = jmax;
		while(--j){
      		__asm volatile (PREFETCH" 256(%0)\n" : : "r" (src): "memory");
		
			CONVERT
			src += IN_ADVANCE;
			dst += OUT_ADVANCE;
		}
		_src += sp;
		_dst += dp;
	}

#ifdef CLEANUP
	CLEANUP
#endif
}

#undef IN_TYPE 
#undef OUT_TYPE
#undef IN_ADVANCE
#undef OUT_ADVANCE
#undef NUM_PIXELS
#undef FUNC_NAME
#undef CONVERT

#ifdef INIT
#undef INIT
#endif

#ifdef FLIP
#undef FLIP
#endif

#ifdef CLEANUP
#undef CLEANUP
#endif

