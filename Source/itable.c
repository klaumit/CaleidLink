/*
 * itable.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: itable.c,v 1.6 2003/06/11 03:29:07 nrt Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <begin.h>
#include <itable_t.h>

/*
 * international character set table
 */

public i_table_t iTable[ I_TABLE_SIZE ] = {
  { ISO646_US,	'B', FALSE },
  { X0201ROMAN,	'J', FALSE },
  { X0201KANA,	'I', FALSE },
  //  { C6226,	'@', TRUE },
  { X0208,	'B', TRUE },
  { X0212,	'D', TRUE },
};

#define I_TABLE_CACHE_SIZE	4

private int iTableCacheIndex = 0;
private boolean_t iTableCacheUsed[ I_TABLE_CACHE_SIZE ];
private i_table_t iTableCache[ I_TABLE_CACHE_SIZE ];

public void ItableInit()
{
  int i;

  for( i = 0 ; i < I_TABLE_SIZE ; i++ )
    if( iTable[ i ].charset != i )
      fprintf( stderr, "invalid ichar table\n" ), exit( -1 );

  for( i = 0 ; i < I_TABLE_CACHE_SIZE ; i++ )
    iTableCacheUsed[ i ] = FALSE;
}

public char ItableLookup( char fin, boolean_t multi )
{
  int i;

  for( i = iTableCacheIndex ; i >= 0 ; i-- ){
    if( TRUE == iTableCacheUsed[ i ]
       && multi == iTableCache[ i ].multi
       && fin == iTableCache[ i ].fin )
	return iTableCache[ i ].charset;
  }
  for( i = I_TABLE_CACHE_SIZE - 1 ; i > iTableCacheIndex ; i-- ){
    if( TRUE == iTableCacheUsed[ i ]
       && multi == iTableCache[ i ].multi
       && fin == iTableCache[ i ].fin )
	return iTableCache[ i ].charset;
  }
  for( i = 0 ; i < I_TABLE_SIZE ; i++ ){
    if( multi == iTable[ i ].multi
       && fin == iTable[ i ].fin ){
      iTableCacheIndex++;
      if( iTableCacheIndex >= I_TABLE_CACHE_SIZE )
	iTableCacheIndex = 0;
      iTableCache[ iTableCacheIndex ] = iTable[ i ];

      return i;
    }
  }

  if( FALSE == multi )
    return ASCII;

  return NOSET;
}
