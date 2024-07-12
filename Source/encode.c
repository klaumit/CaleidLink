/*
 * encode.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: encode.c,v 1.5 2003/06/11 03:29:44 nrt Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <import.h>
#include <iso2jp.h>
#include <eucjapan.h>
#include <shiftjis.h>
#include <begin.h>
#include <encode.h>

typedef void (*encode_table_t)( i_str_t *, char, char *, int * );

private encode_table_t encodeTable[ C_TABLE_SIZE ] = {
  EncodeISO2022jp,		/* AUTOSELECT */
  EncodeEUCjp,			/* EUC_JAPAN */
  EncodeShiftJis,		/* SHIFT_JIS */
  EncodeISO2022jp,		/* ISO_2022_JP */
};

public void Encode( i_str_t *istr, char codingSystem,
		    char *code, int *length )
{
  (*encodeTable[ (int)codingSystem ])( istr, codingSystem, code, length );
}
