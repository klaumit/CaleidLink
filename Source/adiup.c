/*
 * adiup.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: adiup.c,v 1.12 2001/01/31 04:58:29 nrt Exp $
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
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include <import.h>
#include <protocol.h>
#include <session.h>
#include <begin.h>

private boolean_t UploadAdin( char *dev, long baud, char *file )
{
  session_t *session;
  FILE *fp;
  struct stat sb;

  stat( file, &sb );

  if( NULL == (session = SessionAlloc( dev, baud, NULL )) ){
    fprintf( stderr, "adiup: cannot open link (%s)\n", dev );
    return FALSE;
  }

  if( NULL == (fp = fopen( file, "rb" )) ){
    perror( file );
    SessionFree( session );
    return FALSE;
  }

  printf( "Ready to Send\n" );

  if( FALSE == SessionPreamble( session, NULL ) ){
    fprintf( stderr, "adiup: initial hand-shake failed\n" );
    SessionFree( session );
    return FALSE;
  }

  if( FALSE == SessionUpload( session,
			     COM_DESIGNATE_NEW_OBJECT_FOR_SEND,
			     CAT_ADDIN NEW_OBJECT_ID,
			     DAT_ADDIN, DAT_ADDIN_LAST,
			     fp, "Sent",
			     (long)sb.st_size ) ){
    fprintf( stderr, "adiup: upload failed\n" );
    return FALSE;
  }

  /* send can */
  LinkSendCanPacket( session->link, "0100" );

  /* receive ack */
/*
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    fprintf( stderr, "adiup: receive ack failed for can\n" );
*/

  fclose( fp );

  SessionFree( session );

  printf( "\nSending done: %s\n", file );

  return TRUE;
}

private void Usage()
{
  printf( "\n"
	 "# adiup $Id: adiup.c,v 1.12 2001/01/31 04:58:29 nrt Exp $"
#ifdef ACTIVE_SYNC
	 " (ActiveSync)"
#else
	 " (PassiveSync)"
#endif
	 "\n"
	 "# All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio <nrt@ff.iij4u.or.jp>\n"
	 "# ABSOLUTELY NO WARRANTY\n"
	 "\n"
	 "Usage: adiup [-a|-p] [-dDEVICE] [-sSPEED] file_name\n" );
  exit( 0 );
}

int main( int argc, char **argv )
{
  char *dev = "aux";
  char *file = NULL;
  long speed = 38400;

#ifdef ACTIVE_SYNC
  active_sync = TRUE;
#else
  active_sync = FALSE;
#endif

  while( *(++argv) ){
    if( '-' != **argv ){
      if( NULL == file )
	file = *argv;
    } else {
      switch( *( *argv + 1 ) ){
      case 'a':
	active_sync = TRUE; break;
      case 'p':
	active_sync = FALSE; break;
      case 'd':
	if( NULL == *( *argv + 2 ) ){
	  fprintf( stderr, "adiup: argument required\n" );
	  exit( 1 );
	}
	dev = *argv + 2; break;
      case 's':
	if( NULL == *( *argv + 2 ) ){
	  fprintf( stderr, "adiup: argument required\n" );
	  exit( 1 );
	}
	speed = atol( *argv + 2 ); break;
      case 'v':
      case 'h':
	Usage();
      default:
	fprintf( stderr, "adiup: unknown option %s\n", *argv );
	exit( 1 );
      }
    }
  }

  switch( speed ){
  case 4800:
  case 9600:
  case 19200:
  case 38400:
    break;
  default:
    fprintf( stderr, "adiup: invalid speed %ldbps. 38400bps was selected\n",
	    speed );
    speed = 38400;
  }

  if( NULL == file )
    Usage();

  UploadAdin( dev, speed, file );

  exit( 0 );
}
