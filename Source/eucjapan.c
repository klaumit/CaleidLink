/*
 * eucjapan.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: eucjapan.c,v 1.5 2003/06/11 03:31:32 nrt Exp $
 */

#include <import.h>
#include <encode.h>
#include <begin.h>
#include <eucjapan.h>

public void EncodeEUCjp( i_str_t *istr, char codingSystem,
			 char *cstr, int *length )
{
  int idx, len = *length, clen = code_length( len );
  ic_t ic;
  char cset;
  int ptr = 0;

  for( idx = 0 ; idx < len ; idx++ ){
    cset = istr[ idx ].charset;
    if( cset >= I_TABLE_SIZE )
      cset = ASCII;
    ic = istr[ idx ].c;
    if( 0 == ic )
      break;
    if( ASCII == cset ){
      EncodeAddChar( cstr, clen, ptr, ic );
      continue;
    } else if( X0208 == cset /*|| C6226 == cset*/ ){
    } else if( X0201KANA == cset ){
      EncodeAddChar( cstr, clen, ptr, SS2 );
    } else if( X0212 == cset ){
      EncodeAddChar( cstr, clen, ptr, SS3 );
    } else {
      EncodeAddChar( cstr, clen, ptr, ic );
      continue;
    }
    if( TRUE == iTable[ (int)cset ].multi ){
      EncodeAddChar( cstr, clen, ptr, 0x80 | MakeByte1( ic ) );
      EncodeAddChar( cstr, clen, ptr, 0x80 | MakeByte2( ic ) );
    } else {
      EncodeAddChar( cstr, clen, ptr, 0x80 | ic );
    }
  }

  cstr[ ptr ] = '\0';
  *length = ptr;
}
