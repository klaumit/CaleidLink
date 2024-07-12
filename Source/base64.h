/*
 * base64.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: base64.h,v 1.3 1999/01/11 06:36:54 nrt Exp $
 */

#ifndef __BASE64_H__
#define __BASE64_H__

public int DecodeBase64( char *str, char *res );
public boolean_t EncodeBase64( char *str, int length, char *res );

#endif /* __BASE64_H__ */
