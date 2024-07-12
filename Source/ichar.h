/*
 * ichar.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: ichar.h,v 1.4 2003/06/11 03:28:11 nrt Exp $
 */

#ifndef __ICHAR_H__
#define __ICHAR_H__

/*
 * international character
 */

typedef unsigned short		ic_t;

#define ICHAR_WIDTH		2
#define CNTRLWIDTH		1
#define HTAB_INTERNAL_WIDTH	2

#define MakeByte1( ic )		( (char)( (ic) >> 8 ) )
#define MakeByte2( ic )		( (char)( 0x00ff & (ic) ) )
#define MakeIchar( c1, c2 )	( ( (ic_t)(c1) << 8 ) | (ic_t)(c2) )

typedef struct {
  ic_t  c;
  unsigned short charset;
} i_str_t;

#endif /* __ICHAR_H__ */
