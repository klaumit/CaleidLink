/*
 * iso2022.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: iso2022.h,v 1.4 2003/06/11 03:31:14 nrt Exp $
 */

#ifndef __ISO2022_H__
#define __ISO2022_H__

#include <itable.h>
#include <ctable.h>

public void DecodeISO2022( i_str_t *istr, char codingSystem,
			   char *str, int *length,
			   state_t *state );

#endif /* __ISO2022_H__ */
