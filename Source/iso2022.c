/*
 * iso2022.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: iso2022.c,v 1.5 2003/06/11 03:32:42 nrt Exp $
 */

#include <stdio.h>

#include <import.h>
#include <decode.h>
#include <escape.h>
#include <begin.h>
#include <iso2022.h>

public void DecodeISO2022( i_str_t *istr, char codingSystem,
			   char *str, int *length,
			   state_t *state )
{
  int region;
  char charset = ASCII, ch;
  char c[ ICHAR_WIDTH ];
  int ptr = 0;
  int iptr = 0;

  for( ; ; ){
//fputs( "get1\n", stderr );
    GetChar( str, ptr, length, ch );
//fputs( "get2\n", stderr );
    if( ch < SP ){
      if( ESC == ch ){
	if( FALSE == DecodeEscape( istr, &iptr, str, &ptr, length, state ) )
	  break;
      } else if( SO == ch )	/* LS1 for 8bit */
	state->gset[ GL ] = G1;
      else if( SI == ch )	/* LS0 for 8bit */
	state->gset[ GL ] = G0;
      else if( EM == ch )
	state->sset = G2;
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
      } else {
	if( 0x80 & ch ){
	  if( SS2 == ch ){
	    state->sset = G2;
	    continue;
	  } else if( SS3 == ch ){
	    state->sset = G3;
	    continue;
	  }
	  region = GR;
	  ch &= 0x7f;
	} else {
	  region = GL;
	}
	charset = CSET( region );
	c[ 0 ] = ch;
	if( TRUE == iTable[ (int)charset ].multi ){
	  GetChar( str, ptr, length, ch );
	  ch &= 0x7f;
	  c[ 1 ] = ch;
	}
      }
//      fprintf( stderr, "add:%d, %02x, %08x:%d\n", charset, (ic_t)c[ 0 ], istr, iptr );
      if( TRUE == iTable[ (int)charset ].multi ){
//	fputs( "multi\n", stderr );
	DecodeAddIchar( istr, iptr, charset, MakeIchar( c[ 0 ], c[ 1 ] ) );
      } else {
//	fputs( "single1\n", stderr );
	DecodeAddIchar( istr, iptr, charset, (ic_t)c[ 0 ] );
//	fputs( "single2\n", stderr );
      }
    }
  }

  istr[ iptr ].charset = ASCII;
  istr[ iptr ].c = 0;
  *length = iptr;
}
