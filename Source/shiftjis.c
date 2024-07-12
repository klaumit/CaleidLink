/*
 * shiftjis.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: shiftjis.c,v 1.5 2003/06/11 03:31:54 nrt Exp $
 */

#include <import.h>
#include <decode.h>
#include <escape.h>
#include <encode.h>
#include <begin.h>
#include <shiftjis.h>

private void msk2jis( char *c )
{
  int c1, c2;

  c1 = (int)c[ 0 ];
  c2 = (int)c[ 1 ];

  if( 0x00e0 <= c1 )
    c1 = ( c1 << 1 ) - 0x0160;			/* 63-94 ku */
  else
    c1 = ( c1 << 1 ) - 0x00e0;			/* 01-62 ku */

  if( 0x009f <= c2 )
    c2 -= 0x007e;				/* even ku */
  else {
    c1--;					/* odd ku */
    if( c2 >= (int)DEL )
      c2 -= 0x0020;
    else
      c2 -= 0x001f;
  }

  c[ 0 ] = (char)c1;
  c[ 1 ] = (char)c2;
}

public void DecodeShiftJis( i_str_t *istr, char codingSystem,
			    char *str, int *length,
			    state_t *state )
{
  char charset = ASCII, ch;
  char c[ ICHAR_WIDTH ];
  int ptr = 0;
  int iptr = 0;

  for( ; ; ){
    GetChar( str, ptr, length, ch );
    if( ch < SP ){
      if( ESC == ch ){
	if( FALSE == DecodeEscape( istr, &iptr, str, &ptr, length, state ) )
	  break;
      } else if( SO == ch )	/* LS1 for 8bit */
	state->gset[ GL ] = G1;
      else if( SI == ch )	/* LS0 for 8bit */
	state->gset[ GL ] = G0;
      else {
	DecodeAddIchar( istr, iptr, ASCII, (ic_t)ch );
      }
    } else {
      if( NULL != state->sset ){
	/*
	 * single shifted character
	 */
	charset = SSET;
	ch &= 0x7f;	/* for EUC */
	state->sset = 0;
	c[ 0 ] = ch;
	if( TRUE == iTable[ (int)charset ].multi ){
	  GetChar( str, ptr, length, ch );
	  ch &= 0x7f;	/* for EUC */
	  c[ 1 ] = ch;
	}
      } else if( 0x80 & ch ){
	if( IsShiftJisByte1( ch ) ){
	  charset = X0208;
	  c[ 0 ] = ch;
	  GetChar( str, ptr, length, ch );
	  c[ 1 ] = ch;
	  msk2jis( c );
	} else if( IsKatakana( 0x7f & ch ) ){
	  charset = X0201KANA;
	  c[ 0 ] = 0x7f & ch;
	} else {
	  DecodeAddIchar( istr, iptr, ASCII, (ic_t)ch );
	  continue;
	}
      } else {
	/*
	 * iso-2022
	 */
	charset = CSET( G0 );
	c[ 0 ] = ch;
	if( TRUE == iTable[ (int)charset ].multi ){
	  GetChar( str, ptr, length, ch );
	  ch &= 0x7f;
	  c[ 1 ] = ch;
	}
      }
      if( TRUE == iTable[ (int)charset ].multi ){
	DecodeAddIchar( istr, iptr, charset, MakeIchar( c[ 0 ], c[ 1 ] ) );
      } else {
	DecodeAddIchar( istr, iptr, charset, (ic_t)c[ 0 ] );
      }
    }
  }

  istr[ iptr ].charset = ASCII;
  istr[ iptr ].c = 0;
  *length = iptr;
}

private void jis2msk( char *c )
{
  int c1, c2;

  c1 = (int)c[ 0 ];
  c2 = (int)c[ 1 ];

  if( 0 == ( c1 & 0x0001 ) )
    c2 += 0x007e;				/* even ku */
  else {
    if( c2 >= 0x0060 )				/* odd ku */
      c2 += 0x0020;
    else
      c2 += 0x001f;
  }

  if( 0x005f <= c1 )
    c1 = ( ( c1 - 0x005f ) >> 1 ) + 0x00e0;	/* 63-94 ku */
  else
    c1 = ( ( c1 - 0x0021 ) >> 1 ) + 0x0081;	/* 01-62 ku */

  c[ 0 ] = (char)c1;
  c[ 1 ] = (char)c2;
}

public void EncodeShiftJis( i_str_t *istr, char codingSystem,
			    char *cstr, int *length )
{
  int idx, len = *length, clen = code_length( len );
  ic_t ic;
  char cset, sj[ 2 ];
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
    } else if( X0208 == cset /*|| C6226 == cset*/ ){
      sj[ 0 ] = MakeByte1( ic );
      sj[ 1 ] = MakeByte2( ic );
      jis2msk( sj );
      EncodeAddChar( cstr, clen, ptr, sj[ 0 ] );
      EncodeAddChar( cstr, clen, ptr, sj[ 1 ] );
    } else if( X0201KANA == cset ){
      EncodeAddChar( cstr, clen, ptr, 0x80 | ic );
    } else {
      EncodeAddChar( cstr, clen, ptr, ic );
    }
  }

  cstr[ ptr ] = NULL;
  *length = ptr;
}
