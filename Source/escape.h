/*
 * escape.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: escape.h,v 1.4 2003/06/11 03:32:12 nrt Exp $
 */

#ifndef __ESCAPE_H__
#define __ESCAPE_H__

#include <boolean.h>
#include <ctable.h>

public boolean_t DecodeEscape( i_str_t *istr, int *iptrp,
			       char *str, int *ptrp,
			       int *length, state_t *state );

#endif /* __ESCAPE_H__ */
