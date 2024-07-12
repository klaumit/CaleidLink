/*
 * session.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: session.c,v 1.10 2003/11/17 03:53:19 nrt Exp $
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

#include <import.h>
#include <protocol.h>
#include <link.h>
#include <util.h>
#include <begin.h>
#include <session.h>

#define BUF_SIZE	1024

public boolean_t SessionPreamble( session_t *session, char *id )
{
  packet_t *packet;
  char buf[ BUF_SIZE ];
  char *overwrite;
  
  if( NULL != id )
    strncpy( session->id + 16, id, 24 );

  while( NULL == (packet = LinkReceivePacket( session->link )) ){
    /* do nothing */
  }

  if( CAN_PACKET != packet->type || !EqualFunc( packet->func, "0000" ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_CALEID_PDA_ID, packet->func ) ) {
    PacketFree( packet );
    return FALSE;
  }
  if( strncmp( packet->data, CALEID_ID, 8 ) == 0 ) {
    session->pvtype = PV_S250; /* and compatible */
  } else if ( strncmp( packet->data, CALEID_ID1600, 8 ) == 0 ) {
    session->pvtype = PV_S1600;
  } else {
  PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  overwrite = getenv("PV_TYPE");
  if ( overwrite != NULL ) {
    if ( strcmp( overwrite, "PV-750" ) == 0 )
      session->pvtype = PV_750;
    if ( strcmp( overwrite, "PV-S250" ) == 0 )
      session->pvtype = PV_S250;
    if ( strcmp( overwrite, "PV-S1600" ) == 0 )
      session->pvtype = PV_S1600;
  }
  printf( "Type = %s\n",
          ( session->pvtype == PV_S250 )  ? "PV-S250 (compatible)" :
          ( session->pvtype == PV_S1600 ) ? "PV-S1600" :
          ( session->pvtype == PV_750 )   ? "PV-750 (compatible)" :
                                            "unknown" );

  /* send ack */
  LinkSendAckPacket( session->link );

  /* send com */
  LinkSendCommandPacket( session->link,
			COM_CALEID_HOST_ID,
                        ( session->pvtype == PV_S1600 ) ? CALEID_ID1600 :
                        ( session->pvtype == PV_750 ) ? CALEID_ID750 :
                                                        CALEID_ID,
			8 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_PDA_SYNC_ID, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  if( NULL != id && !strncmp( packet->data + 16, id, 24 ) )
    session->consistent = TRUE;
  else
    session->consistent = FALSE;
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /* send com */
  LinkSendCommandPacket( session->link, COM_HOST_SYNC_ID, session->id, 40 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_AVAILABLE_SPEED, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  switch( session->baud ){
    /* XM500EM    001011010101 */
  case 4800:
    strcpy( buf, "000000000001" ); break;
  case 19200:
    strcpy( buf, "000000010000" ); break;
  case 38400:
    strcpy( buf, "000001000000" ); break;
  case 9600:
  default:
    strcpy( buf, "000000000100" ); break;
  }

  /* send com */
  LinkSendCommandPacket( session->link, COM_SELECT_SPEED, buf, 12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_CLOSE_PREAMBLE, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /* send com */
  LinkSendCommandPacket( session->link, COM_START_SESSION, "0000", 4 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  LinkSetRate( session->link, session->baud );

  Wait100msec();
  Wait100msec();

  return TRUE;
}

/**********************************************************************/

public session_t *SessionAlloc( char *dev, long baud, char *log )
{
  session_t *session;

  if( NULL == (session = (session_t *)malloc( sizeof( session_t ) )) )
    return NULL;

  if( NULL == (session->link = LinkOpen( dev, log )) ){
    free( session );
    return NULL;
  }

  /*
   * backup/restore 時の session-id (40bytes)
   */
  strncpy( session->id, INIT_SESSION_ID, 40 );

  session->baud = baud;
  session->consistent = FALSE;

  return session;
}

public boolean_t SessionFree( session_t *session )
{
  LinkClose( session->link );
  free( session );

  return TRUE;
}

/**********************************************************************/

public boolean_t SessionDownload( session_t *session,
				 char *func, char *did,
				 FILE *fp, char *heading,
				 long flash_size )
{
  packet_t *packet;
  int length;
  long totalLength = 0;
  char buf[ BUF_SIZE ];

  /* send com */
  LinkSendCommandPacket( session->link,
			func,
			did,
			12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) ){
    fprintf( stderr, "caleidlink: receive ack for designation failed\n" );
    SessionFree( session );
    return FALSE;
  }

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) ){
    fprintf( stderr, "caleidlink: receive com packet failed\n" );
    SessionFree( session );
    return FALSE;
  }
  if( FALSE == EqualFunc( COM_START_DATA, packet->func ) ){
    PacketFree( packet );
    fprintf( stderr, "caleidlink: not equal COM_START_DATA\n" );
    SessionFree( session );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  for( ; ; ){
    if( NULL == (packet = LinkReceivePacket( session->link )) ){
      fprintf( stderr, "caleidlink: receive packet failed\n" );
      SessionFree( session );
      return FALSE;
    }
    if( DAT_PACKET == packet->type ){
      /*
       * データの書き出し.
       */
      length = Shifted2Bin( packet->data, packet->length, buf );
      totalLength += length;
      if( length != fwrite( buf, 1, length, fp ) ){
	fprintf( stderr, "caleidlink: fwrite failed\n" );
	SessionFree( session );
	return FALSE;
      }
      PacketFree( packet );
      printf( "\r%s: %ld(%ld%%) ", heading,
	      totalLength, 100 * totalLength / flash_size );
    } else if( COM_PACKET == packet->type ){
      fflush( fp );

      if( EqualFunc( packet->func, COM_END_OBJECT_FOR_GET ) ){
	PacketFree( packet );
	break;
      }
      PacketFree( packet );

      /* send ack */
      LinkSendAckPacket( session->link );
    } else if( ACK_PACKET == packet->type ){
      PacketFree( packet );
    } else {
      fprintf( stderr, "caleidlink: unexpected packet (%02x)\n", packet->type );
      SessionFree( session );
      return FALSE;
    }
  }

  /* send ack */
  LinkSendAckPacket( session->link );

  return TRUE;
}

/**********************************************************************/

public boolean_t SessionUpload( session_t *session,
			       char *func, char *did,
			       char *dataID, char *lastDataID,
			       FILE *fp, char *heading,
			       long flash_size )
{
  packet_t *packet;
  int length, blockLength;
  int count;
  long totalLength = 0;
  char buf[ BUF_SIZE ], cont[ BUF_SIZE ];

  /* send com */
  LinkSendCommandPacket( session->link,
			func,
			did,
			12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) ){
    fprintf( stderr, "caleidlink: receive ack for designation failed\n" );
    SessionFree( session );
    return FALSE;
  }

  /* send com */
  LinkSendCommandPacket( session->link,
			COM_START_DATA,
			NULL,
			0 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) ){
    fprintf( stderr, "caleidlink: receive ack for start failed\n" );
    SessionFree( session );
    return FALSE;
  }

  blockLength = 512;
  for( count = 0 ; ; ){
    if( 0 == (length = fread( buf, 1, blockLength, fp )) )
      break;
    totalLength += length;
    printf( "\r%s: %ld(%ld%%) ", heading,
	    totalLength, 100 * totalLength / flash_size );
    length = Bin2Shifted( buf, length, cont );
    if( length < blockLength ){
      LinkSendDataPacket( session->link,
			 lastDataID,
			 cont,
			 length );
      break;
    } else {
      LinkSendDataPacket( session->link,
			 dataID,
			 cont,
			 length );
    }
    count++;
    if( 0 == count % 12 ){
      count = 1;

      /* send com */
      LinkSendCommandPacket( session->link, COM_DATA_LAP, NULL, 0 );

      /* receive ack */
      if( FALSE == LinkReceiveAckPacket( session->link ) ){
	fprintf( stderr, "caleidlink: receive ack for lap failed\n" );
	SessionFree( session );
	return FALSE;
      }
    }
  }

  /* send com */
  LinkSendCommandPacket( session->link,
			COM_END_OBJECT_FOR_SEND,
			NULL,
			0 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) ){
    fprintf( stderr, "caleidlink: receive ack for end failed\n" );
    SessionFree( session );
    return FALSE;
  }

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) ){
    fprintf( stderr, "caleidlink: receive com packet failed\n" );
    SessionFree( session );
    return FALSE;
  }
  if( FALSE == EqualFunc( COM_STORED_OBJECT, packet->func )
     && FALSE == EqualFunc( COM_STORED_NEW_OBJECT, packet->func ) ){
    fprintf( stderr, "caleidlink: unexpected function (%s)\n", packet->func );
    PacketFree( packet );
    SessionFree( session );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  return TRUE;
}
