/*
 * mcd.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: mcd.h,v 1.4 1999/01/11 06:36:55 nrt Exp $
 */

/*
 * mcd v.0.8 ダイレクト C コール 呼び出しモデュール
 */

#ifndef __MCD_H__
#define __MCD_H__

#include <boolean.h>

#define BLOCK_SIZE 1024
public char axBuf[ BLOCK_SIZE ];

#define AxChar( i )	(axBuf[ (i) ])

public int Axopen( char *path );
public void Axclose( int handle );

public boolean_t Connected();
public boolean_t Rts();
public boolean_t DTRoff();
public boolean_t DTRon();
public boolean_t XoffSent();
public void Xoff();
public void Xon();
public unsigned Axgets();
public void SendBreak();
public char Axin();
public void Axout( char ch );
public boolean_t Rest();
public boolean_t CanPut();
public boolean_t CanGet();
public void SetSpeed( unsigned long speed );

#endif /* __MCD_H__ */
