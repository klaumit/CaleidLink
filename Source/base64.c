/*
 * base64.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: base64.c,v 1.4 2003/06/11 03:13:49 nrt Exp $
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

#include <import.h>
#include <begin.h>
#include <base64.h>

/*
 * base64 $BJ8;z%F!<%V%k(B
 */
private char base64char[ 64 ] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

/*
 * base64 $B%F!<%V%k(B
 */
private int base64[ 256 ] = {
/* 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 1 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /* 2 */
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, /* 3 */
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 4 */
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /* 5 */
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 6 */
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /* 7 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 8 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 9 */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* a */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* b */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* c */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* d */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* e */
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /* f */
};

/*
 * base64 $B$N%G%3!<%I(B
 *
 * $BJ8;zNs(B str $B$rAv::$7$F(B, base64 $B$G%G%3!<%I$7$?8e$K(B
 * $BJ8;zNs(B res $B$K3JG<$7$F$f$/(B. res $B$O(B NULL $B=*C<$9$k(B.
 * res $B$ND9$5$rJV$9(B. (res $B$ND9$5$O(B str $B$ND9$5$N(B 3/4 $B$K$J$k(B).
 */
public int DecodeBase64( char *str, char *res )
{
  unsigned short acc;			/* 16$B%S%C%H$N0l;~0h(B */
  int val, ptr;

  ptr = 0;
  do {
    /*
     * base64 $B$O(B 6$B%S%C%H(B. 6$B%S%C%H$N%7!<%1%s%9$+$i(B 8$B%S%C%H$NJ8;zNs$r@8@.(B.
     */
    if( 0 <= (val = base64[ (int)*str++ ]) ){
      /*
       * $B$3$N%k!<%W$N@hF,$G(B acc $B$O%/%j%"(B. $B>e$N(B val $B$+$iAH$_N)$F3+;O(B.
       */
      acc = val << 10;
      /*
       * XXXXXX__________		$B>e0L(B 6$B%S%C%H40@.(B
       */

      if( 0 <= (val = base64[ (int)*str++ ]) ){
	acc |= val << 4;
	/*
	 * XXXXXXYYYYYY____		$B>e0L(B 12$B%S%C%H40@.(B
	 */

	res[ ptr++ ] = acc >> 8;

	/*
	 * ( XXXXXXYY )( YYYY____ )	$B>e0L(B 8$B%S%C%H@Z$jN%$7(B
	 */

	acc = ( acc & 0x00ff ) << 8;
	/*
	 * ( YYYY____ )(          )	$B2<0L(B 8$B%S%C%H$r>e0L(B 8$B%S%C%H$K0\F0(B
	 */

	if( 0 <= (val = base64[ (int)*str++ ]) ){
	  acc |= val << 6;
	  /*
	   * YYYYXXXXXX______		$B>e0L(B 10$B%S%C%H40@.(B
	   */

	  res[ ptr++ ] = acc >> 8;
	  /*
	   * ( YYYYXXXX )( XX______ )	$B>e0L(B 8$B%S%C%H@Z$jN%$7(B
	   */

	  acc = ( acc & 0x00ff ) << 8;
	  /*
	   * ( XX______ )(          )	$B2<0L(B 8$B%S%C%H$r>e0L(B 8$B%S%C%H$K0\F0(B
	   */

	  if( 0 <= (val = base64[ (int)*str++ ]) ){
	    acc |= val << 8;
	    /*
	     * XXYYYYYY________		$B>e0L(B 8$B%S%C%H40@.(B
	     */

	    res[ ptr++ ] = acc >> 8;
	    /*
	     * ( XXYYYYYY )( ________ )	$B>e0L(B 8$B%S%C%H@Z$jN%$7(B
	     */
	  }
	}
      }
    }
  } while( val >= 0 );

  res[ ptr ] = '\0';

  return ptr;
}

/*
 * base64 $B$N%(%s%3!<%I(B
 *
 * $BJ8;zNs(B str $B$rAv::$7$F(B, base64 $B$G%(%s%3!<%I$7$?8e$K(B
 * $BJ8;zNs(B res $B$K3JG<$7$F$f$/(B. res $B$O(B NULL $B=*C<$9$k(B.
 * res $B$ND9$5$O(B str $B$ND9$5$N(B 4/3 $B$K$J$k(B.
 */
public boolean_t EncodeBase64( char *str, int length, char *res )
{
  int ch, ptr;

  for( ptr = 0 ; ptr < length ; ptr++ ){
    ch = str[ ptr ] >> 2;
    *res++ = base64char[ ch ];
    ch = ( str[ ptr ] & 0x03 ) << 4;

    ptr++;
    if( ptr < length )
      ch |= ( str[ ptr ] & 0xf0 ) >> 4;
    *res++ = base64char[ ch ];

    if( ptr >= length ){
      *res++ = '=';
      *res++ = '=';
      break;
    }
    ch = ( str[ ptr ] & 0x0f ) << 2;

    ptr++;
    if( ptr < length )
      ch |= ( str[ ptr ] & 0xc0 ) >> 6;
    *res++ = base64char[ ch ];

    if( ptr >= length ){
      *res++ = '=';
      break;
    }
    ch = str[ ptr ] & 0x3f;

    *res++ = base64char[ ch ];
  }

  *res = '\0';

  return TRUE;
}
