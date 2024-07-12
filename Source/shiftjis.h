/*
 * shiftjis.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: shiftjis.h,v 1.4 2003/06/11 03:30:40 nrt Exp $
 */

#ifndef __SHIFTJIS_H__
#define __SHIFTJIS_H__

#include <itable.h>
#include <ctable.h>

public void DecodeShiftJis( i_str_t *istr, char codingSystem,
			    char *str, int *length,
			    state_t *state );
public void EncodeShiftJis( i_str_t *istr, char codingSystem,
			    char *cstr, int *length );

#endif /* __SHIFTJIS_H__ */
