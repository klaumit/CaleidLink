/*
 * caleidlink.c
 * All rights reserved. Copyright (C) 1998,2000 by NARITA Tomio.
 * $Id: caleidlink.c,v 1.18 2003/11/17 05:11:08 nrt Exp $
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
#include <unistd.h>

#include <import.h>
#include <version.h>
#ifdef INTERNAL_ENCODING
#include <ctable.h>
#endif
#include <protocol.h>
#include <session.h>
#include <sync.h>
#include <show.h>
#include <folder.h>
#include <util.h>
#include <begin.h>

#define BUF_SIZE	128

#define SHOW_FOLDERS	0
#define SYNCHRONIZE	1
#define BACKUP		2
#define RESTORE		3

#define DEVICE		"/dev/ttyS0"

private long flash_size = 2 * 1024 * 1024;

private boolean_t Backup( char *dev, long baud, char *file )
{
  session_t *session;
  FILE *fp;

  if( NULL == (session = SessionAlloc( dev, baud, NULL )) ){
    fprintf( stderr, "caleidlink: cannot open link (%s)\n", dev );
    return FALSE;
  }

  if( Exist( file ) || NULL == (fp = fopen( file, "w" )) ){
    perror( file );
    SessionFree( session );
    return FALSE;
  }

  printf( "Ready to Backup\n" );

  if( FALSE == SessionPreamble( session, NULL ) ){
    fprintf( stderr, "caleidlink: initial hand-shake failed\n" );
    SessionFree( session );
    return FALSE;
  }

  printf( "Starting: Backup\n" );

  if( FALSE == SessionDownload( session,
			       COM_DESIGNATE_OBJECT_FOR_GET, "F00000000000",
			       fp, "Backup",
			       flash_size) ){
    fprintf( stderr, "caleidlink: download failed\n" );
    SessionFree( session );
    return FALSE;
  }

  /* send can */
  LinkSendCanPacket( session->link, "0100" );

  /* receive ack */
/*
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    fprintf( stderr, "caleidlink: receive ack for can failed\n" );
*/
  /*
   * XXX!
   *   Instead above, we need some wait for transmission of CAN packet.
   */
  Wait100msec();
  Wait100msec();

  fclose( fp );

  SessionFree( session );

  printf( "\nDone: Backup\n" );

  return TRUE;
}

private boolean_t Restore( char *dev, long baud, char *file )
{
  session_t *session;
  FILE *fp;

  if( NULL == (session = SessionAlloc( dev, baud, NULL )) ){
    fprintf( stderr, "caleidlink: cannot open link (%s)\n", dev );
    return FALSE;
  }

  if( NULL == (fp = fopen( file, "r" )) ){
    perror( file );
    SessionFree( session );
    return FALSE;
  }

  printf( "Ready to Restore\n" );

  if( FALSE == SessionPreamble( session, NULL ) ){
    fprintf( stderr, "caleidlink: initial hand-shake failed\n" );
    SessionFree( session );
    return FALSE;
  }

  printf( "Starting: Restore\n" );

  if( FALSE == SessionUpload( session,
			     COM_DESIGNATE_NEW_OBJECT_FOR_SEND, "F00000FFFFFF",
			     DAT_BACKUP, DAT_BACKUP_LAST,
			     fp, "Restore",
			     flash_size ) ){
    fprintf( stderr, "caleidlink: upload failed\n" );
    return FALSE;
  }

  /* send can */
  LinkSendCanPacket( session->link, "0100" );

  /* receive ack */
/*
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    fprintf( stderr, "caleidlink: receive ack failed for can\n" );
*/
  /*
   * XXX
   */
  Wait100msec();
  Wait100msec();

  fclose( fp );

  SessionFree( session );

  printf( "\nDone: Restore\n" );

  return TRUE;
}

private void Banner()
{
  printf( "\n"
#ifdef PVLINK
	 "# pvlink "
#else
	 "# caleidlink "
#endif
	 VERSION
#ifdef ACTIVE_SYNC
	 " (ActiveSync"
#else
	 " (PassiveSync"
#endif
	  ", %dMB Flash)"
	 "\n"
	 "# Copyright (C) 1998,2003 by NARITA Tomio <nrt@ff.iij4u.or.jp>\n"
	 "# All rights reserved. ABSOLUTELY NO WARRANTY!\n"
	 "\n",
	 FLASH_SIZE );
}

private void Usage()
{
  Banner();

  fprintf( stderr,
	  "Usage: "
#ifdef PVLINK
	  "pvlink"
#else
	  "caleidlink"
#endif
	  " [-a|-p] [-f]\n"
	  "             [-dDEVICE] [-sSPEED] [-cCODE] [-tDELIMITER] [-lLOG]\n"
	  "             [-S|-B|-R] folder [category] [object]\n"
	  "Options:\n"
	  "\t-S: Synchronize\n"
	  "\t-B: Backup\n"
	  "\t-R: Restore\n"
	  "\n"
	  "\t-a: active synchronization\n"
	  "\t-p: passive synchronization\n"
	  "\t-f: force to synchronize, backup and restore,\n"
	  "\t    and protect consistent synchronization\n"
	  "\n"
	  "\t-dDEVICE (default: -d" DEVICE ") Serial device name\n"
	  "\t-sSPEED (default: -s38400) 4800,9600,19200,38400bps\n"
#ifdef INTERNAL_ENCODING
	  "\t-cSYSTEM (default: -ci) Coding system\n"
	  "\t  [i|e|s] i:iso-2022-jp, e:euc-japan, s:shift-jis\n"
#endif
	  "\t-tDELIMITER (default: 44 = comma) Decimal number of the delimiter\n"
	  "\t-lLOG Logging file name\n"
          "Set the environment variable \"PV_TYPE\" to \"PV-750\" if you have "
                                                               "this machine.\n"
	  "\n" );
  exit( 1 );
}

int main( int argc, char **argv )
{
  char *dev, *folder, *category, *object;
  int role = SHOW_FOLDERS;
  long speed;
  boolean_t flagForce = FALSE;
#ifdef INTERNAL_ENCODING
  char codingSystem = ISO_2022_JP;
#else
  char codingSystem = 0;
#endif
  char delimiter = ',';
  char buf[ 128 ];
  char *log = NULL;

  dev = DEVICE;
  folder = NULL;
  category = NULL;
  object = NULL;
  speed = 38400;

  flash_size = FLASH_SIZE * 1024 * 1024;

#ifdef ACTIVE_SYNC
  active_sync = TRUE;
#else
  active_sync = FALSE;
#endif

  while( *(++argv) ){
    if( '-' != **argv ){
      if( NULL == folder )
	folder = *argv;
      else if( NULL == category )
	category = *argv;
      else
	object = *argv;
    } else {
      switch( *( *argv + 1 ) ){
      case 'a':
	active_sync = TRUE; break;
      case 'p':
	active_sync = FALSE; break;
      case 'f':
	flagForce = TRUE; break;
      case 'd':
	if( '\0' == *( *argv + 2 ) ){
	  fprintf( stderr, "caleidlink: argument required\n" );
	  exit( 1 );
	}
	dev = *argv + 2; break;
      case 's':
	if( '\0' == *( *argv + 2 ) ){
	  fprintf( stderr, "caleidlink: argument required\n" );
	  exit( 1 );
	}
	speed = atoi( *argv + 2 ); break;
      case 't':
	if( '\0' == *( *argv + 2 ) ){
	  fprintf( stderr, "caleidlink: argument required\n" );
	  exit( 1 );
	}
	delimiter = (char)atoi( *argv + 2 ); break;
      case 'l':
	log = *argv + 2; break;
      case 'S':
	role = SYNCHRONIZE; break;
      case 'B':
	role = BACKUP; break;
      case 'R':
	role = RESTORE; break;
#ifdef INTERNAL_ENCODING
      case 'c':
	switch( *( *argv + 2 ) ){
	case 'a': codingSystem = AUTO_SELECT; break;
	case 'e': codingSystem = EUC_JAPAN; break;
	case 'm':
	case 's': codingSystem = SHIFT_JIS; break;
	case 'i': codingSystem = ISO_2022_JP; break;
	default:
	  codingSystem = AUTO_SELECT; break;
	}
	break;
#endif
      case 'v':
      case 'h':
	Usage();
      default:
	fprintf( stderr, "caleidlink: unknown option %s\n", *argv );
	exit( 1 );
      }
    }
  }

  switch( speed ){
  case 4800:
  case 9600:
  case 19200:
  case 38400:
#if 0
  case 76800:
#endif
    break;
  default:
    fprintf( stderr, "caleidlink: invalid speed %ldbps. 38400bps was selected\n",
	    speed );
    speed = 38400;
  }

  if( NULL == folder ){
    Usage();
  }

  if( SHOW_FOLDERS == role ){
    if( FALSE == ShowFolder( folder, category, object, codingSystem, delimiter ) ){
      exit( 1 );
    } else {
      exit( 0 );
    }
  }

  Banner();

  printf( "Conditions:\n"
	 "\tDevice:\t%s\n"
	 "\tFolder:\t%s\n"
	 "\tSpeed:\t%ld\n\n",
	 dev, folder, speed );

  printf( "Just a moment...\n" );

/*  if( TRUE == flagForce ){*/
    setvbuf( stdout, NULL, _IONBF, 0 );
/*  }*/

  switch( role ){
  case SYNCHRONIZE:
    if( FALSE == Synchronize( dev, speed,
			      folder, codingSystem,
			      flagForce, log ) ){
      fprintf( stderr, "\ncaleidlink: Sorry, failed to Synchronize.\n" );
      exit( 1 );
    } else {
      printf( "Succeeded to Synchronize.\n" );
    }
    break;
  case BACKUP:
    if( FALSE == flagForce ){
      fprintf( stderr, "\n"
	      "You are going to BACKUP Caleid. Continue? [y/n]: " );
      fgets( buf, BUF_SIZE, stdin );
      if( 'y' != *buf && 'Y' != *buf ){
	fprintf( stderr, "caleidlink: BACKUP aborted\n" );
	exit( 1 );
      }
    }
    if( FALSE == Backup( dev, speed, folder ) ){
      fprintf( stderr, "\ncaleidlink: Sorry, failed to Backup.\n" );
      exit( 1 );
    } else {
      printf( "Succeeded to Backup.\n" );
    }
    break;
  case RESTORE:
    if( FALSE == flagForce ){
      fprintf( stderr, "\n"
	      "** CAUTION **\n"
	      "\n"
	      "Current data in your Caleid will be LOST by this operation.\n"
	      "Do you really want to RESTORE full of Caleid instance? [y/n]: " );
      fgets( buf, BUF_SIZE, stdin );
      if( 'y' != *buf && 'Y' != *buf ){
	fprintf( stderr, "caleidlink: RESTORE aborted\n" );
	exit( 1 );
      }
      fprintf( stderr, "\n"
	      "Have you already Reset your Caleid? [y/n]: " );
      fgets( buf, BUF_SIZE, stdin );
      if( 'y' != *buf && 'Y' != *buf ){
	fprintf( stderr, "caleidlink: RESTORE aborted\n" );
	exit( 1 );
      }
    }
    if( FALSE == Restore( dev, speed, folder ) ){
      fprintf( stderr, "\ncaleidlink: Sorry, failed to Restore.\n" );
      exit( 1 );
    } else {
      printf( "Succeeded to Restore.\n" );
    }
    break;
  }

  exit( 0 );
}
