/*
 * util.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: util.c,v 1.5 2000/06/15 07:16:58 nrt Exp $
 */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <import.h>
#include <begin.h>
#include <util.h>

#define BUF_SIZE 		1024

public boolean_t EqualFunc( char *f1, char *f2 )
{
  if( !strncmp( f1, f2, 4 ) )
    return TRUE;
  else
    return FALSE;
}

#define IsShiftedBin( c )	( (c) >= 0xa0 && (c) < 0xc0 )

/*
 * shifted は最大で bin の倍の長さになる. shifted の長さを返す.
 */
public int Bin2Shifted( char *bin, int length, char *shifted )
{
  int bptr, sptr;

  for( bptr = sptr = 0 ; bptr < length ; ){
    if( bin[ bptr ] < 0x20 ){
      shifted[ sptr++ ] = 0xa0 + bin[ bptr++ ];
    } else if( IsShiftedBin( bin[ bptr ] ) ){
      shifted[ sptr++ ] = 0x1b;
      shifted[ sptr++ ] = bin[ bptr++ ];
    } else {
      shifted[ sptr++ ] = bin[ bptr++ ];
    }
  }

  return sptr;
}

/*
 * bin は最大で shifted と同じ長さになる. bin の長さを返す.
 */
public int Shifted2Bin( char *shifted, int length, char *bin )
{
  int bptr, sptr;

  for( bptr = sptr = 0 ; sptr < length ; ){
    if( 0x1b == shifted[ sptr ] ){
      sptr++;
      bin[ bptr++ ] = shifted[ sptr++ ];
    } else if( IsShiftedBin( shifted[ sptr ] ) ){
      bin[ bptr++ ] = shifted[ sptr++ ] - 0xa0;
    } else {
      bin[ bptr++ ] = shifted[ sptr++ ];
    }
  }

  return bptr;
}

#ifdef MSDOS
#include <dos.h>

public void wait100msec()
{
  struct dostime_t snap, now;

  _dos_gettime( &snap );

  snap.hsecond += 10;
  if( snap.hsecond > 99 ){
    snap.hsecond -= 100;
    if( 60 == ++snap.second ){
      snap.second = 0;
      if( 60 == ++snap.minute ){
	snap.minute = 0;
	if( 24 == ++snap.hour ){
	  snap.hour = 0;
	}
      }
    }
  }

  for( ; ; ){
    _dos_gettime( &now );

    if( ( ( now.hour > snap.hour && (0 != snap.hour || 23 != now.hour) )
	 || ( now.hour == snap.hour
	     && ( now.minute > snap.minute
		 || ( now.minute == snap.minute
		     && ( now.second > snap.second
			 || ( now.second == snap.second
			     && now.hsecond >= snap.hsecond ) ) ) ) ) ) ){
      break;
    }
  }
}
#else
#include <unistd.h>

public void Wait100msec()
{
  usleep( 100000 );
}
#endif
