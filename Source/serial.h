/*
 * serial.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: serial.h,v 1.5 2003/11/17 03:53:19 nrt Exp $
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#ifndef MSDOS
#include <termios.h>
#endif /* MSDOS */

typedef struct {
  char *dev;
  int fd;
#ifndef MSDOS
  struct termios ttyOld;
  struct termios ttyNew;
#endif /* MSDOS */
} serial_t;

public serial_t *SerialOpen( char *path );
public boolean_t SerialSetRate( serial_t *serial, unsigned int rate );
public boolean_t SerialClose( serial_t *serial );

public boolean_t SerialProbe( serial_t *serial );
public int SerialProbe2( serial_t *serial1, serial_t *serial2 );

public int SerialRead( serial_t *serial, char *buf, int nbytes );
public int SerialWrite( serial_t *serial, char *buf, int nbytes );

public boolean_t SerialCheckData( char *data, int length );

public boolean_t active_sync;

#endif /* __SERIAL_H__ */
