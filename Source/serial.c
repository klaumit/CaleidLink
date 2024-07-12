/*
 * serial.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: serial.c,v 1.15 2003/11/17 03:53:19 nrt Exp $
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
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <import.h>
#include <util.h>
#include <begin.h>
#include <serial.h>

#define TMP_DIR		"/tmp"
#define BUF_SIZE	256

public boolean_t SerialSetRate( serial_t *serial, unsigned int rate )
{
  speed_t speed;

  switch( rate ){
  case 4800:
    speed = B4800;  break;
  case 19200:
    speed = B19200; break;
  case 38400:
    speed = B38400; break;
  case 9600:
  default:
    speed = B9600;
  }

  cfsetispeed( &serial->ttyNew, speed );
  cfsetospeed( &serial->ttyNew, speed );
  tcsetattr( serial->fd, TCSADRAIN, &serial->ttyNew );

  return TRUE;
}

private char *LockFilename( char *buf, int len, char *path )
{
  char *ptr;

  strcpy( buf, TMP_DIR "/clink" );
  ptr = buf + strlen( buf );
  strcpy( ptr, path );

  while( *ptr ){
    if( '/' == *ptr )
      *ptr = '.';
    ptr++;
  }

  return buf;
}

private boolean_t SerialLock( char *path )
{
  int fd, done, pid;
  char pidbuf[ 20 ];
  char buf[ BUF_SIZE ];

  LockFilename( buf, BUF_SIZE, path );

  if( -1 != ( fd = open( buf, O_RDONLY ) ) ) {
    done = read( fd, pidbuf, 19 - 1 );
    if ( done > 0 ) {
      pidbuf[done] = '\0';
      pid = atoi( pidbuf );
      if ( pid > 0 )  {
         if ( kill( pid, 0 ) != 0 ) {
            /*
             * parent of lock is dead
             */
            if ( fchmod( fd, 0666 ) == 0 )
              unlink( buf );
         }
      }
    }
    close( fd );
  }

  if( 0 > (fd = open( buf, O_RDWR | O_CREAT | O_EXCL, 0444 )) ){
    fprintf( stderr, "lock file exists: %s\n", buf );
    return FALSE;
  }

  sprintf( buf, "%d\n", (int)getpid() );
  write( fd, buf, strlen( buf ) );

  return TRUE;
}

private boolean_t SerialUnlock( char *path )
{
  char buf[ BUF_SIZE ];

  LockFilename( buf, BUF_SIZE, path );

  unlink( buf );

  return TRUE;
}

private serial_t *current_serial;

private void CloseHandler( int arg )
{
  SerialClose( current_serial );

  printf( "\ncaleidlink: software termination signal from kill.\n" );

  exit( arg );
}

private boolean_t StatusGet( int fd, int *status )
{
  if( 0 > ioctl( fd, TIOCMGET, status ) ){
    fprintf( stderr, "ioctl get status failed\n" );
    return FALSE;
  }
  return TRUE;
}

private boolean_t StatusSet( int fd, int *status )
{
  if( 0 > ioctl( fd, TIOCMSET, status ) ){
    fprintf( stderr, "ioctl set status failed\n" );
    return FALSE;
  }
  return TRUE;
}

public serial_t *SerialOpen( char *path )
{
  serial_t *serial;

  if( NULL == (serial = (serial_t *)malloc( sizeof( serial_t ) )) )
    return NULL;

  if( FALSE == SerialLock( path ) )
    return NULL;

  /*
   */
  serial->dev = malloc( strlen( path ) + 1 );
  strcpy( serial->dev, path );

  current_serial = serial;
  signal( SIGINT, CloseHandler );
  signal( SIGTERM, CloseHandler );

#ifdef __NetBSD__
  if( 0 > (serial->fd = open( path, O_RDWR|O_NONBLOCK )) ){
    free( serial );
    return NULL;
  }
  if( 0 > fcntl( serial->fd, F_SETFL,
		 ~O_NONBLOCK & fcntl( serial->fd, F_GETFL, NULL ) ) ){
    close( serial->fd );
    free( serial );
    return NULL;
  }
#else
  if( 0 > (serial->fd = open( path, O_RDWR )) ){
    free( serial );
    return NULL;
  }
#endif

  tcgetattr( serial->fd, &serial->ttyOld );

  serial->ttyNew = serial->ttyOld;		/* 構造体の代入 */

  serial->ttyNew.c_iflag = IXON | IXOFF;
  serial->ttyNew.c_oflag = 0;
  serial->ttyNew.c_cflag = CS8 | CREAD | CLOCAL | CRTSCTS;
  serial->ttyNew.c_lflag = 0;
  serial->ttyNew.c_cc[ VMIN ] = 0;
  serial->ttyNew.c_cc[ VTIME ] = 10;	/* timeout 1.0 sec */

  cfsetispeed( &serial->ttyNew, B9600 );
  cfsetospeed( &serial->ttyNew, B9600 );

  tcsetattr( serial->fd, TCSADRAIN, &serial->ttyNew );

  if( TRUE == active_sync ){
    /*
     * Wake up, Caleid!
     */
    int status;

    /*
     * なぜか Sun Sparc Ultra-2 の /dev/cua/{a|b} では以下の DTR の切り換えに
     * それぞれ 3秒ほどかかってしまい, タイミングがずれてカレイド君が起きて
     * くれません. これがクリアできれば問題解決なのですが...
     */
    StatusGet( serial->fd, &status );

#define StatusSetDTR( fd ){ status |= TIOCM_DTR; StatusSet( (fd), &status ); }
#define StatusClearDTR( fd ){ status &= ~TIOCM_DTR; StatusSet( (fd), &status ); }

    StatusClearDTR( serial->fd );	/* DTR OFF */
    Wait100msec();
    StatusSetDTR( serial->fd );		/* DTR ON */
    Wait100msec();

    StatusClearDTR( serial->fd );	/* DTR OFF */
    Wait100msec();
    StatusSetDTR( serial->fd );		/* DTR ON */
    Wait100msec();

    StatusClearDTR( serial->fd );	/* DTR OFF */
    Wait100msec();
    StatusSetDTR( serial->fd );		/* DTR ON */
  }

  return serial;
}

public boolean_t SerialClose( serial_t *serial )
{
  tcsetattr( serial->fd, TCSADRAIN, &serial->ttyOld );

  close( serial->fd );
  SerialUnlock( serial->dev );
  free( serial->dev );

  free( serial );

  return TRUE;
}

public boolean_t SerialProbe( serial_t *serial )
{
  fd_set fdset;
  struct timeval tv;

  FD_ZERO( &fdset );
  FD_SET( serial->fd, &fdset );
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  select( 1 + serial->fd, &fdset, NULL, NULL, &tv );

  if( FD_ISSET( serial->fd, &fdset ) )
    return TRUE;
  else
    return FALSE;
}

public int SerialProbe2( serial_t *serial1, serial_t *serial2 )
{
  int max;
  fd_set fdset;
  struct timeval tv;

  FD_ZERO( &fdset );
  FD_SET( serial1->fd, &fdset );
  FD_SET( serial2->fd, &fdset );
  tv.tv_sec = 3600;
  tv.tv_usec = 0;

  if ( serial1->fd > serial2->fd )
    max = serial1->fd + 1;
  else
    max = serial2->fd + 1;
  select( max, &fdset, NULL, NULL, &tv );

  if( FD_ISSET( serial1->fd, &fdset ) )
    return 1;
  if( FD_ISSET( serial2->fd, &fdset ) )
    return 2;
  return 0;
}

public int SerialRead( serial_t *serial, char *buf, int nbytes )
{
  return read( serial->fd, buf, nbytes );
}

public int SerialWrite( serial_t *serial, char *buf, int nbytes )
{
  return write( serial->fd, buf, nbytes );
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

