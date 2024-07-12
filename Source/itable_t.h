/*
 * itable_t.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: itable_t.h,v 1.5 2003/06/11 03:27:53 nrt Exp $
 */

#ifndef __ITABLE_T_H__
#define __ITABLE_T_H__

#include <ascii.h>
#include <ichar.h>

/*
 * character sets's name space (char)
 */

#define ISO646_US	0	/* ISO 646 United states (ANSI X3.4-1968) */
#define X0201ROMAN	1	/* JIS X0201-1976 Japanese Roman */
#define X0201KANA	2	/* JIS X0201-1976 Japanese Katakana */
//#define C6226		3	/* JIS C 6226-1978 Japanese kanji */
#define X0208		3	/* JIS X 0208-1983 Japanese kanji */
#define X0212		4	/* JIS X 0212-1990 Supplementary charset */

#define I_TABLE_SIZE	5

#define NOSET		255

#define ASCII		ISO646_US

#define STR_SIZE	(1024*4)

/*
 * international charset table
 */

typedef struct {
  char       charset;
  char       fin;		/* final character */
  boolean_t  multi;		/* is multi bytes charset */
} i_table_t;

public void ItableInit();
public char ItableLookup( char fin, boolean_t multi );

#endif /* __ITABLE_T_H__ */
