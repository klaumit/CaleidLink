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
 * 送受信すべき回線.
 */
typedef struct {
  serial_t *serial;
  unsigned char sent;			/* 次に送信すべきシーケンス. */
  unsigned char received;		/* どこまで受信したか. */
  FILE *flog;
} link_t;

/*
 * 受信パケットを格納する構造体.
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
