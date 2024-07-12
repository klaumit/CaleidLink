/*
 * ctable.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: ctable.c,v 1.7 2003/06/11 03:29:26 nrt Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <itable.h>
#include <begin.h>
#include <ctable_t.h>

/*
 * coding system table
 */

public c_table_t cTable[ C_TABLE_SIZE ] = {
  { AUTO_SELECT, {{0, 1}, {ASCII, X0201KANA, ASCII, ASCII}, 0x00, 0x00 } },
  { EUC_JAPAN, {{0, 1}, {ASCII, X0208, X0201KANA, X0212}, 0x00, 0x00 } },
  { SHIFT_JIS, {{0, 1}, {ASCII, X0201KANA, ASCII, ASCII}, 0x00, 0x00 } },
  { ISO_2022_JP, {{0, 3}, {ASCII, X0201KANA, X0201KANA, X0208}, 0x00, 0x00 } },
};

public void CtableInit()
{
  int i;

  for( i = 0 ; i < C_TABLE_SIZE ; i++ )
    if( cTable[ i ].codingSystem != i )
      fprintf( stderr, "invalid charset table\n" ), exit( -1 );
}
