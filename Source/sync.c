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
 * 受信用にオブジェクトを指示する.
 * 指示のコマンドは引数で与える.
 */
private boolean_t DesignateObject( session_t *session, char *func, char *did )
{
  packet_t *packet;

  /*
   * オブジェクトの指示
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
 * 送信用にオブジェクトを指示する.
 * 指示のコマンドは引数で与える.
 */
private boolean_t DesignateObjectForSend( session_t *session,
					 char *func, char *did )
{
  /*
   * オブジェクトの指示
   */
  LinkSendCommandPacket( session->link, func, did, 12 );

  /* receive ack */
  if( FALSE == LinkReceiveAckPacket( session->link ) )
    return FALSE;

  return TRUE;
}

/**********************************************************************/

/*
 * オブジェクトを受信して, 構造体として返す.
 * この関数はオブジェクトを指示してから呼ぶ.
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
      free( packet );	/* data を残すため, PacketFree() ではない */
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
 * 指定されたオブジェクトを送信する.
 * この関数はオブジェクトを指示してから呼ぶ.
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
 * 指定されたオブジェクトと同一 ID のオブジェクトを受信する.
 * 受信したオブジェクトは指定されたオブジェクトのファイルに格納される.
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
 * 指定されたカテゴリの新規オブジェクトを受信する (オブジェクト ID を振る).
 * 受信したオブジェクトはオブジェクト ID 名の新規ファイルに格納される.
 */
private object_t *ReceiveNewObject( session_t *session, folder_t *folder,
				   category_t *category )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + new object ID.
   * オブジェクトのファイル名は object->id.
   */
  strncpy( did, category->id, 6 );
  ID2Hex( folder->lastObjectID++, did + 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObject( session, COM_DESIGNATE_NEW_OBJECT, did ))
    return NULL;
  return ReceiveObjectData( session, did, did + 6 );
}

/*
 * 指定されたオブジェクト ID のオブジェクトを受信する.
 * 受信したオブジェクトはオブジェクト ID 名の新規ファイルに格納される.
 */
private object_t *ReceiveNumberedObject( session_t *session,
					category_t *category, char *objectId )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + object ID.
   * オブジェクトのファイル名は object->id.
   */
  strncpy( did, category->id, 6 );
  strncpy( did + 6, objectId, 6 );
  did[ 12 ] = '\0';

  if( FALSE == DesignateObject( session, COM_DESIGNATE_OBJECT, did ) )
    return NULL;
  return ReceiveObjectData( session, did, did + 6 );
}

/*
 * 指定されたオブジェクト ID のオブジェクトを受信する.
 * 受信したオブジェクトはオブジェクト ID 名の新規ファイルに格納される.
 */
private object_t *ReceiveCountedObject( session_t *session,
				       category_t *category, char *objectId )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + object ID.
   * オブジェクトのファイル名は object->id.
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
 * 指定されたオブジェクトを送信する.
 */
private boolean_t SendObject( session_t *session,
			     category_t *category, object_t *object )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + object ID.
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
 * 指定されたオブジェクトを送信する (オブジェクト ID を振る).
 * newfile フラグが立っていた場合はファイル名も書き換える.
 */
private boolean_t SendNewObject( session_t *session, folder_t *folder,
				category_t *category, object_t *object,
				boolean_t newfile )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + new object ID.
   * object->id を書き換え.
   */
  strncpy( did, category->id, 6 );
  ID2Hex( folder->lastObjectID++, object->id );
  object->id[ 6 ] = '\0';

  category->modified = TRUE;

  if( TRUE == newfile ){
    /*
     * file を書き換え
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
 * 指定されたオブジェクトを送信する (オブジェクト ID は FFFFFF).
 */
private boolean_t SendFreshObject( session_t *session,
				  category_t *category, object_t *object )
{
  char did[ 16 ];

  /*
   * 指示 ID は: category ID + new object ID.
   * object->id を書き換え.
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
 * 指定されたオブジェクトの削除命令を送信する.
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
   * オブジェクトの指示
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
 * オブジェクトをカタログから削除する.
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
     * オブジェクトはカタログの先頭
     */
    if( NULL == object->next ){
      /*
       * カタログにはこのオブジェクトのみ含まれる.
       */
      category->catalog = NULL;
    } else {
      category->catalog = object->next;
      category->catalog->prev = object->prev;
    }
  } else if( category->catalog->prev == object ){
    /*
     * オブジェクトはカタログの末尾
     */
    object->prev->next = NULL;
    category->catalog->prev = object->prev;
  } else {
    /*
     * オブジェクトはカタログの中間
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
 * カタログの最後にオブジェクトを追加する.
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
     * カタログが空.
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
 * カタログにオブジェクトを挿入する.
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
     * カタログの先頭に挿入
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
 * 指定されたカテゴリについて, カレイドからデータ個数を受信し,
 * ホスト側のデータ個数と比較した上で全受信/新規送信を行なう.
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
   * カテゴリの指示
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
   * 個数の受信
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
   * 個数の数え上げ
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
     * オブジェクトの受信
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
       * カタログの最後に追加
       */
      CatalogAddObject( category, newObject );
    }
  }

  /*
   * 新規オブジェクトの送信と, 削除オブジェクトの削除.
   */
  for( object = category->catalog ; object ; ){
    if( TRUE == object->new ){
      object->new = FALSE;
      object->alive = TRUE;
      /*
       * ホスト側・新規オブジェクトの送信.
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
       * オブジェクトをカタログから削除.
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
       * 削除の場合, object を更新しない.
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
 * 指定されたカテゴリについて, カレイドからカタログを受信し,
 * ホスト側のカタログと比較した上でシンクロナイズを行なう.
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
   * カテゴリの指示
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
   * カタログの受信
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
      free( packet );	/* data を残すため PacketFree() ではない. */
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
   * ホスト側のカタログと比較してシンクロナイズを実行.
   *
   * シンクロに入る際の注意事項:
   *   ・FALSE == consistent の場合, つまり, 新規のシンクロナイズ時は,
   *     カタログの受信のみを行ない, 最終オブジェクト番号だけを更新する.
   *   ・TRUE == object->die はホスト側の『ファイル』が削除されて
   *     いることを意味する.
   *   ・TRUE == object->updated はホスト側の『ファイル』が更新されて
   *     いることを意味する.
   *   ・TRUE == object->new はホスト側の『ファイル』が新規のもので
   *     あることを意味する.
   *
   * シンクロを終了する際の注意事項:
   *   ・TRUE == object->alive は, そのオブジェクトがカレイド側に存在するか,
   *     新規のオブジェクトである (つまり, 生きている) ことを意味する.
   *     alive フラグが立っていないオブジェクトはバッサリ削除する.
   *   ・TRUE == object->hot は, シンクロが終わった後でオブジェクトの内容を
   *     ファイルに書き出すことを意味する.
   *   ・TRUE == category->modified は, シンクロが終わった後でカタログの内容を
   *     ファイルに書き出すことを意味する.
   */
/*
  FolderCatalogCheck( category->catalog );
*/

  /*
   * alive フラグの初期化.
   */
  for( tmp = category->catalog ; tmp ; tmp = tmp->next )
    tmp->alive = FALSE;

  /*
   * まだ比較の完了していないオブジェクトの先頭
   */
  currentObject = category->catalog;

  for( dptr = 0 ; dptr < dictc ; dptr++ ){
    dict = dictv[ dptr ];
    for( optr = 0 ; optr < dictl[ dptr ] ; optr++ ){
      if( EqualID( dict[ optr ].id, NEW_OBJECT_ID ) ){
	/*
	 * カレイド側・新規オブジェクト.
	 */
	if( FALSE == session->consistent ){
	  /*
	   * 初回シンクロ時は, 新しいオブジェクトにつけるべき最終番号が
	   * 分からないので FFFFFF オブジェクトは無視する.
	   *
	   * でも, 本当の通信では, ひょっとすると, どこかで最終番号を
	   * ハンドシェイクしてるのかもしれない. その場所と方法が, まだ,
	   * 解析できていないので, これで納得することにする.
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
	   * 新規オブジェクトをカタログ・リストの最後に挿入.
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
	 * カレイド側・既存オブジェクト.
	 * この ID をホスト側・カタログから検索.
	 */
	for( object = currentObject ; object ; object = object->next ){
	  if( EqualID( dict[ optr ].id, object->id ) ){
	    /*
	     * カレイド側 ID をホスト側カタログ内に発見.
	     */
	    object->alive = TRUE;
	    /*
	     * オブジェクトの送受信.
	     */
	    if( '1' == dict[ optr ].flag[ 1 ] ){
	      /*
	       * カレイド・オブジェクトが更新されている.
	       */
	      if( FALSE == object->die && TRUE == object->updated ){
		object->updated = FALSE; /* 後で新規に送信する. */
	      } else {
		object->alive = FALSE;	/* 後でオブジェクトを free する. */
	      }

	      /* 上書き受信 */
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
	       * object をキューから外して,
	       * newObject を currentObject の前に挿入.
	       */
	      if( object == currentObject )
		currentObject = currentObject->next;
	      CatalogRemoveObject( category, object );
	      CatalogInsertObject( category, newObject, currentObject );
	      currentObject = newObject;

	      if( TRUE == object->alive ){
		/*
		 * ホスト・オブジェクトが更新されている.
		 * ホスト側を新規のオブジェクトとして送信.
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
		 * object をリストの最後に追加.
		 */
		CatalogAddObject( category, object );
		if( NULL == currentObject )
		  currentObject = object;
	      } else {
		ObjectFree( object );
	      }
	    } else {
	      /*
	       * カレイド・オブジェクトが更新されていない.
	       */
	      if( TRUE == object->die ){
		/*
		 * ホスト・オブジェクトが削除されている.
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
		 * ホスト・オブジェクトが更新されている.
		 * ホスト側を送信.
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
		 * object をキューから外して,
		 * object を currentObject の前に挿入.
		 */
		CatalogRemoveObject( category, object );
		CatalogInsertObject( category, object, currentObject );
		currentObject = object;
	      }
	    }
	    /*
	     * 検索が一致したので外の for ループを抜けます.
	     */
	    break;
	  } /* end of if( EqualID( dict[ optr ].id, object->id ) ) */
	} /* end of for( object = currentObject ;; ) */

	if( NULL == object ){
	  if( TRUE == session->consistent ){
	    /*
	     * オブジェクトが見つからない.
	     */
	    dict[ optr ].id[ 6 ] = '\0';
	    fprintf( stderr,
		    "caleidlink: cannot find object (%s) in host catalog\n",
		    dict[ optr ].id );
	    /*
	     * 続ける?
	     */
	    continue;
/*
	    DictFree();
	    return FALSE;
*/
	  }

	  /*
	   * 新規シンクロ中の未知のオブジェクト.
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
	   * newObject を currentObject の前に挿入.
	   */
	  CatalogInsertObject( category, newObject, currentObject );
	  currentObject = newObject;
	} /* end of if( NULL == object ) */

	/*
	 * Caleid カタログと Host カタログの比較部分の最後.
	 * ここで currentObject 以前のオブジェクトは比較が終わっている.
	 */
	currentObject = currentObject->next;
      } /* end of if( EqualID( dict[ optr ].id, NEW_OBJECT_ID ) ) */
    }
  }

  /*
   * 新規オブジェクトの送信と, 削除オブジェクトの削除.
   */
  for( object = category->catalog ; object ; ){
    if( TRUE == object->new ){
      if( TRUE == session->consistent ){
	object->new = FALSE;
	object->alive = TRUE;
	/*
	 * ホスト側・新規オブジェクトの送信.
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
       * オブジェクトをカタログから削除.
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
       * 削除の場合, object を更新しない.
       */
      continue;
    }
    object = object->next;
  }

  /*
   * ディクショナリの free
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
     * セッション ID が一致しない.
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
     * エラー発生時にも, 途中までシンクロした状態でフォルダをセーブする.
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
   * 新しいセッション ID
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
   * 新しいフォルダ ID を送信する. これ以後, 異常終了時でも必ず
   * フォルダを save する. 次のシンクロ時にシンクロ ID が合わない場合は
   * ユーザに手で前回のシンクロ時の情報 (*.org) を復活してもらう.
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
