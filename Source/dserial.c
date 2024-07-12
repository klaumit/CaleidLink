/*
 * dserial.c -- DOS version of serial.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: dserial.c,v 1.8 2003/11/17 03:53:19 nrt Exp $
 */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <dos.h>

#include <import.h>
#include <util.h>
#include <mcd.h>
#include <begin.h>
#include <serial.h>

public boolean_t SerialSetRate( serial_t *serial, unsigned int rate )
{
  unsigned long speed;

  switch( rate ){
  case 4800:
    speed = 4800;  break;
  case 19200:
    speed = 19200; break;
  case 38400:
    speed = 38400; break;
  case 9600:
  default:
    speed = 9600;
  }

  SetSpeed( speed );

  return TRUE;
}

public serial_t *SerialOpen( char *path )
{
  serial_t *serial;

  if( NULL == (serial = (serial_t *)malloc( sizeof( serial_t ) )) )
    return NULL;

  serial->dev = malloc( strlen( path ) + 1 );
  strcpy( serial->dev, path );

  if( 0 > (serial->fd = Axopen( path )) ){
    free( serial );
    return NULL;
  }

  SerialSetRate( serial, 9600 );

  if( TRUE == active_sync ){
    DTRoff();		/* DTR OFF */
    Wait100msec();
    DTRon();		/* DTR ON */
    Wait100msec();
    DTRoff();		/* DTR OFF */
    Wait100msec();
    DTRon();		/* DTR ON */
    Wait100msec();
    DTRoff();		/* DTR OFF */
    Wait100msec();
    DTRon();		/* DTR ON */
  }

  return serial;
}

public boolean_t SerialClose( serial_t *serial )
{
  Axclose( serial->fd );

  free( serial->dev );
  free( serial );

  return TRUE;
}

public boolean_t SerialProbe( serial_t *serial )
{
  return CanGet();
}

public int SerialProbe2( serial_t *serial1, serial_t *serial2 )
{
  /*
   * This won't work. We have no chance to distinguish between serial1 and
   * serial2.
   */
  return (CanGet()) ? 1 : 0;
}

public int SerialRead( serial_t *serial, char *buf, int nbytes )
{
  int len = 0;

  while( len < nbytes ){
    if( CanGet() ){
      *buf++ = Axin();
      len++;
    }
    bdos( 0x0b, 0, 0 );
  }

  return len;
}

public int SerialWrite( serial_t *serial, char *buf, int nbytes )
{
  int len = 0;

  while( len < nbytes ){
    if( CanPut() ){
      Axout( *buf++ );
      len++;
    }
    bdos( 0x0b, 0, 0 );
  }

  return len;
}

public boolean_t SerialCheckData( char *data, int length )
{
  int i;

  for( i = 0 ; i < length ; i++ ){
    fprintf( stderr, "<%02x>", data[ i ] );
  }
  fprintf( stderr, "\n" );

  return TRUE;
}
