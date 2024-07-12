/*
 * util.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: util.h,v 1.5 2000/06/15 07:16:58 nrt Exp $
 */

#ifndef __UTIL_H__
#define __UTIL_H__

public boolean_t EqualFunc( char *f1, char *f2 );

public int Bin2Shifted( char *bin, int length, char *shifted );
public int Shifted2Bin( char *shifted, int length, char *bin );

public void Wait100msec();

#endif /* __UTIL_H__ */
