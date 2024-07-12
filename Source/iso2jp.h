/*
 * iso2jp.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: iso2jp.h,v 1.4 2003/06/11 03:30:03 nrt Exp $
 */

#ifndef __ISO2JP_H__
#define __ISO2JP_H__

#include <itable.h>

public void EncodeISO2022jp( i_str_t *istr, char codingSystem,
			     char *cstr, int *length );

#endif /* __ISO2JP_H__ */
