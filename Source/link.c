/*
 * link.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: link.c,v 1.9 2003/06/11 03:37:19 nrt Exp $
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

#include <import.h>
#include <serial.h>
#include <folder.h>
#include <begin.h>
#include <link.h>

#define DATA_SIZE	1024
#define RETRY_MAX	10

/*
 * $B%Q%1%C%H$r2rJ|(B
 */
public boolean_t PacketFree( packet_t *packet )
{
  if( NULL != packet->data )
    free( packet->data );
  free( packet );

  return TRUE;
}

/*
 * $B%j%s%/$N%*!<%W%s(B, $B%/%m!<%:(B
 */
public link_t *LinkOpen( char *path, char *log )
{
  link_t *link;

  if( NULL == (link = (link_t *)malloc( sizeof( link_t ) )) )
    return NULL;

  if( NULL == (link->serial = SerialOpen( path )) ){
    free( link );
    return NULL;
  }

  link->sent = 0;
  link->received = 0xff;

  if( log ){
    if( Exist( log ) || NULL == (link->flog = fopen( log, "w" )) )
      perror( log );
  } else {
    link->flog = NULL;
  }

  return link;
}

public boolean_t LinkSetRate( link_t *link, int rate )
{
  return SerialSetRate( link->serial, rate );
}

public boolean_t LinkClose( link_t *link )
{
  SerialClose( link->serial );
  if( link->flog )
    fclose( link->flog );
  free( link );

  return TRUE;
}

/*
 * $B<u?.4X?t(B
 */

private int X2I( char c )
{
  return ( ( (c) >= '0' && (c) <= '9' ) ? ( (c) - '0' ) :
	  ( (c) >= 'A' && (c) <= 'F' ) ? ( (c) - 'A' + 0x0a ) : -1 );
}

public int Hex2Byte( char *hex )
{
  int high, low;

  if( 0 > (high = X2I( hex[ 0 ] )) || 0 > (low = X2I( hex[ 1 ] )) )
    return -1;

  return ( high << 4 ) | low;
}

public int Hex2Word( char *hex )
{
  int highH, lowH, highL, lowL;

  if( 0 > (highH = X2I( hex[ 0 ] ))
     || 0 > (lowH = X2I( hex[ 1 ] ))
     || 0 > (highL = X2I( hex[ 2 ] ))
     || 0 > (lowL = X2I( hex[ 3 ] )) )
    return -1;

  return ( highH << 12 ) | ( lowH << 8 ) | ( highL << 4 ) | lowL;
}

/*
 * $B;XDjD9$N%G!<%?$NFI$_9~$_(B.
 */
private boolean_t ReceiveData( link_t *link, char *buf, int length )
{
  int res, retries;

  retries = 0;
  while( length > 0
	&& length != (res = SerialRead( link->serial, buf, length )) ){
    if( -1 == res ){
      fprintf( stderr,
	      "caleidlink: received unexpected size packet (%d) != (%d)\n",
	      length, res );
      return FALSE;
    }
    buf += res;
    length -= res;
    if( 0 == res && ++retries > RETRY_MAX ){
/*
      fprintf( stderr,
	      "caleidlink: retry count to receive exceeded %d\n", RETRY_MAX );
*/
      return FALSE;
    }
  }

  return TRUE;
}

private unsigned char CalculateChecksum( unsigned char checksum,
					unsigned char *buf, int length )
{
  while( length > 0 ){
    checksum -= *buf;
    buf++;
    length--;
  }

  return checksum;
}

private void log( link_t *link, char *header, int len, char *func, char *data )
{
  int i;

  if( NULL == link->flog )
    return;

  fprintf( link->flog, "%s", header );

  if( data ){
    fprintf( link->flog, "[%s](%d)[", func, len );
    for( i = 0 ; i < len ; i++ ){
      if( data[ i ] >= ' ' && data[ i ] < 0x7f )
	fprintf( link->flog, "%c", data[ i ] );
      else
	fprintf( link->flog, "\\x%02x", data[ i ] );
    }
    fprintf( link->flog, "]" );
  } else if( len ){
    fprintf( link->flog, "type:%02x", len );
  }

  fprintf( link->flog, "\n" );
}

/*
 * $B%;%C%7%g%s$+$i(B 1$B%Q%1%C%HFI$_9~$_(B.
 * $B<u?.(B/$BAw?.%7!<%1%s%9$r99?7(B.
 * $B0[>o%G!<%?;~(B, NULL $B$rJV$9(B. NULL $B$O%;%C%7%g%s$N0[>o=*N;$r0UL#$9$k(B.
 */
public packet_t *LinkReceivePacket( link_t *link )
{
  packet_t *packet;
  unsigned char checksum = 0;
  unsigned char sequence;
  char buf[ DATA_SIZE ];

  if( NULL == (packet = (packet_t *)malloc( sizeof( packet_t ) )) )
    return NULL;
  packet->data = NULL;

  if( FALSE == ReceiveData( link, &packet->type, 1 ) ){
    free( packet );
    return NULL;
  }

  switch( packet->type ){
  case SOH:
    /*
     * $B%3%^%s%I%Q%1%C%H!&<u?.%7!<%1%s%9<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      free( packet );
      return NULL;
    }
    sequence = Hex2Byte( buf );
    link->received++;
    if( link->received != sequence ){
      fprintf( stderr, "caleidlink: duplicated sequence %02x\n", sequence );
      free( packet );
      return NULL;
    }
    /*
     * $B%3%^%s%I%Q%1%C%H!&%G!<%?D9<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      free( packet );
      return NULL;
    }
    packet->length = Hex2Byte( buf );
    packet->length -= 4;		/* $B%U%!%s%/%7%g%sD9$r=|$/(B. */
    checksum = CalculateChecksum( checksum, buf, 2 );
    /*
     * $B%3%^%s%I%Q%1%C%H!&%U%!%s%/%7%g%s<u?.(B.
     */
    if( FALSE == ReceiveData( link, packet->func, 4 ) ){
      free( packet );
      return NULL;
    }
    packet->func[ 4 ] = '\0';
    checksum = CalculateChecksum( checksum, packet->func, 4 );
    /*
     * $B%3%^%s%I%Q%1%C%H!&%G!<%?<u?.(B.
     */
    if( packet->length > DATA_SIZE ){
      fprintf( stderr,
	      "caleidlink: COM, data size (%d) is longer than DATA_SIZE (%d)\n",
	      packet->length,
	      DATA_SIZE );
      free( packet );
      return NULL;
    }
    if( NULL == (packet->data = (char *)malloc( packet->length + 1 )) ){
      free( packet );
      return NULL;
    }
    if( FALSE == ReceiveData( link, packet->data, packet->length ) ){
      PacketFree( packet );
      return NULL;
    }
    checksum = CalculateChecksum( checksum,
				 packet->data, packet->length );
    if( link->flog )
      log( link, "C>H: COM", packet->length, packet->func, packet->data );
    /*
     * $B%3%^%s%I%Q%1%C%H!&%A%'%C%/%5%`<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      PacketFree( packet );
      return NULL;
    }
    if( checksum != (unsigned char)Hex2Byte( buf ) ){
      fprintf( stderr,
	      "caleidlink: COM, corrupted packet, checksum (%02x) != (%02x)\n",
	      checksum,
	      (unsigned char)Hex2Byte( buf ) );
      PacketFree( packet );
      return NULL;
    }
    break;
  case STX:
    /*
     * $B%G!<%?%Q%1%C%H!&<u?.%7!<%1%s%9<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      free( packet );
      return NULL;
    }
    sequence = Hex2Byte( buf );
    link->received++;
    if( link->received != sequence ){
      fprintf( stderr, "caleidlink: duplicated sequence %02x\n", sequence );
      free( packet );
      return NULL;
    }
    /*
     * $B%G!<%?%Q%1%C%H!&%G!<%?D9<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 4 ) ){
      free( packet );
      return NULL;
    }
    packet->length = Hex2Word( buf );
    packet->length -= 6;		/* $B%U%!%s%/%7%g%sD9$r=|$/(B. */
    checksum = CalculateChecksum( checksum, buf, 4 );
    /*
     * $B%G!<%?%Q%1%C%H!&%U%!%s%/%7%g%s<u?.(B.
     */
    if( FALSE == ReceiveData( link, packet->func, 6 ) ){
      free( packet );
      return NULL;
    }
    packet->func[ 6 ] = '\0';
    checksum = CalculateChecksum( checksum, packet->func, 6 );
    /*
     * $B%G!<%?%Q%1%C%H!&%G!<%?<u?.(B.
     */
    if( packet->length > DATA_SIZE ){
      fprintf( stderr,
	      "caleidlink: DATA, data size (%d) is longer than DATA_SIZE (%d)\n",
	      packet->length,
	      DATA_SIZE );
      free( packet );
      return NULL;
    }
    if( NULL == (packet->data = (char *)malloc( packet->length )) ){
      free( packet );
      return NULL;
    }
    if( FALSE == ReceiveData( link, packet->data, packet->length ) ){
      PacketFree( packet );
      return NULL;
    }
    checksum = CalculateChecksum( checksum,
				 packet->data, packet->length );
    if( link->flog )
      log( link, "C>H: DAT", packet->length, packet->func, packet->data );
    /*
     * $B%G!<%?%Q%1%C%H!&%A%'%C%/%5%`<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      PacketFree( packet );
      return NULL;
    }
    if( checksum != (unsigned char)Hex2Byte( buf ) ){
      fprintf( stderr,
	      "caleidlink: DATA, corrupted packet, checksum (%02x) != (%02x)\n",
	      checksum,
	      (unsigned char)Hex2Byte( buf ) );
      PacketFree( packet );
      return NULL;
    }
    break;
  case CAN:
    /*
     * CAN $B%Q%1%C%H!&<u?.%7!<%1%s%9<u?.(B.
     */
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      free( packet );
      return NULL;
    }
    sequence = Hex2Byte( buf );
    link->received++;
    if (!link->received) { /* we got our first packet, accept sequence */
      link->received = sequence;
    }
    if( link->received != sequence ){
      fprintf( stderr, "caleidlink: duplicated sequence %02x\n", sequence );
      free( packet );
      return NULL;
    }
    /*
     * CAN $B%Q%1%C%H!&%U%!%s%/%7%g%s<u?.(B.
     */
    if( FALSE == ReceiveData( link, packet->func, 4 ) ){
      free( packet );
      return NULL;
    }
    packet->func[ 4 ] = '\0';
    if( link->flog )
      log( link, "C>H: CAN", 0, NULL, NULL );
    break;
  case ACK:
    /*
     * ACK $B%Q%1%C%H!&(BACK $BBP>]%7!<%1%s%9<u?.(B.
     */
    packet->data = NULL;
    if( FALSE == ReceiveData( link, buf, 2 ) ){
      free( packet );
      return NULL;
    }
    if( link->flog )
      log( link, "C>H: ACK", 0, NULL, NULL );
    sequence = Hex2Byte( buf );
    sequence++;
    if( link->sent != sequence ){
      fprintf( stderr, "caleidlink: inconsistent ACK, (%02x) != (%02x)\n",
	      link->sent,
	      sequence );
      free( packet );
      return NULL;
    }
    break;
  default:
    if( link->flog )
      log( link, "C>H: UNK", packet->type, NULL, NULL );
    fprintf( stderr, "caleidlink: unknown packet type (%02x)\n", packet->type );
    free( packet );
    return NULL;
  }

  return packet;
}

public packet_t *LinkReceiveCommandPacket( link_t *link )
{
  packet_t *packet;

  if( NULL == (packet = LinkReceivePacket( link )) )
    return NULL;

  if( COM_PACKET != packet->type ){
    PacketFree( packet );
    return NULL;
  }

  return packet;
}

public packet_t *LinkReceiveDataPacket( link_t *link )
{
  packet_t *packet;

  if( NULL == (packet = LinkReceivePacket( link )) )
    return NULL;

  if( DAT_PACKET != packet->type ){
    PacketFree( packet );
    return NULL;
  }

  return packet;
}

public packet_t *LinkReceiveCanPacket( link_t *link )
{
  packet_t *packet;

  if( NULL == (packet = LinkReceivePacket( link )) )
    return NULL;

  if( CAN_PACKET != packet->type ){
    PacketFree( packet );
    return NULL;
  }

  return packet;
}

public boolean_t LinkReceiveAckPacket( link_t *link )
{
  packet_t *packet;

  if( NULL == (packet = LinkReceivePacket( link )) )
    return FALSE;

  if( ACK_PACKET != packet->type ){
    fprintf( stderr, "caleidlink: unexpected packet type (%02x) against ACK\n", packet->type );
    PacketFree( packet );
    return FALSE;
  }

  free( packet );

  return TRUE;
}

/*
 * $BAw?.4X?t(B
 */

#define STR_SOH		"\x01"
#define STR_STX		"\x02"
#define STR_ACK		"\x06"
#define STR_CAN		"\x18"

private char *hexTable = "0123456789ABCDEF";

public void Byte2Hex( unsigned char ch, char *hex )
{
  hex[ 0 ] = hexTable[ ch >> 4 ];
  hex[ 1 ] = hexTable[ ch & 0x0f ];
}

public void Word2Hex( unsigned short num, char *hex )
{
  hex[ 0 ] = hexTable[ num >> 12 ];
  hex[ 1 ] = hexTable[ ( num & 0x0f00 ) >> 8 ];
  hex[ 2 ] = hexTable[ ( num & 0x00f0 ) >> 4 ];
  hex[ 3 ] = hexTable[ num & 0x000f ];
}

/*
 * $B;XDjD9$N%G!<%?$NAw?.(B.
 */
private boolean_t SendData( link_t *link, char *buf, int length )
{
  if( length != SerialWrite( link->serial, buf, length ) ){
    fprintf( stderr, "caleidlink: sent unexpected size packet\n" );
    exit( 2 );
  }

  return TRUE;
}

public boolean_t LinkSendCommandPacket( link_t *link,
				       char *func, char *data, int length )
{
  unsigned char checksum = 0;
  char buf[ DATA_SIZE ];

  if( link->flog )
    log( link, "H>C: COM", length, func, data );

  /*
   * $B%3%^%s%I%Q%1%C%H!&%G!<%?%?%$%WAw?.(B.
   */
  SendData( link, STR_SOH, 1 );
  /*
   * $B%3%^%s%I%Q%1%C%H!&%7!<%1%s%9Aw?.(B.
   */
  Byte2Hex( link->sent, buf );
  SendData( link, buf, 2 );
  link->sent++;
  /*
   * $B%3%^%s%I%Q%1%C%H!&%G!<%?D9Aw?.(B.
   */
  Byte2Hex( 4 + length, buf );		/* $B%U%!%s%/%7%g%sD9$rDI2C(B. */
  SendData( link, buf, 2 );
  checksum = CalculateChecksum( checksum, buf, 2 );
  /*
   * $B%3%^%s%I%Q%1%C%H!&%U%!%s%/%7%g%sAw?.(B.
   */
  SendData( link, func, 4 );
  checksum = CalculateChecksum( checksum, func, 4 );
  /*
   * $B%3%^%s%I%Q%1%C%H!&%G!<%?Aw?.(B.
   */
  if( NULL != data ){
    SendData( link, data, length );
    checksum = CalculateChecksum( checksum, data, length );
  }
  /*
   * $B%3%^%s%I%Q%1%C%H!&%A%'%C%/%5%`Aw?.(B.
   */
  Byte2Hex( checksum, buf );
  SendData( link, buf, 2 );

  return TRUE;
}

public boolean_t LinkSendDataPacket( link_t *link,
				    char *func, char *data, int length )
{
  unsigned char checksum = 0;
  char buf[ DATA_SIZE ];

  if( link->flog )
    log( link, "H>C: DAT", length, func, data );

  /*
   * $B%G!<%?%Q%1%C%H!&%G!<%?%?%$%WAw?.(B.
   */
  SendData( link, STR_STX, 1 );
  /*
   * $B%G!<%?%Q%1%C%H!&%7!<%1%s%9Aw?.(B.
   */
  Byte2Hex( link->sent, buf );
  SendData( link, buf, 2 );
  link->sent++;
  /*
   * $B%G!<%?%Q%1%C%H!&%G!<%?D9Aw?.(B.
   */
  Word2Hex( 6 + length, buf );		/* $B%U%!%s%/%7%g%sD9$rDI2C(B. */
  SendData( link, buf, 4 );
  checksum = CalculateChecksum( checksum, buf, 4 );
  /*
   * $B%G!<%?%Q%1%C%H!&%U%!%s%/%7%g%sAw?.(B.
   */
  SendData( link, func, 6 );
  checksum = CalculateChecksum( checksum, func, 6 );
  /*
   * $B%G!<%?%Q%1%C%H!&%G!<%?Aw?.(B.
   */
  SendData( link, data, length );
  checksum = CalculateChecksum( checksum, data, length );
  /*
   * $B%G!<%?%Q%1%C%H!&%A%'%C%/%5%`Aw?.(B.
   */
  Byte2Hex( checksum, buf );
  SendData( link, buf, 2 );

  return TRUE;
}

public boolean_t LinkSendAckPacket( link_t *link )
{
  char buf[ DATA_SIZE ];

  if( link->flog )
    log( link, "H>C: ACK", 0, NULL, NULL );

  /*
   * ACK $B%Q%1%C%H!&%G!<%?%?%$%WAw?.(B
   */
  SendData( link, STR_ACK, 1 );
  /*
   * ACK $B%Q%1%C%H!&(BACK $BBP>]%7!<%1%s%9Aw?.(B
   */
  Byte2Hex( link->received, buf );
  SendData( link, buf, 2 );

  return TRUE;
}

public boolean_t LinkSendCanPacket( link_t *link, char *func )
{
  char buf[ DATA_SIZE ];

  if( link->flog )
    log( link, "H>C: CAN", 0, NULL, NULL );

  /*
   * CAN $B%Q%1%C%H!&%G!<%?%?%$%WAw?.(B
   */
  SendData( link, STR_CAN, 1 );
  /*
   * CAN $B%Q%1%C%H!&%7!<%1%s%9Aw?.(B
   */
  Byte2Hex( link->sent, buf );
  SendData( link, buf, 2 );
  link->sent++;
  /*
   * CAN $B%Q%1%C%H!&%U%!%s%/%7%g%sAw?.(B.
   */
  SendData( link, func, 4 );

  return TRUE;
}
