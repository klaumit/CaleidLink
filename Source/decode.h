/*
 * decode.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: decode.h,v 1.5 2003/06/11 03:27:35 nrt Exp $
 */

#ifndef __DECODE_H__
#define __DECODE_H__

#include <itable.h>
#include <ctable.h>

/*
 * ISO2022 & ISO2375 style escape sequence
 */

#define IsIchar( c )		( (c) >= 0x20 && (c) <= 0x2f )
#define IsFchar( c )		( (c) >= 0x30 && (c) <= 0x7e )

#define IsKatakana( c )							\
  ( (c) >= 0x21 && (c) <= 0x5f )

#define IsKatakanaByte( c )						\
  ( (c) >= 0xa1 && (c) <= 0xdf )

#define IsShiftJisByte1( c )						\
  ( ( (c) >= 0x81 && (c) <= 0x9f ) || ( (c) >= 0xe0 && (c) <= 0xfc ) )

#define IsShiftJisByte2( c )						\
  ( ( (c) >= 0x40 && (c) <= 0x7e ) || ( (c) >= 0x80 && (c) <= 0xfc ) )

#define IsBig5Byte1( c )						\
  ( ( (c) >= 0xa1 && (c) <= 0xfe ) )

#define IsBig5Byte2( c )						\
  ( ( (c) >= 0x40 && (c) <= 0x7e ) || ( (c) >= 0xa1 && (c) <= 0xfe ) )

#define IsEucByte( c )							\
  ( (c) >= 0xa1 && (c) <= 0xfe )

#define IsGLchar( c )							\
  ( (c) >= SP && (c) <= DEL )

#define IsGRchar( c )							\
  ( (c) >= 0xa0 && (c) <= 0xff )

#define IsGraphicChar( c )						\
  ( (c) > SP && (c) < DEL )

#define GetChar( str, ptr, length, c )					\
{									\
  if( *(length) == (ptr) ){						\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
  }									\
  if( '\0' == (str)[ (ptr) ] )						\
    /*									\
     * BREAK for EXTERNAL LOOP						\
     */									\
    break;								\
  (c) = (str)[ (ptr)++ ];						\
}

#define GetCharRet( str, ptr, length, c )				\
{									\
  if( *(length) == (ptr) )						\
    return FALSE;							\
  if( '\0' == (str)[ (ptr) ] )						\
    return FALSE;							\
  (c) = (str)[ (ptr)++ ];						\
}

#define DecodeAddIchar( istr, iptr, cset, ic )				\
{									\
  (istr)[ (iptr) ].charset = (cset);					\
  (istr)[ (iptr) ].c = (ic);						\
  (iptr)++;								\
}

#define CSET( region )	(state->cset[ (int)state->gset[ (int)(region) ] ])
#define SSET		(state->cset[ (int)state->sset ])

public void Decode( i_str_t *istr, char codingSystem, char *str, int *shigh );

#endif /* __DECODE_H__ */
