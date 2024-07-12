/*
 * ctable_t.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: ctable_t.h,v 1.5 2003/06/11 03:27:16 nrt Exp $
 */

#ifndef __CTABLE_T_H__
#define __CTABLE_T_H__

/* coding systems (char) */

#define AUTO_SELECT		0	/* pseudo coding system */

#define EUC_JAPAN		1	/* Extended unix code */
#define SHIFT_JIS		2	/* shift-jis encoding */
#define ISO_2022_JP		3	/* iso-2022-jp */

#define C_TABLE_SIZE		4	/* pseudo coding system */

#define GL			0
#define GR			1

#define G0			0
#define G1			1
#define G2			2
#define G3			3

typedef struct {
  char      gset[ 2 ];
  char      cset[ 4 ];
  char      sset;
  char      attr;
} state_t;

typedef struct {
  char    codingSystem;
  state_t state;
} c_table_t;

public void CtableInit();

#endif /* __CTABLE_T_H__ */
