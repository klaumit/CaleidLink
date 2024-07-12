/*
 * boolean.h
 * All rights reserved. Copyright (C) 1994,1999 by NARITA Tomio
 * $Id: boolean.h,v 1.4 2003/06/11 03:13:49 nrt Exp $
 */

#ifndef __BOOLEAN_H__
#define __BOOLEAN_H__

#include <sys/types.h>

#ifdef NULL
#undef NULL
#endif

#define NULL		0

#ifndef FALSE
#define FALSE		0
#endif

#ifndef TRUE
#define TRUE		!0
#endif

#ifndef boolean_t
#define boolean_t	int
#endif

#endif /* __BOOLEAN_H__ */
