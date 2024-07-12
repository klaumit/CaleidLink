/*
 * escape.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: escape.c,v 1.4 2003/06/11 03:13:49 nrt Exp $
 */

#include <import.h>
#include <decode.h>
#include <begin.h>
#include <escape.h>

public boolean_t DecodeEscape( i_str_t *istr, int *iptrp,
			       char *str, int *ptrp,
			       int *length, state_t *state )
{
  boolean_t multi;
  boolean_t omitted;
  boolean_t unknownSequence;
  char charset, ic, ch;
  int g = 0;
  int iptr = *iptrp;
  int ptr = *ptrp;

  /*
   * escape sequence
   */

  multi = FALSE;
  omitted = FALSE;
  unknownSequence = FALSE;

  GetCharRet( str, ptr, length, ch );

  switch( ch ){
  case ' ': /* announcer */
    GetCharRet( str, ptr, length, ch );
    /*
     * current implementation just ignores it.
     */
    *iptrp = iptr; *ptrp = ptr;
    return TRUE;
  case '&': /* KOUSIN of registered charset */
    GetCharRet( str, ptr, length, ch );
    /*
     * current implementation just ignores it.
     */
    *iptrp = iptr; *ptrp = ptr;
    return TRUE;
  case '~': /* LS1R */
    state->gset[ GR ] = G1; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case 'n': /* LS2 */
    state->gset[ GL ] = G2; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case '}': /* LS2R */
    state->gset[ GR ] = G2; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case 'o': /* LS3 */
    state->gset[ GL ] = G3; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case '|': /* LS3R */
    state->gset[ GR ] = G3; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case 'N': /* SS2 */
    state->sset = G2; *iptrp = iptr; *ptrp = ptr; return TRUE;
  case 'O': /* SS3 */
    state->sset = G3; *iptrp = iptr; *ptrp = ptr; return TRUE;
  }

  if( ch < SP || IsFchar( ch ) ){
    /*
     * unknown ESC F sequence
     */
    DecodeAddIchar( istr, iptr, ASCII, ESC );
    DecodeAddIchar( istr, iptr, ASCII, ch );
    *iptrp = iptr; *ptrp = ptr;
    return TRUE;
  }

  if( '$' == ch ){
    multi = TRUE;
    GetCharRet( str, ptr, length, ch );
  }

  ic = ch;
  if( ch >= '(' && ch <= '/' ){
    switch( ch ){
    case '(': g = 0; break;
    case ')': g = 1; break;
    case '*': g = 2; break;
    case '+': g = 3; break;
    }
  } else {
    if( '@' == ch || 'A' == ch || 'B' == ch ){
      g = 0;
      omitted = TRUE;
    } else {
      unknownSequence = TRUE;
    }
  }

  if( FALSE == unknownSequence && FALSE == omitted ){
    GetCharRet( str, ptr, length, ch );
  }

  if( TRUE == unknownSequence
     || NOSET == (charset = ItableLookup( ch, multi )) ){
    /*
     * unknown ESC I ... F sequence
     */
    DecodeAddIchar( istr, iptr, ASCII, ESC );
    if( TRUE == multi ){
      DecodeAddIchar( istr, iptr, ASCII, '$' );
    }
    DecodeAddIchar( istr, iptr, ASCII, ic );
    if( FALSE == unknownSequence && FALSE == omitted ){
      DecodeAddIchar( istr, iptr, ASCII, ch );
    } else {
      ch = ic;
    }
    while( ! IsFchar( ch ) ){
      GetCharRet( str, ptr, length, ch );
      DecodeAddIchar( istr, iptr, ASCII, ch );
    }
    *iptrp = iptr; *ptrp = ptr;
    return TRUE;
  }

  state->cset[ g ] = charset;

  *iptrp = iptr; *ptrp = ptr;
  return TRUE;
}
