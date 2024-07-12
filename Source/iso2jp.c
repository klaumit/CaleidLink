/*
 * iso2jp.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: iso2jp.c,v 1.5 2003/06/11 03:32:27 nrt Exp $
 */

#include <stdio.h>

#include <import.h>
#include <encode.h>
#include <begin.h>
#include <iso2jp.h>

/*
 * iso-2022-jp
 *
 * all 94charsets use G0
 */
public void EncodeISO2022jp( i_str_t *istr, char codingSystem,
			     char *cstr, int *length )
{
  int idx, len = *length, clen = code_length( len );
  ic_t ic;
  char cset, lastCset = ASCII;
  int ptr = 0;

  lastCset = ASCII;

  for( idx = 0 ; idx < len ; idx++ ){
    cset = istr[ idx ].charset;
    if( cset >= I_TABLE_SIZE )
      cset = ASCII;
    ic = istr[ idx ].c;
    if( 0 == ic )
      break;
    if( lastCset != cset ){
      EncodeAddCharAbsolutely( cstr, ptr, ESC );
      if( TRUE == iTable[ (int)cset ].multi ){
	EncodeAddCharAbsolutely( cstr, ptr, '$' );
	if( !( cset == X0208 /*|| cset == C6226*/ ) ){
	  EncodeAddCharAbsolutely( cstr, ptr, '(' );
	}
      } else {
	EncodeAddCharAbsolutely( cstr, ptr, '(' ); /* designate set94 to G0 */
      }
      EncodeAddCharAbsolutely( cstr, ptr, iTable[ (int)cset ].fin );
    }
    lastCset = cset;
    if( TRUE == iTable[ (int)cset ].multi ){
      EncodeAddChar( cstr, clen, ptr, MakeByte1( ic ) );
      EncodeAddChar( cstr, clen, ptr, MakeByte2( ic ) );
    } else {
      EncodeAddChar( cstr, clen, ptr, ic );
    }
  }
  if( ASCII != lastCset ){
    EncodeAddCharAbsolutely( cstr, ptr, ESC );
    EncodeAddCharAbsolutely( cstr, ptr, '(' );
    EncodeAddCharAbsolutely( cstr, ptr, iTable[ ASCII ].fin );
  }

  cstr[ ptr ] = '\0';
  *length = ptr;
}
