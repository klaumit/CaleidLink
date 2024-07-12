/*
 * encode.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: encode.h,v 1.6 2003/06/11 03:27:35 nrt Exp $
 */

#ifndef __ENCODE_H__
#define __ENCODE_H__

#include <itable.h>
#include <ctable.h>

#define CODE_SIZE	(STR_SIZE << 2)

#define code_length( len )	\
     ( CODE_SIZE > ((len) << 2) ? CODE_SIZE : ((len) << 2) )

#define EncodeAddCharAbsolutely( cstr, ptr, c )				\
{									\
  (cstr)[ (ptr)++ ] = (c);						\
}

#define EncodeAddChar( cstr, high, ptr, c )				\
{									\
  (cstr)[ (ptr)++ ] = (c);						\
  if( (ptr) >= ( (high) - 16 ) )					\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
}

#define EncodeAddCharRet( cstr, high, ptr, c )				\
{									\
  (cstr)[ (ptr)++ ] = (c);						\
  if( (ptr) >= ( (high) - 16 ) )					\
    return FALSE;							\
}

public void Encode( i_str_t *istr, char codingSystem,
		    char *code, int *length );

#endif /* __ENCODE_H__ */
