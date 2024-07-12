/*
 * folder.h
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: folder.h,v 1.10 2001/03/12 08:47:50 nrt Exp $
 */

#ifndef __FOLDER_H__
#define __FOLDER_H__

#include <time.h>

/*
 * $B%3%s%F%s%D$r3JG<$9$k9=B$BN(B.
 * $B%3%s%F%s%D$O%3%s%F%s%H$N%j%9%H(B.
 */
typedef struct CONTENT_T {
  char id[ 6 + 2 ];			/* $B%U%#!<%k%I(B ID */
  char *data;
  int length;
  struct CONTENT_T *next;
} content_t;

/*
 * $B%+%?%m%0$r3JG<$9$k9=B$BN(B.
 * $B%+%?%m%0$O%*%V%8%'%/%H$N%j%9%H(B.
 */
typedef struct OBJECT_T {
  char id[ 6 + 2 ];			/* $B%*%V%8%'%/%H(B ID */
  boolean_t new;			/* $B%[%9%HB&?75,%*%V%8%'%/%H(B */
  boolean_t updated;			/* $B%[%9%HB&99?7%*%V%8%'%/%H(B */
  boolean_t die;			/* $B%[%9%HB&:o=|%*%V%8%'%/%H(B */
  boolean_t hot;			/* $B=*N;;~$K=q$-=P$7$9$k%*%V%8%'%/%H(B */
  boolean_t alive;			/* $B%+%l%$%IB&$KF1$8(B object id $B$,$"$k(B */
  char *file;
  time_t timeStamp;
  content_t *contents;
  struct OBJECT_T *next;
  struct OBJECT_T *prev;
} object_t;

/*
 * $B%X%G%#%s%0$r3JG<$9$k9=B$BN(B.
 * $B%X%G%#%s%0$O%U%#!<%k%I$N%j%9%H(B.
 */
typedef struct FIELD_T {
  char id[ 6 + 1 ];			/* $B%U%#!<%k%I(B ID */
  char type;				/* $B7?(B */
  char *alias;
  struct FIELD_T *next;
} field_t;

/*
 * $B%U%#!<%k%I$N7?(B
 */
#define F_NONE		'N'
#define F_BINARY	'B'
#define F_CONTINUE	'C'
#define F_STRING	'S'
#define F_DATE		'D'
#define F_TIME		'T'

/*
 * $B%+%F%4%j$r3JG<$9$k9=B$BN(B.
 * $B%+%F%4%j$O%X%G%#%s%0$H%+%?%m%0$+$i9=@.$5$l$k(B.
 */
typedef struct CATEGORY_T {
  char id[ 6 + 2 ];			/* $B%+%F%4%j(B ID */
  boolean_t modified;
  char *directory;
  char *name;
  field_t *heading;			/* $B%X%G%#%s%0(B */
  object_t *catalog;			/* $B%+%?%m%0(B */
  struct CATEGORY_T *next;
} category_t;

/*
 * $B%U%)%k%@Cj>](B
 */
typedef struct {
  char id[ 24 ];			/* $B%U%)%k%@(B ID */
  /*
   * 24bytes $B$NFbLu(B:
   *     o $B%[%9%H<1JL(B ? (4bytes) $B%[%9%H;XDj(B (ex. "$B@.ED(B").
   *     o $B%7%s%/%m(B ID (7bytes) $B%[%9%H;XDj(B.
   *          $B$3$NFbMF$,$$$^$$$AJ,$+$i$J$$(B. 16$B?J$8$c$J$$$_$?$$$@$7(B.
   *     o $B6uGr(B (13bytes) $B8GDj(B.
   */
  char hostId[ 4 ];			/* $B%[%9%H<1JL(B */
  char *top;				/* $B%H%C%W%G%#%l%/%H%j(B */
  long lastObjectID;			/* $B%*%V%8%'%/%H(B ID $B$N:GBgCM(B */
  category_t *categories;
  boolean_t consistent;			/* $B%7%s%/%m(B ID $B$,0lCW$7$F$$$k$+(B */
  char codingSystem;			/* $B=q$-=P$9;~$N%3!<%I7O(B */
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
