/*
 * session.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: session.h,v 1.9 2003/11/17 03:53:19 nrt Exp $
 */

#ifndef __SESSION_H__
#define __SESSION_H__

#include <link.h>
#include <folder.h>

/*
 * セッション構造体.
 */
typedef struct {
  char id[ 40 ];		/* セッション ID */
  link_t *link;
  long baud;
  boolean_t consistent;
  enum { PV_S250, PV_S1600, PV_750 } pvtype;
} session_t;

public session_t *SessionAlloc( char *dev, long baud, char *log );
public boolean_t SessionFree( session_t *session );

public boolean_t SessionPreamble( session_t *session, char *id );

public boolean_t SessionDownload( session_t *session,
				 char *func, char *did,
				 FILE *fp, char *heading,
				 long flash_size );
public boolean_t SessionUpload( session_t *session,
			       char *func, char *did,
			       char *dataID, char *lastDataID,
			       FILE *fp, char *heading,
			       long flash_size );

#endif /* __SESSION_H__ */
