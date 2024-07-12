/*
 * sync.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: sync.h,v 1.5 2000/06/01 16:41:08 nrt Exp $
 */

#ifndef __SYNC_H__
#define __SYNC_H__

public boolean_t Synchronize( char *dev, long baud,
			      char *file, char codingSystem,
			      boolean_t flagForce,
			      char *log );

#endif /* __SYNC_H__ */
