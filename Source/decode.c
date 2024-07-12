/*
 * decode.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: decode.c,v 1.5 2003/06/11 03:30:56 nrt Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <iso2022.h>
#include <shiftjis.h>
#include <begin.h>
#include <decode.h>

typedef void (*decode_table_t)( i_str_t *, char, char *, int *, state_t * );

private decode_table_t decodeTable[ C_TABLE_SIZE ] = {
  DecodeISO2022,		/* AUTOSELECT */
  DecodeISO2022,		/* EUC_JAPAN */
  DecodeShiftJis,		/* SHIFT_JIS */
  DecodeISO2022,		/* ISO_2022_JP */
};

public void Decode( i_str_t *istr, char codingSystem, char *str, int *shigh )
{
  state_t state;

  state = cTable[ (int)codingSystem ].state;

  (*decodeTable[ (int)codingSystem ])( istr, codingSystem,
				       str, shigh, &state );
}
