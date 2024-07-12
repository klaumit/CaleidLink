/*
 * guess.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: guess.c,v 1.7 2003/06/11 03:32:58 nrt Exp $
 */

#include <import.h>
#include <ctable.h>
#include <decode.h>
#include <begin.h>
#include <guess.h>

public char GuessCodingSystem( char *str, int length )
{
  int i;
  char ch;
  int sjisPenalty = 0;
  int eucjapanPenalty = 0;

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for iso-2022-jp
     */
    ch = str[ i ];
    if( ESC == ch )
      return ISO_2022_JP;
    if( 0x80 & ch )
      break;
  }
  if( i == length )
    return SHIFT_JIS;

  for( i = 0 ; i < length ; i++ ){
    /*
     * check for euc-japan
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( SS2 == ch ){
	if( ++i >= length )
	  break;
	ch = str[ i ];
	if( !IsKatakanaByte( ch ) )
	  eucjapanPenalty++;
	continue;
      }
      if( SS3 == ch ){
	if( ++i >= length )
	  break;
	ch = str[ i ];
      }
      if( !IsEucByte( ch ) )
	eucjapanPenalty++;
      if( ++i >= length )
	break;
      ch = str[ i ];
      if( !IsEucByte( ch ) )
	eucjapanPenalty++;
    }
  }
  for( i = 0 ; i < length ; i++ ){
    /*
     * check for shift-jis
     */
    ch = str[ i ];
    if( 0x80 & ch ){
      if( IsKatakanaByte( ch ) )
	continue;
      if( !IsShiftJisByte1( ch ) )
	sjisPenalty++;
      ch = str[ ++i ];
      if( i >= length )
	break;
      if( !IsShiftJisByte2( ch ) )
	sjisPenalty++;
    }
  }

  if( 0 == eucjapanPenalty )
    return EUC_JAPAN;
  else if( 0 == sjisPenalty )
    return SHIFT_JIS;
  else if( eucjapanPenalty < sjisPenalty )
    return EUC_JAPAN;
  else
    return SHIFT_JIS;
}
