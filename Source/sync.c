/*
 * sync.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: sync.c,v 1.18 2003/06/11 03:13:50 nrt Exp $
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
#include <time.h>

#include <import.h>
#include <protocol.h>
#include <session.h>
#include <util.h>
#include <begin.h>
#include <sync.h>

#define BUF_SIZE	1024

/*
#define DEBUG
*/

private boolean_t EqualDataFunc( char *f1, char *f2 )
{
  if( !strncmp( f1, f2, 6 ) )
    return TRUE;
  else
    return FALSE;
}

/**********************************************************************/

/*
 * $B<u?.MQ$K%*%V%8%'%/%H$r;X<($9$k(B.
 * $B;X<($N%3%^%s%I$O0z?t$GM?$($k(B.
 */
private boolean_t DesignateObject( session_t *session, char *func, char *did )
{
  packet_t *packet;

  /*
   * $B%*%V%8%'%/%H$N;X<((B
   */
  LinkSendCommandPacket( session->link, func, did, 12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( FALSE == EqualFunc( COM_START_DATA, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  return TRUE;
}

/*
 * $BAw?.MQ$K%*%V%8%'%/%H$r;X<($9$k(B.
 * $B;X<($N%3%^%s%I$O0z?t$GM?$($k(B.
 */
private boolean_t DesignateObjectForSend( session_t *session,
					 char *func, char *did )
{
  /*
   * $B%*%V%8%'%/%H$N;X<((B
   */
  LinkSendCommandPacket( session->link, func, did, 12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  return TRUE;
}

/**********************************************************************/

/*
 * $B%*%V%8%'%/%H$r<u?.$7$F(B, $B9=B$BN$H$7$FJV$9(B.
 * $B$3$N4X?t$O%*%V%8%'%/%H$r;X<($7$F$+$i8F$V(B.
 */
private object_t *ReceiveObjectData( session_t *session,
				    char *did, char *file )
{
  object_t *object;
  content_t *contentsRoot, *contentsLast = NULL, *content;
  packet_t *packet;

  if( NULL == (object = ObjectAlloc( did + 6, file, 0 )) )
    return NULL;

  printf( "Receiving: %s\n", object->file );

  /* receive */
  for( contentsRoot = NULL ; ; ){
    if( NULL == (packet = LinkReceivePacket( session->link )) ){
      fprintf( stderr, "caleidlink: cannot receive packet\n" );
      break;
    }
    if( DAT_PACKET == packet->type ){
      if( NULL == (content = ContentAlloc( packet->func,
					  packet->data,
					  packet->length )) ){
	fprintf( stderr, "ReceiveObjectData(): cannot alloc content\n" );
	PacketFree( packet );
	continue;
      }
      free( packet );	/* data $B$r;D$9$?$a(B, PacketFree() $B$G$O$J$$(B */
      if( NULL == contentsRoot ){
	contentsRoot = content;
	contentsLast = content;
      } else {
	contentsLast->next = content;
	contentsLast = content;
      }
    } else if( COM_PACKET == packet->type
	      && EqualFunc( COM_DATA_LAP, packet->func ) ){
      PacketFree( packet );

      /* send ack */
      LinkSendAckPacket( session->link );
    } else if( COM_PACKET == packet->type
	      && EqualFunc( COM_END_DATA, packet->func ) ){
      PacketFree( packet );
      break;
    } else {
      fprintf( stderr,
	      "caleidlink: unexpected packet type (%02x) while receiving object\n", packet->type );
      PacketFree( packet );
      break;
    }
  }

  object->contents = contentsRoot;

  /* send ack */
  LinkSendAckPacket( session->link );

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) ){
    ObjectFree( object );
    return FALSE;
  }
  if( !EqualFunc( COM_END_OBJECT, packet->func )
     && !EqualFunc( COM_END_NEW_OBJECT, packet->func )
     && !EqualFunc( COM_END_OBJECT_FOR_GET, packet->func ) ){
    PacketFree( packet );
    ObjectFree( object );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
  printf( "Receiving done: %s\n", object->file );
  */

  return object;
}

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$rAw?.$9$k(B.
 * $B$3$N4X?t$O%*%V%8%'%/%H$r;X<($7$F$+$i8F$V(B.
 */
private boolean_t SendObjectData( session_t *session, object_t *object )
{
  content_t *content;
  packet_t *packet;

  printf( "Sending: %s\n", object->file );

  /* send com */
  LinkSendCommandPacket( session->link, COM_START_DATA, NULL, 0 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* send */
  for( content = object->contents ; content ; content = content->next ){
    LinkSendDataPacket( session->link,
		       content->id,
		       content->data,
		       content->length );
  }

  /* send com */
  LinkSendCommandPacket( session->link,
			COM_END_OBJECT_FOR_SEND,
			NULL,
			0 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( FALSE == EqualFunc( COM_STORED_OBJECT, packet->func )
     && FALSE == EqualFunc( COM_STORED_NEW_OBJECT, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
  printf( "Sending done: %s\n", object->file );
  */

  return TRUE;
}

/**********************************************************************/

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$HF10l(B ID $B$N%*%V%8%'%/%H$r<u?.$9$k(B.
 * $B<u?.$7$?%*%V%8%'%/%H$O;XDj$5$l$?%*%V%8%'%/%H$N%U%!%$%k$K3JG<$5$l$k(B.
 */
private object_t *ReceiveObject( session_t *session,
				category_t *category, object_t *object )
{
  char did[ 16 ];

  /*
   * category ID + object ID
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, object->id, 6 );

  if( FALSE == DesignateObject( session, COM_DESIGNATE_OBJECT, did ) )
    return NULL;

  return ReceiveObjectData( session, did, object->file );
}

/*
 * $B;XDj$5$l$?%+%F%4%j$N?75,%*%V%8%'%/%H$r<u?.$9$k(B ($B%*%V%8%'%/%H(B ID $B$r?6$k(B).
 * $B<u?.$7$?%*%V%8%'%/%H$O%*%V%8%'%/%H(B ID $BL>$N?75,%U%!%$%k$K3JG<$5$l$k(B.
 */
private object_t *ReceiveNewObject( session_t *session, folder_t *folder,
				   category_t *category )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + new object ID.
   * $B%*%V%8%'%/%H$N%U%!%$%kL>$O(B object->id.
   */
  strncpy( did, category->id, 6 );
  ID2Hex( folder->lastObjectID++, did + 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObject( session, COM_DESIGNATE_NEW_OBJECT, did ))
    return NULL;
  return ReceiveObjectData( session, did, did + 6 );
}

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H(B ID $B$N%*%V%8%'%/%H$r<u?.$9$k(B.
 * $B<u?.$7$?%*%V%8%'%/%H$O%*%V%8%'%/%H(B ID $BL>$N?75,%U%!%$%k$K3JG<$5$l$k(B.
 */
private object_t *ReceiveNumberedObject( session_t *session,
					category_t *category, char *objectId )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + object ID.
   * $B%*%V%8%'%/%H$N%U%!%$%kL>$O(B object->id.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, objectId, 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObject( session, COM_DESIGNATE_OBJECT, did ) )
    return NULL;
  return ReceiveObjectData( session, did, did + 6 );
}

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H(B ID $B$N%*%V%8%'%/%H$r<u?.$9$k(B.
 * $B<u?.$7$?%*%V%8%'%/%H$O%*%V%8%'%/%H(B ID $BL>$N?75,%U%!%$%k$K3JG<$5$l$k(B.
 */
private object_t *ReceiveCountedObject( session_t *session,
				       category_t *category, char *objectId )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + object ID.
   * $B%*%V%8%'%/%H$N%U%!%$%kL>$O(B object->id.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, objectId, 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObject( session, COM_DESIGNATE_OBJECT_FOR_GET, did ) )
    return NULL;
  return ReceiveObjectData( session, did, did + 6 );
}

/**********************************************************************/

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$rAw?.$9$k(B.
 */
private boolean_t SendObject( session_t *session,
			     category_t *category, object_t *object )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + object ID.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, object->id, 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObjectForSend( session,
				      COM_DESIGNATE_OBJECT_FOR_SEND,
				      did ) ){
    return FALSE;
  }
  return SendObjectData( session, object );
}

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$rAw?.$9$k(B ($B%*%V%8%'%/%H(B ID $B$r?6$k(B).
 * newfile $B%U%i%0$,N)$C$F$$$?>l9g$O%U%!%$%kL>$b=q$-49$($k(B.
 */
private boolean_t SendNewObject( session_t *session, folder_t *folder,
				category_t *category, object_t *object,
				boolean_t newfile )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + new object ID.
   * object->id $B$r=q$-49$((B.
   */
  strncpy( did, category->id, 6 );
  ID2Hex( folder->lastObjectID++, object->id );
  object->id[ 6 ] = '\0';

  category->modified = TRUE;

  if( TRUE == newfile ){
    /*
     * file $B$r=q$-49$((B
     */
    free( object->file );
    object->file = TokenAlloc( object->id );
  }

  strncpy( did + 6, object->id, 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObjectForSend( session,
				      COM_DESIGNATE_NEW_OBJECT_FOR_SEND,
				      did ) ){
    return FALSE;
  }
  return SendObjectData( session, object );
}

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$rAw?.$9$k(B ($B%*%V%8%'%/%H(B ID $B$O(B FFFFFF).
 */
private boolean_t SendFreshObject( session_t *session,
				  category_t *category, object_t *object )
{
  char did[ 16 ];

  /*
   * $B;X<((B ID $B$O(B: category ID + new object ID.
   * object->id $B$r=q$-49$((B.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, NEW_OBJECT_ID, 6 );

  category->modified = TRUE;

  if( FALSE == DesignateObjectForSend( session,
				      COM_DESIGNATE_NEW_OBJECT_FOR_SEND,
				      did ) ){
    return FALSE;
  }
  return SendObjectData( session, object );
}

/**********************************************************************/

/*
 * $B;XDj$5$l$?%*%V%8%'%/%H$N:o=|L?Na$rAw?.$9$k(B.
 */
private boolean_t RemoveObject( session_t *session,
			       category_t *category, object_t *object )
{
  char did[ 16 ];
  packet_t *packet;

  object->id[ 6 ] = '\0';
  printf( "Removing: %s\n", object->id );

  /*
   * category ID + object ID.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, object->id, 6 );
  did[ 12 ] = '\0';

  /*
   * $B%*%V%8%'%/%H$N;X<((B
   */
  if( FALSE == DesignateObjectForSend( session,
				      COM_DESIGNATE_OBJECT_FOR_REMOVE,
				      did ) ){
    return FALSE;
  }

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_REMOVED_OBJECT, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
  printf( "Removing done: %s\n", object->id );
  */

  return TRUE;
}

/**********************************************************************/

/*
 * $B%*%V%8%'%/%H$r%+%?%m%0$+$i:o=|$9$k(B.
 */
private void CatalogRemoveObject( category_t *category, object_t *object )
{
  category->modified = TRUE;

#ifdef DEBUG
  object->id[ 6 ] = NULL;
  fprintf( stderr, "Remove(%s):[\n", object->id );
  FolderCatalogCheck( category->catalog );
  fprintf( stderr, "]\n" );
#endif

  if( category->catalog == object ){
    /*
     * $B%*%V%8%'%/%H$O%+%?%m%0$N@hF,(B
     */
    if( NULL == object->next ){
      /*
       * $B%+%?%m%0$K$O$3$N%*%V%8%'%/%H$N$_4^$^$l$k(B.
       */
      category->catalog = NULL;
    } else {
      category->catalog = object->next;
      category->catalog->prev = object->prev;
    }
  } else if( category->catalog->prev == object ){
    /*
     * $B%*%V%8%'%/%H$O%+%?%m%0$NKvHx(B
     */
    object->prev->next = NULL;
    category->catalog->prev = object->prev;
  } else {
    /*
     * $B%*%V%8%'%/%H$O%+%?%m%0$NCf4V(B
     */
    object->prev->next = object->next;
    object->next->prev = object->prev;
  }

#ifdef DEBUG
  fprintf( stderr, "Removed:[\n" );
  FolderCatalogCheck( category->catalog );
  fprintf( stderr, "]\n" );
#endif
}

/*
 * $B%+%?%m%0$N:G8e$K%*%V%8%'%/%H$rDI2C$9$k(B.
 */
private void CatalogAddObject( category_t *category, object_t *object )
{
  category->modified = TRUE;

#ifdef DEBUG
  object->id[ 6 ] = NULL;
  fprintf( stderr, "Add(%s):[\n", object->id );
  FolderCatalogCheck( category->catalog );
  fprintf( stderr, "]\n" );
#endif

  if( NULL == category->catalog ){
    /*
     * $B%+%?%m%0$,6u(B.
     */
    category->catalog = object;
  } else {
    object->prev = category->catalog->prev;
    object->prev->next = object;
  }
  category->catalog->prev = object;
  object->next = NULL;

#ifdef DEBUG
  fprintf( stderr, "Added:[\n" );
  FolderCatalogCheck( category->catalog );
  fprintf( stderr, "]\n" );
#endif
}

/*
 * $B%+%?%m%0$K%*%V%8%'%/%H$rA^F~$9$k(B.
 */
private void CatalogInsertObject( category_t *category,
				 object_t *object, object_t *before )
{
  category->modified = TRUE;

#ifdef DEBUG
  if( NULL != before ){
    object->id[ 6 ] = NULL;
    before->id[ 6 ] = NULL;
    fprintf( stderr, "Insert(%s/%s):[\n", object->id, before->id );
    FolderCatalogCheck( category->catalog );
    fprintf( stderr, "]\n" );
  }
#endif

  if( NULL == before ){
    CatalogAddObject( category, object );
    return;
  } else if( category->catalog == before ){
    /*
     * $B%+%?%m%0$N@hF,$KA^F~(B
     */
    object->prev = category->catalog->prev;
    category->catalog = object;
  } else {
    object->prev = before->prev;
    object->prev->next = object;
  }
  before->prev = object;
  object->next = before;

#ifdef DEBUG
  fprintf( stderr, "Inserted:[\n" );
  FolderCatalogCheck( category->catalog );
  fprintf( stderr, "]\n" );
#endif
}

/**********************************************************************/

/*
 * $B;XDj$5$l$?%+%F%4%j$K$D$$$F(B, $B%+%l%$%I$+$i%G!<%?8D?t$r<u?.$7(B,
 * $B%[%9%HB&$N%G!<%?8D?t$HHf3S$7$?>e$GA4<u?.(B/$B?75,Aw?.$r9T$J$&(B.
 */
private boolean_t SyncData( session_t *session, folder_t *folder,
			   category_t *category )
{
  packet_t *packet;
  int index, count, oldCount;
  object_t *object, *newObject, *tmp;
  char buf[ BUF_SIZE ];

  category->id[ 6 ] = '\0';
  printf( "Checking: %s\n", category->directory );

  /*
   * $B%+%F%4%j$N;X<((B
   */
  LinkSendCommandPacket( session->link,
			COM_DESIGNATE_CATEGORY_FOR_GET,
			category->id,
			6 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_START_DATA, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
   * $B8D?t$N<u?.(B
   */
  if( NULL == (packet = LinkReceiveDataPacket( session->link )) )
    return FALSE;
  if( !EqualDataFunc( DAT_COUNT, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  count = Hex2ID( packet->data );
  PacketFree( packet );

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_END_DATA, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_END_CATEGORY_FOR_GET, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
   * $B8D?t$N?t$(>e$2(B
   */
  oldCount = 0;
  for( tmp = category->catalog ; tmp ; tmp = tmp->next ){
    if( FALSE == tmp->die && FALSE == tmp->new )
      oldCount++;
  }

  if( count == oldCount ){
    for( tmp = category->catalog ; tmp ; tmp = tmp->next )
      tmp->alive = TRUE;
  } else if( count != oldCount ){
    for( tmp = category->catalog ; tmp ; tmp = tmp->next )
      tmp->alive = FALSE;
    /*
     * $B%*%V%8%'%/%H$N<u?.(B
     */
    for( index = 0 ; index < count ; index++ ){
      ID2Hex( index, buf );
      if( NULL == (newObject = ReceiveCountedObject( session,
						    category,
						    buf )) ){
	buf[ 6 ] = '\0';
	fprintf( stderr, "caleidlink: failed to receive object (%s)\n",
		buf );
	return FALSE;
      }
      newObject->alive = TRUE;
      newObject->hot = TRUE;

      /*
       * $B%+%?%m%0$N:G8e$KDI2C(B
       */
      CatalogAddObject( category, newObject );
    }
  }

  /*
   * $B?75,%*%V%8%'%/%H$NAw?.$H(B, $B:o=|%*%V%8%'%/%H$N:o=|(B.
   */
  for( object = category->catalog ; object ; ){
    if( TRUE == object->new ){
      object->new = FALSE;
      object->alive = TRUE;
      /*
       * $B%[%9%HB&!&?75,%*%V%8%'%/%H$NAw?.(B.
       */
      if( FALSE == FolderLoadObject( folder, category, object ) ){
	object->id[ 6 ] = '\0';
	fprintf( stderr, "caleidlink: cannot load object (%s)\n", object->id );
      } else if( FALSE == SendFreshObject( session, category, object ) ){
	fprintf( stderr, "caleidlink: failed to send fresh object\n" );
	return FALSE;
      } else {
	ContentsFree( object->contents );
	object->contents = NULL;
      }
      ID2Hex( count, object->id );
    } else if( FALSE == object->alive ){
      /*
       * $B%*%V%8%'%/%H$r%+%?%m%0$+$i:o=|(B.
       */
      /*
      printf( "Deleting done: %s\n", object->file );
      */

      FolderDeleteFile( folder, category, object );
      CatalogRemoveObject( category, object );

      tmp = object->next;
      ObjectFree( object );
      object = tmp;
      /*
       * $B:o=|$N>l9g(B, object $B$r99?7$7$J$$(B.
       */
      continue;
    }
    object = object->next;
  }

  /*
  printf( "Checking done: %s\n", category->directory );
  */

  return TRUE;
}

/**********************************************************************/

#define DICT_LIMIT	16

#define DictFree()							\
{									\
  while( dictc > 0 )							\
    free( dictv[ --dictc ] );						\
}

/*
 * $B;XDj$5$l$?%+%F%4%j$K$D$$$F(B, $B%+%l%$%I$+$i%+%?%m%0$r<u?.$7(B,
 * $B%[%9%HB&$N%+%?%m%0$HHf3S$7$?>e$G%7%s%/%m%J%$%:$r9T$J$&(B.
 */
private boolean_t SyncCategory( session_t *session, folder_t *folder,
			       category_t *category )
{
  typedef struct {
    char id[ 6 ];
    char flag[ 2 ];
  } dictionary_t;

  dictionary_t *dict, *dictv[ DICT_LIMIT ];
  int dictc;
  int dictl[ DICT_LIMIT ];
  int dptr, optr;
  long id;
  packet_t *packet;
  object_t *object, *newObject = NULL, *tmp, *currentObject;

  if( EqualID( category->id, CAT_BACKUP )
     ||  EqualID( category->id, CAT_CALENDAR )
     ||  EqualID( category->id, CAT_CALENDAR_Q ) ){
    return TRUE;
  }

  if( EqualID( category->id, CAT_SHEET )
     || EqualID( category->id, CAT_ADDIN ) ){
    return SyncData( session, folder, category );
  }

  category->id[ 6 ] = '\0';
  printf( "Syncing: %s\n", category->directory );

  /*
   * $B%+%F%4%j$N;X<((B
   */
  LinkSendCommandPacket( session->link,
			COM_DESIGNATE_CATEGORY,
			category->id,
			6 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) )
    return FALSE;
  if( !EqualFunc( COM_START_DATA, packet->func ) ){
    PacketFree( packet );
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
   * $B%+%?%m%0$N<u?.(B
   */

  dictv[ 0 ] = NULL;
  dictc = 0;

  for( ; ; ){
    if( NULL == (packet = LinkReceivePacket( session->link )) ){
      DictFree();
      return FALSE;
    }
    if( DAT_PACKET == packet->type
       && (EqualDataFunc( DAT_CATALOG_LAST, packet->func )
	   || EqualDataFunc( DAT_CATALOG, packet->func )) ){
      dictv[ dictc ] = (dictionary_t *)packet->data;
      if( 0 != packet->length % sizeof( dictionary_t ) ){
	fprintf( stderr, "caleidlink: unexpected length (%d)\n", packet->length );
	PacketFree( packet );
	DictFree();
	return FALSE;
      }
      dictl[ dictc ] = packet->length / sizeof( dictionary_t );
      dictc++;
      free( packet );	/* data $B$r;D$9$?$a(B PacketFree() $B$G$O$J$$(B. */
      dictv[ dictc ] = NULL;
      if( EqualDataFunc( DAT_CATALOG, packet->func ) ){
	/* send ack */
	LinkSendAckPacket( session->link );
      }
    } else if( COM_PACKET == packet->type
	      && EqualFunc( COM_END_DATA, packet->func ) ){
      PacketFree( packet );
      break;
    } else if( COM_PACKET == packet->type
	      && EqualFunc( COM_DATA_LAP, packet->func ) ){
      PacketFree( packet );

      /* send ack */
      LinkSendAckPacket( session->link );
    } else {
      if( DAT_PACKET == packet->type ){
	packet->func[ 6 ] = '\0';
      } else {
	packet->func[ 4 ] = '\0';
      }
      fprintf( stderr,
	      "caleidlink: unexpected packet type (%02x/%s) while receiving catalog\n", packet->type, packet->func );
      PacketFree( packet );
      DictFree();
      return FALSE;
    }
  }

  /* send ack */
  LinkSendAckPacket( session->link );

  /* receive com */
  if( NULL == (packet = LinkReceiveCommandPacket( session->link )) ){
    DictFree();
    return FALSE;
  }
  if( !EqualFunc( COM_END_CATEGORY, packet->func ) ){
    PacketFree( packet );
    DictFree();
    return FALSE;
  }
  PacketFree( packet );

  /* send ack */
  LinkSendAckPacket( session->link );

  /*
   * $B%[%9%HB&$N%+%?%m%0$HHf3S$7$F%7%s%/%m%J%$%:$r<B9T(B.
   *
   * $B%7%s%/%m$KF~$k:]$NCm0U;v9`(B:
   *   $B!&(BFALSE == consistent $B$N>l9g(B, $B$D$^$j(B, $B?75,$N%7%s%/%m%J%$%:;~$O(B,
   *     $B%+%?%m%0$N<u?.$N$_$r9T$J$$(B, $B:G=*%*%V%8%'%/%HHV9f$@$1$r99?7$9$k(B.
   *   $B!&(BTRUE == object->die $B$O%[%9%HB&$N!X%U%!%$%k!Y$,:o=|$5$l$F(B
   *     $B$$$k$3$H$r0UL#$9$k(B.
   *   $B!&(BTRUE == object->updated $B$O%[%9%HB&$N!X%U%!%$%k!Y$,99?7$5$l$F(B
   *     $B$$$k$3$H$r0UL#$9$k(B.
   *   $B!&(BTRUE == object->new $B$O%[%9%HB&$N!X%U%!%$%k!Y$,?75,$N$b$N$G(B
   *     $B$"$k$3$H$r0UL#$9$k(B.
   *
   * $B%7%s%/%m$r=*N;$9$k:]$NCm0U;v9`(B:
   *   $B!&(BTRUE == object->alive $B$O(B, $B$=$N%*%V%8%'%/%H$,%+%l%$%IB&$KB8:_$9$k$+(B,
   *     $B?75,$N%*%V%8%'%/%H$G$"$k(B ($B$D$^$j(B, $B@8$-$F$$$k(B) $B$3$H$r0UL#$9$k(B.
   *     alive $B%U%i%0$,N)$C$F$$$J$$%*%V%8%'%/%H$O%P%C%5%j:o=|$9$k(B.
   *   $B!&(BTRUE == object->hot $B$O(B, $B%7%s%/%m$,=*$o$C$?8e$G%*%V%8%'%/%H$NFbMF$r(B
   *     $B%U%!%$%k$K=q$-=P$9$3$H$r0UL#$9$k(B.
   *   $B!&(BTRUE == category->modified $B$O(B, $B%7%s%/%m$,=*$o$C$?8e$G%+%?%m%0$NFbMF$r(B
   *     $B%U%!%$%k$K=q$-=P$9$3$H$r0UL#$9$k(B.
   */
/*
  FolderCatalogCheck( category->catalog );
*/

  /*
   * alive $B%U%i%0$N=i4|2=(B.
   */
  for( tmp = category->catalog ; tmp ; tmp = tmp->next )
    tmp->alive = FALSE;

  /*
   * $B$^$@Hf3S$N40N;$7$F$$$J$$%*%V%8%'%/%H$N@hF,(B
   */
  currentObject = category->catalog;

  for( dptr = 0 ; dptr < dictc ; dptr++ ){
    dict = dictv[ dptr ];
    for( optr = 0 ; optr < dictl[ dptr ] ; optr++ ){
      if( EqualID( dict[ optr ].id, NEW_OBJECT_ID ) ){
	/*
	 * $B%+%l%$%IB&!&?75,%*%V%8%'%/%H(B.
	 */
	if( FALSE == session->consistent ){
	  /*
	   * $B=i2s%7%s%/%m;~$O(B, $B?7$7$$%*%V%8%'%/%H$K$D$1$k$Y$-:G=*HV9f$,(B
	   * $BJ,$+$i$J$$$N$G(B FFFFFF $B%*%V%8%'%/%H$OL5;k$9$k(B.
	   *
	   * $B$G$b(B, $BK\Ev$NDL?.$G$O(B, $B$R$g$C$H$9$k$H(B, $B$I$3$+$G:G=*HV9f$r(B
	   * $B%O%s%I%7%'%$%/$7$F$k$N$+$b$7$l$J$$(B. $B$=$N>l=j$HJ}K!$,(B, $B$^$@(B,
	   * $B2r@O$G$-$F$$$J$$$N$G(B, $B$3$l$GG<F@$9$k$3$H$K$9$k(B.
	   */
	} else {
	  if( NULL == (newObject = ReceiveNewObject( session, folder,
						    category )) ){
	    fprintf( stderr, "caleidlink: failed to receive new object\n" );
	    DictFree();
	    return FALSE;
	  }
	  newObject->alive = TRUE;
	  newObject->hot = TRUE;
	  /*
	   * $B?75,%*%V%8%'%/%H$r%+%?%m%0!&%j%9%H$N:G8e$KA^F~(B.
	   */
	  CatalogAddObject( category, newObject );
	  if( NULL == currentObject )
	    currentObject = newObject;
/*
	  FolderCatalogCheck( category->catalog );
*/
	}
      } else {
	/*
	 * $B%+%l%$%IB&!&4{B8%*%V%8%'%/%H(B.
	 * $B$3$N(B ID $B$r%[%9%HB&!&%+%?%m%0$+$i8!:w(B.
	 */
	for( object = currentObject ; object ; object = object->next ){
	  if( EqualID( dict[ optr ].id, object->id ) ){
	    /*
	     * $B%+%l%$%IB&(B ID $B$r%[%9%HB&%+%?%m%0Fb$KH/8+(B.
	     */
	    object->alive = TRUE;
	    /*
	     * $B%*%V%8%'%/%H$NAw<u?.(B.
	     */
	    if( '1' == dict[ optr ].flag[ 1 ] ){
	      /*
	       * $B%+%l%$%I!&%*%V%8%'%/%H$,99?7$5$l$F$$$k(B.
	       */
	      if( FALSE == object->die && TRUE == object->updated ){
		object->updated = FALSE; /* $B8e$G?75,$KAw?.$9$k(B. */
	      } else {
		object->alive = FALSE;	/* $B8e$G%*%V%8%'%/%H$r(B free $B$9$k(B. */
	      }

	      /* $B>e=q$-<u?.(B */
	      if( NULL == (newObject = ReceiveObject( session,
						     category, object )) ){
		object->id[ 6 ] = '\0';
		fprintf( stderr, "caleidlink: failed to receive object (%s)\n",
			object->id );
		DictFree();
		return FALSE;
	      }
	      newObject->alive = TRUE;
	      newObject->hot = TRUE;

	      /*
	       * object $B$r%-%e!<$+$i30$7$F(B,
	       * newObject $B$r(B currentObject $B$NA0$KA^F~(B.
	       */
	      if( object == currentObject )
		currentObject = currentObject->next;
	      CatalogRemoveObject( category, object );
	      CatalogInsertObject( category, newObject, currentObject );
	      currentObject = newObject;

	      if( TRUE == object->alive ){
		/*
		 * $B%[%9%H!&%*%V%8%'%/%H$,99?7$5$l$F$$$k(B.
		 * $B%[%9%HB&$r?75,$N%*%V%8%'%/%H$H$7$FAw?.(B.
		 */
		if( FALSE == FolderLoadObject( folder, category, object ) ){
		  object->id[ 6 ] = '\0';
		  fprintf( stderr, "caleidlink: cannot load object (%s)\n", object->id );
		} else if( FALSE == SendNewObject( session, folder,
						  category, object, TRUE ) ){
		  fprintf( stderr, "caleidlink: failed to send new object\n" );
		  DictFree();
		  return FALSE;
		} else {
		  ContentsFree( object->contents );
		  object->contents = NULL;
		}
		object->hot = TRUE;
		/*
		 * object $B$r%j%9%H$N:G8e$KDI2C(B.
		 */
		CatalogAddObject( category, object );
		if( NULL == currentObject )
		  currentObject = object;
	      } else {
		ObjectFree( object );
	      }
	    } else {
	      /*
	       * $B%+%l%$%I!&%*%V%8%'%/%H$,99?7$5$l$F$$$J$$(B.
	       */
	      if( TRUE == object->die ){
		/*
		 * $B%[%9%H!&%*%V%8%'%/%H$,:o=|$5$l$F$$$k(B.
		 */
		object->alive = FALSE;
		if( FALSE == RemoveObject( session, category, object ) ){
		  object->id[ 6 ] = '\0';
		  fprintf( stderr, "caleidlink: failed to remove object (%s)\n",
			  object->id );
		  DictFree();
		  return FALSE;
		}
	      } else if( TRUE == object->updated ){
		object->updated = FALSE;
		/*
		 * $B%[%9%H!&%*%V%8%'%/%H$,99?7$5$l$F$$$k(B.
		 * $B%[%9%HB&$rAw?.(B.
		 */
		if( FALSE == FolderLoadObject( folder, category, object ) ){
		  object->id[ 6 ] = '\0';
		  fprintf( stderr, "caleidlink: cannot load object (%s)\n", object->id );
		} else if( FALSE == SendObject( session, category, object ) ){
		  object->id[ 6 ] = '\0';
		  fprintf( stderr, "caleidlink: failed to send object (%s)\n",
			  object->id );
		  DictFree();
		  return FALSE;
		} else {
		  ContentsFree( object->contents );
		  object->contents = NULL;
		}
	      }
	      if( object != currentObject ){
		/*
		 * object $B$r%-%e!<$+$i30$7$F(B,
		 * object $B$r(B currentObject $B$NA0$KA^F~(B.
		 */
		CatalogRemoveObject( category, object );
		CatalogInsertObject( category, object, currentObject );
		currentObject = object;
	      }
	    }
	    /*
	     * $B8!:w$,0lCW$7$?$N$G30$N(B for $B%k!<%W$rH4$1$^$9(B.
	     */
	    break;
	  } /* end of if( EqualID( dict[ optr ].id, object->id ) ) */
	} /* end of for( object = currentObject ;; ) */

	if( NULL == object ){
	  if( TRUE == session->consistent ){
	    /*
	     * $B%*%V%8%'%/%H$,8+$D$+$i$J$$(B.
	     */
	    dict[ optr ].id[ 6 ] = '\0';
	    fprintf( stderr,
		    "caleidlink: cannot find object (%s) in host catalog\n",
		    dict[ optr ].id );
	    /*
	     * $BB3$1$k(B?
	     */
	    continue;
/*
	    DictFree();
	    return FALSE;
*/
	  }

	  /*
	   * $B?75,%7%s%/%mCf$NL$CN$N%*%V%8%'%/%H(B.
	   */
	  id = Hex2ID( dict[ optr ].id );
	  if( id >= folder->lastObjectID )
	    folder->lastObjectID = id + 1;
	  /*
	   */
	  if( NULL == (newObject = ReceiveNumberedObject( session,
							 category,
							 dict[ optr ].id )) ){
	    dict[ optr ].id[ 6 ] = '\0';
	    fprintf( stderr, "caleidlink: failed to receive object (%s)\n",
		    dict[ optr ].id );
	    DictFree();
	    return FALSE;
	  }
	  newObject->alive = TRUE;
	  newObject->hot = TRUE;
	  /*
	   * newObject $B$r(B currentObject $B$NA0$KA^F~(B.
	   */
	  CatalogInsertObject( category, newObject, currentObject );
	  currentObject = newObject;
	} /* end of if( NULL == object ) */

	/*
	 * Caleid $B%+%?%m%0$H(B Host $B%+%?%m%0$NHf3SItJ,$N:G8e(B.
	 * $B$3$3$G(B currentObject $B0JA0$N%*%V%8%'%/%H$OHf3S$,=*$o$C$F$$$k(B.
	 */
	currentObject = currentObject->next;
      } /* end of if( EqualID( dict[ optr ].id, NEW_OBJECT_ID ) ) */
    }
  }

  /*
   * $B?75,%*%V%8%'%/%H$NAw?.$H(B, $B:o=|%*%V%8%'%/%H$N:o=|(B.
   */
  for( object = category->catalog ; object ; ){
    if( TRUE == object->new ){
      if( TRUE == session->consistent ){
	object->new = FALSE;
	object->alive = TRUE;
	/*
	 * $B%[%9%HB&!&?75,%*%V%8%'%/%H$NAw?.(B.
	 */
	if( FALSE == FolderLoadObject( folder, category, object ) ){
	  object->id[ 6 ] = '\0';
	  fprintf( stderr, "caleidlink: cannot load object (%s)\n", object->id );
	} else if( FALSE == SendNewObject( session, folder,
					  category, object, FALSE ) ){
	  fprintf( stderr, "caleidlink: failed to send new object\n" );
	  DictFree();
	  return FALSE;
	} else {
	  ContentsFree( object->contents );
	  object->contents = NULL;
	}
      }
    } else if( FALSE == object->alive ){
      /*
       * $B%*%V%8%'%/%H$r%+%?%m%0$+$i:o=|(B.
       */
      /*
      printf( "Deleting done: %s\n", object->file );
      */

      FolderDeleteFile( folder, category, object );
      CatalogRemoveObject( category, object );

      tmp = object->next;
      ObjectFree( object );
      object = tmp;
      /*
       * $B:o=|$N>l9g(B, object $B$r99?7$7$J$$(B.
       */
      continue;
    }
    object = object->next;
  }

  /*
   * $B%G%#%/%7%g%J%j$N(B free
   */
  DictFree();

  /*
  printf( "Syncing done: %s\n", category->directory );
  */

  return TRUE;
}

public boolean_t Synchronize( char *dev, long baud,
			     char *file, char codingSystem,
			     boolean_t flagForce,
			     char *log )
{
  session_t *session;
  folder_t *folder;
  category_t *category;
  boolean_t error;
  packet_t *packet;
  time_t now;
  char buf[ BUF_SIZE ];

  if( NULL == (session = SessionAlloc( dev, baud, log )) ){
    fprintf( stderr, "caleidlink: cannot open link (%s)\n", dev );
    return FALSE;
  }

  if( NULL == (folder = FolderLoad( file, codingSystem )) ){
    fprintf( stderr, "caleidlink: cannot open folder (%s)\n", file );
    SessionFree( session );
    return FALSE;
  }

  printf( "Ready to Synchronize\n" );

  if( FALSE == SessionPreamble( session, folder->id ) ){
    fprintf( stderr, "caleidlink: initial hand-shake failed\n" );
    SessionFree( session );
    return FALSE;
  }

  if( FALSE == session->consistent ){
    /*
     * $B%;%C%7%g%s(B ID $B$,0lCW$7$J$$(B.
     */
    if( TRUE == flagForce ){
      printf( "Error: inconsistent session ID\n" );
      SessionFree( session );
      return FALSE;
    }

    if( FALSE == flagForce ){
      fprintf( stderr, "caleidlink: Initial synchronization. Continue? [y/n]: " );
      fgets( buf, BUF_SIZE, stdin );
      if( 'y' != *buf && 'Y' != *buf ){
	SessionFree( session );
	return FALSE;
      }
    }

    session->consistent = FALSE;
  }

  printf( "Starting: Synchronize\n" );

  for( error = FALSE ; ; ){
    for( category = folder->categories ; category ; category = category->next ){
      if( FALSE == SyncCategory( session, folder, category ) ){
	error = TRUE;
	break;
      }
    }
    if( FALSE == error && FALSE == session->consistent ){
      session->consistent = TRUE;
      continue;
    } else {
      break;
    }
  }

  if( TRUE == error ){
    /*
     * $B%(%i!<H/@8;~$K$b(B, $BESCf$^$G%7%s%/%m$7$?>uBV$G%U%)%k%@$r%;!<%V$9$k(B.
     */
    if( FALSE == FolderSave( folder ) )
      fprintf( stderr, "caleidlink: cannot save folder\n" );

/*
    LinkSendCanPacket( session->link, "0101" );
*/

    SessionFree( session );
    return FALSE;
  }

/*
  FolderCheck( folder );
*/

  /*
   * $B?7$7$$%;%C%7%g%s(B ID
   */
  if( ' ' == folder->id[ 4 ] ){
    time( &now );
    srand( now );
  } else {
    srand( Hex2ID( folder->id + 4 ) );
  }

  strncpy( folder->id, folder->hostId, 4 );
  ID2Hex( rand(), buf );
  strncpy( folder->id + 4, buf, 6 );
  strncpy( folder->id + 10, "0", 1 );

  /*
   * $B?7$7$$%U%)%k%@(B ID $B$rAw?.$9$k(B. $B$3$l0J8e(B, $B0[>o=*N;;~$G$bI,$:(B
   * $B%U%)%k%@$r(B save $B$9$k(B. $B<!$N%7%s%/%m;~$K%7%s%/%m(B ID $B$,9g$o$J$$>l9g$O(B
   * $B%f!<%6$K<j$GA02s$N%7%s%/%m;~$N>pJs(B (*.org) $B$rI|3h$7$F$b$i$&(B.
   */

  /* send com */
  LinkSendCommandPacket( session->link,
			COM_SYNC_ID,
			folder->id,
			24 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) ){
    fprintf( stderr, "caleidlink: cannot receive ack\n" );
    error = TRUE;
  }

  /* receive com */
  if( FALSE == error ){
    if( NULL == (packet = LinkReceiveCommandPacket( session->link )) ){
      fprintf( stderr, "caleidlink: cannot receive COM packet (expecting COM_STORED_SYNC_ID)\n" );
      error = TRUE;
    }
    if( FALSE == error ){
      if( !EqualFunc( COM_STORED_SYNC_ID, packet->func ) ){
	fprintf( stderr, "caleidlink: cannot receive COM_STORED_SYNC_ID\n" );
	error = TRUE;
      }
      PacketFree( packet );
    }
  }


  if( FALSE == error ){
    /* send ack */
    LinkSendAckPacket( session->link );

    /* send can */
    LinkSendCanPacket( session->link, "0100" );

    /* receive ack */
/*
    if( FALSE == LinkReceiveAckPacket( session->link ) ){
      fprintf( stderr, "caleidlink: receive ack for CAN failed\n" );
      error = TRUE;
    }
*/
    /*
     * XXX
     */
    Wait100msec();
  }

  if( FALSE == FolderSave( folder ) ){
    fprintf( stderr, "caleidlink: cannot save folder\n" );
    error = TRUE;
  }

  FolderFree( folder );
  SessionFree( session );

  if( FALSE == error ){
    printf( "Done: Synchronize\n" );
    return TRUE;
  } else {
    printf( "Done: Synchronize (with some errors)\n" );
    return FALSE;
  }
}
