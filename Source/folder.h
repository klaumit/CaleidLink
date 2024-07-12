/*
 * folder.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: folder.h,v 1.10 2001/03/12 08:47:50 nrt Exp $
 */

#ifndef __FOLDER_H__
#define __FOLDER_H__

#include <time.h>

/*
 * コンテンツを格納する構造体.
 * コンテンツはコンテントのリスト.
 */
typedef struct CONTENT_T {
  char id[ 6 + 2 ];			/* フィールド ID */
  char *data;
  int length;
  struct CONTENT_T *next;
} content_t;

/*
 * カタログを格納する構造体.
 * カタログはオブジェクトのリスト.
 */
typedef struct OBJECT_T {
  char id[ 6 + 2 ];			/* オブジェクト ID */
  boolean_t new;			/* ホスト側新規オブジェクト */
  boolean_t updated;			/* ホスト側更新オブジェクト */
  boolean_t die;			/* ホスト側削除オブジェクト */
  boolean_t hot;			/* 終了時に書き出しするオブジェクト */
  boolean_t alive;			/* カレイド側に同じ object id がある */
  char *file;
  time_t timeStamp;
  content_t *contents;
  struct OBJECT_T *next;
  struct OBJECT_T *prev;
} object_t;

/*
 * ヘディングを格納する構造体.
 * ヘディングはフィールドのリスト.
 */
typedef struct FIELD_T {
  char id[ 6 + 1 ];			/* フィールド ID */
  char type;				/* 型 */
  char *alias;
  struct FIELD_T *next;
} field_t;

/*
 * フィールドの型
 */
#define F_NONE		'N'
#define F_BINARY	'B'
#define F_CONTINUE	'C'
#define F_STRING	'S'
#define F_DATE		'D'
#define F_TIME		'T'

/*
 * カテゴリを格納する構造体.
 * カテゴリはヘディングとカタログから構成される.
 */
typedef struct CATEGORY_T {
  char id[ 6 + 2 ];			/* カテゴリ ID */
  boolean_t modified;
  char *directory;
  char *name;
  field_t *heading;			/* ヘディング */
  object_t *catalog;			/* カタログ */
  struct CATEGORY_T *next;
} category_t;

/*
 * フォルダ抽象
 */
typedef struct {
  char id[ 24 ];			/* フォルダ ID */
  /*
   * 24bytes の内訳:
   *     o ホスト識別 ? (4bytes) ホスト指定 (ex. "成田").
   *     o シンクロ ID (7bytes) ホスト指定.
   *          この内容がいまいち分からない. 16進じゃないみたいだし.
   *     o 空白 (13bytes) 固定.
   */
  char hostId[ 4 ];			/* ホスト識別 */
  char *top;				/* トップディレクトリ */
  long lastObjectID;			/* オブジェクト ID の最大値 */
  category_t *categories;
  boolean_t consistent;			/* シンクロ ID が一致しているか */
  char codingSystem;			/* 書き出す時のコード系 */
} folder_t;

public boolean_t EqualID( char *f1, char *f2 );
public void ID2Hex( long num, char *hex );
public long Hex2ID( char *hex );

public boolean_t MakeDir( char *path );
public boolean_t Exist( char *path );

public char *TokenAlloc( char *str );
public int Split( char *ptr, char **split, int limit );

public content_t *ContentAlloc( char *id, char *data, int length );
public boolean_t ContentsFree( content_t *content );

public object_t *ObjectAlloc( char *id, char *file, time_t timeStamp );
public boolean_t ObjectFree( object_t *object );

public category_t *CategoryAlloc( char *id, char *directory, char *name );

public boolean_t FolderLoadObject( folder_t *folder,
				  category_t *category, object_t *object );
public folder_t *FolderLoad( char *top, char codingSystem );
public boolean_t FolderSave( folder_t *folder );
public boolean_t FolderFree( folder_t *folder );

public boolean_t FolderDeleteFile( folder_t *folder,
				  category_t *category, object_t *object );

public boolean_t FolderCatalogCheck( object_t *catalog );
public boolean_t FolderCheck( folder_t *folder );

public boolean_t SaveObject( category_t *category,
			    object_t *object, FILE *fp,
			    char codingSystem );
public boolean_t SaveDataObject( object_t *object, FILE *fp );

public char *Local2Str( char *str, char codingSystem );
public char *Str2Local( char *str );

public void DateToStr( char *data, char *str );
public void TimeToStr( char *data, char *str );

#endif /* __FOLDER_H__ */
