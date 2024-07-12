/*
 * link.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: link.h,v 1.4 2000/06/01 16:41:07 nrt Exp $
 */

#ifndef __LINK_H__
#define __LINK_H__

#include <serial.h>

#define SOH			0x01		/* C-a */
#define STX			0x02		/* C-b */
#define ACK			0x06		/* C-f */
#define CAN			0x18		/* C-x */

#define COM_PACKET		SOH
#define DAT_PACKET		STX
#define ACK_PACKET		ACK
#define CAN_PACKET		CAN

/*
 * $BAw<u?.$9$Y$-2s@~(B.
 */
typedef struct {
  serial_t *serial;
  unsigned char sent;			/* $B<!$KAw?.$9$Y$-%7!<%1%s%9(B. */
  unsigned char received;		/* $B$I$3$^$G<u?.$7$?$+(B. */
  FILE *flog;
} link_t;

/*
 * $B<u?.%Q%1%C%H$r3JG<$9$k9=B$BN(B.
 */
typedef struct {
  char type;
  char func[ 7 ];
  char *data;
  int  length;
} packet_t;

public boolean_t PacketFree( packet_t *packet );

public link_t *LinkOpen( char *path, char *log );
public boolean_t LinkSetRate( link_t *link, int rate );
public boolean_t LinkClose( link_t *link );

public packet_t *LinkReceivePacket( link_t *link );

public packet_t *LinkReceiveCommandPacket( link_t *link );
public packet_t *LinkReceiveDataPacket( link_t *link );
public packet_t *LinkReceiveCanPacket( link_t *link );
public boolean_t LinkReceiveAckPacket( link_t *link );

public boolean_t LinkSendCommandPacket( link_t *link,
				       char *func, char *data, int length );
public boolean_t LinkSendDataPacket( link_t *link,
				    char *func, char *data, int length );
public boolean_t LinkSendCanPacket( link_t *link, char *data );
public boolean_t LinkSendAckPacket( link_t *link );


public int Hex2Byte( char *hex );
public int Hex2Word( char *hex );

public void Byte2Hex( unsigned char ch, char *hex );
public void Word2Hex( unsigned short num, char *hex );

#endif /* __LINK_H__ */
