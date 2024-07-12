/*
 * csnoop.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: csnoop.c,v 1.5 2003/11/17 03:53:19 nrt Exp $
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
#include <signal.h>

#include <import.h>
#include <folder.h>
#include <link.h>
#include <base64.h>
#include <serial.h>
#include <begin.h>

#define BUF_SIZE 1024

static FILE *flog;
static int stopflag = 0;

void log( char *s )
{
  printf( "%s", s );
  if( NULL != flog )
    fprintf( flog, "%s", s );
}

private void SnoopExit( int code )
{
  if( NULL != flog )
    fclose( flog );
  exit( code );
}

private void termRequest( int sig )
{
   signal( sig, SIG_DFL );
   stopflag = 1;
}

int main( int argc, char **argv )
{
  link_t *caleidLink, *hostLink;
  char *caleidDev, *hostDev;
  packet_t *packet;
  char *logfile;
  boolean_t flagClosing = FALSE;
  boolean_t flagSetup = FALSE;
  int rate = 9600, active;
  char buf[ BUF_SIZE ];

  logfile = "csnoop.log";

  if( NULL == (flog = fopen( logfile, "w" )) )
    perror( logfile ), exit( 1 );

  caleidDev = "/dev/cuaa0";
  hostDev = "/dev/cuaa1";
  caleidDev = "/dev/ttyS0";
  hostDev = "/dev/ttyS1";

  if( NULL == (caleidLink = LinkOpen( caleidDev, NULL )) )
    fprintf( stderr, "cannot open %s\n", caleidDev ), exit( 1 );
  if( NULL == (hostLink = LinkOpen( hostDev, NULL )) )
    fprintf( stderr, "cannot open %s\n", hostDev ), exit( 1 );

  signal( SIGINT, termRequest );
  signal( SIGTERM, termRequest );
  printf( "snooping...\n" );

  while ( stopflag == 0 ){
    active = SerialProbe2( caleidLink->serial, hostLink->serial );
    if( active == 1 ){
      if( NULL == (packet = LinkReceivePacket( caleidLink )) )
	fprintf( stderr, "Caleid, receive error\n" ), exit( 1 );
      switch( packet->type ){
      case COM_PACKET:
	log( "C>H: COM [" );
	log( packet->func );
	log( "] \"" );
	log( packet->data );
	log( "\"\n" );
	LinkSendCommandPacket( hostLink,
			      packet->func,
			      packet->data,
			      packet->length );
	if( !strncmp( packet->data, "0900", 4 ) ){
	  flagSetup = TRUE;
	}
	break;
      case DAT_PACKET:
	log( "C>H: DAT [" );
	log( packet->func );
	log( "] \"" );
	if( EqualID( packet->func, "102001" )
	   || EqualID( packet->func, "102000" ) ){
	  EncodeBase64( packet->data, packet->length, buf );
	  log( buf );
	} else {
	  log( packet->data );
	}
	log( "\"\n" );
	LinkSendDataPacket( hostLink,
			   packet->func,
			   packet->data,
			   packet->length );
	break;
      case ACK_PACKET:
	if( flagSetup ){
	  LinkSetRate( caleidLink, rate );
	  LinkSetRate( hostLink, rate );
	  flagSetup = FALSE;
	}
	log( "C>H: ACK\n" );
	LinkSendAckPacket( hostLink );
	if( flagClosing )
	  SnoopExit( 0 );
	break;
      case CAN_PACKET:
        log( "C>H: CAN [" );
        log( packet->func );
        log( "]\n" );
        LinkSendCanPacket( hostLink, packet->func );
        if( !strncmp( packet->func, "0100", 4 ) ) {
	  flagClosing = TRUE;
        }
        if( !strncmp( packet->func, "0001", 4 ) ) {
          flagClosing = TRUE;
        }
        if( !strncmp( packet->func, "0101", 4 ) ) {
          flagClosing = TRUE;
        }
        if ( NULL != flog )
          fflush( flog );
	break;
      default:
	fprintf( stderr, "what packet?\n" ), exit( 1 );
      }
    }
    if( active == 2 ){
      if( NULL == (packet = LinkReceivePacket( hostLink )) )
	fprintf( stderr, "Caleid, receive error\n" ), exit( 1 );
      switch( packet->type ){
      case COM_PACKET:
	log( "H>C: COM [" );
	log( packet->func );
	log( "] \"" );
	log( packet->data );
	log( "\"\n" );
	LinkSendCommandPacket( caleidLink,
			      packet->func,
			      packet->data,
			      packet->length );
	if( !strncmp( packet->data, "0900", 4 ) ){
	  flagSetup = TRUE;
	}
	break;
      case DAT_PACKET:
	log( "H>C: DAT [" );
	log( packet->func );
	log( "] \"" );
	if( EqualID( packet->func, "102001" )
	   || EqualID( packet->func, "102000" ) ){
	  EncodeBase64( packet->data, packet->length, buf );
	  log( buf );
	} else {
	  log( packet->data );
	}
	log( "\"\n" );
	LinkSendDataPacket( caleidLink,
			   packet->func,
			   packet->data,
			   packet->length );
	break;
      case ACK_PACKET:
	if( flagSetup ){
	  LinkSetRate( caleidLink, rate );
	  LinkSetRate( hostLink, rate );
	  flagSetup = FALSE;
	}
	log( "H>C: ACK\n" );
	LinkSendAckPacket( caleidLink );
	if( flagClosing )
	  SnoopExit( 0 );
	break;
      case CAN_PACKET:
        log( "H>C: CAN [" );
        log( packet->func );
        log( "]\n" );
        LinkSendCanPacket( caleidLink, packet->func );
        if( !strncmp( packet->func, "0100", 4 ) ) {
          flagClosing = TRUE;
        }
        if( !strncmp( packet->func, "0001", 4 ) ) {
          flagClosing = TRUE;
        }
        if( !strncmp( packet->func, "0101", 4 ) ) {
	  flagClosing = TRUE;
        }
        if ( NULL != flog )
          fflush( flog );
	break;
      default:
	fprintf( stderr, "what packet?\n" ), exit( 1 );
      }
    }
  }

  exit( 0 );
}
