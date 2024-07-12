/*
 * eucjapan.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: eucjapan.h,v 1.4 2003/06/11 03:30:22 nrt Exp $
 */

#ifndef __EUCJAPAN_H__
#define __EUCJAPAN_H__

#include <itable.h>

public void EncodeEUCjp( i_str_t *istr, char codingSystem,
			 char *cstr, int *length );

#endif /* __EUCJAPAN_H__ */
