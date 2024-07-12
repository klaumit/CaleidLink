/*
 * folder.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: folder.c,v 1.25 2003/06/11 03:13:49 nrt Exp $
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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <import.h>
#include <protocol.h>
#include <base64.h>
#if defined(INTERNAL_ENCODING) && INTERNAL_ENCODING == SHIFT_JIS
#include <encode.h>
#include <decode.h>
#include <guess.h>
#endif
#include <util.h>
#include <begin.h>
#include <folder.h>

#define BUF_SIZE 		1024
#define DATA_SIZE		4096
#define SPLIT_COUNT		16

#define CONF_DIRECTORY		"/conf/"
#define HOST_ID			"CLNK"

#define EXT_BACKUP		".bak"
#define EXT_ORIGINAL		".org"
#define DIR_MASK		0700

public boolean_t EqualID( char *f1, char *f2 )
{
  if( !strncmp( f1, f2, 6 ) )
    return TRUE;
  else
    return FALSE;
}

public boolean_t MakeDir( char *path )
{
  char *ptr;
  struct stat sb;
  char buf[ BUF_SIZE ];

  if( '/' == *path )
    ptr = path + 1;
  else
    ptr = path;

  for( ; ; ){
    ptr = strchr( ptr, '/' );
    if( NULL != ptr && ptr != path && '/' == *( ptr - 1 ) ){
      ptr++;
      continue;
    }
    if( NULL != ptr )
      *ptr = '\0';
    if( 0 > stat( path, &sb ) ){
      if( 0 > mkdir( path, DIR_MASK ) )
	return FALSE;
    } else if( S_IFDIR != ( sb.st_mode & S_IFMT ) ){
      strcpy( buf, path );
      strcat( buf, EXT_ORIGINAL );
      rename( path, buf );
      if( 0 > mkdir( path, DIR_MASK ) )
	return FALSE;
    }
    if( NULL != ptr )
      *ptr++ = '/';
    else
      break;
  }

  return TRUE;
}

public boolean_t Exist( char *path )
{
  char *ptr;
  struct stat sb;
  char buf[ BUF_SIZE ];

  if( NULL != (ptr = strrchr( path, '/' )) ){
    *ptr = '\0';
    if( FALSE == MakeDir( path ) )
      return FALSE;
    *ptr = '/';
  }

  if( 0 == stat( path, &sb ) ){
    strcpy( buf, path );
    strcat( buf, EXT_ORIGINAL );
    if( 0 > rename( path, buf ) ){
      return TRUE;
    }
  }

  return FALSE;
}

/*
 * コード系変換
 */

public char *Local2Str( char *str, char codingSystem )
{
#if defined(INTERNAL_ENCODING) && INTERNAL_ENCODING == SHIFT_JIS
  i_str_t istr[ STR_SIZE ];
  static char mb_buf[ CODE_SIZE ];
  int length;

  if( INTERNAL_ENCODING != codingSystem ){
    length = strlen( str );
    if( length >= STR_SIZE )
      length = STR_SIZE;
    Decode( istr, INTERNAL_ENCODING, str, &length );
    Encode( istr, codingSystem, mb_buf, &length );
    return mb_buf;
  } else {
#endif
    return str;
#if defined(INTERNAL_ENCODING) && INTERNAL_ENCODING == SHIFT_JIS
  }
#endif
}

public char *Str2Local( char *str )
{
#if defined(INTERNAL_ENCODING) && INTERNAL_ENCODING == SHIFT_JIS
  i_str_t istr[ STR_SIZE ];
  static char mb_buf[ CODE_SIZE ];
  int length;
  char codingSystem;

  length = strlen( str );
  codingSystem = GuessCodingSystem( str, length );
  if( length >= STR_SIZE )
    length = STR_SIZE;

  if( INTERNAL_ENCODING != codingSystem ){
    Decode( istr, codingSystem, str, &length );
    Encode( istr, INTERNAL_ENCODING, mb_buf, &length );
    return mb_buf;
  } else {
#endif
    return str;
#if defined(INTERNAL_ENCODING) && INTERNAL_ENCODING == SHIFT_JIS
  }
#endif
}

/*
 * トークン
 */
private char *NextToken( char *str )
{
  /*
   * スペース & タブ & LF を読み飛し
   */
  while( *str && ( 0x20 == *str || 0x09 == *str || 0x0a == *str ) )
    str++;

  return str;
}

private char *NextDelimiter( char *str )
{
  /*
   * スペース & タブ & LF を捜す
   */
  while( *str && ( 0x20 != *str && 0x09 != *str && 0x0a != *str ) )
    str++;

  return str;
}

private boolean_t IsID( char *str )
{
  int count = 0;

  if( 6 != strlen( str ) )
    return FALSE;

  for( count = 0 ; count < 6 ; count++ ){
    if( !(
	  ( *str >= '0' && *str <= '9' )
	  || ( *str >= 'A' && *str <= 'F' )
	  || ( *str >= 'a' && *str <= 'f' )
	  ) )
      return FALSE;
    str++;
  }

  return TRUE;
}

/*
 * NULL 終端された文字列からトークンを切り出す
 */
public int Split( char *ptr, char **split, int limit )
{
  int count;
  char *token;

  for( count = 0 ; count < limit ; count++ ){
    token = NextToken( ptr );
    if( '\0' == *token ){
      *split = NULL;
      return count;
    }

    ptr = NextDelimiter( token );
    if( '\0' != *ptr )
      *ptr++ = '\0';
    *split++ = token;
  }

  return count;
}

/*
 * NULL 終端された文字列からエイリアスとデータを切り出す
 */
private int SplitField( char *ptr, char **split )
{
  char *token;
  int count = 0;

  /*
   * ID 部分
   */
  token = NextToken( ptr );
  if( '\0' == *token ){
    *split = NULL;
    return count;
  }

  count++;
  ptr = NextDelimiter( token );
  if( '\0' != *ptr )
    *ptr++ = '\0';
  *split++ = token;

  if( !IsID( token ) )
    goto skip;

  /*
   * エイリアス部分
   */
  token = NextToken( ptr );
  if( '\0' == *token ){
    *split = '\0';
    return count;
  }

  count++;
  ptr = NextDelimiter( token );
  if( '\0' != *ptr )
    *ptr++ = '\0';
  *split++ = token;

skip:

  /*
   * データ部分
   */
  if( '\0' == *ptr ){
    *split = NULL;
    return count;
  }

  count++;
  *split++ = ptr;

  return count;
}

/*
 * NULL 終端されたトークンをコピーする (メモリは malloc で切り出し).
 */
public char *TokenAlloc( char *str )
{
  char *token;

  if( NULL == (token = (char *)malloc( 1 + strlen( str ) )) )
    return NULL;

  strcpy( token, str );

  return token;
}

/**********************************************************************/

#define X2I( c )							\
  ( ( (c) >= '0' && (c) <= '9' ) ? ( (c) - '0' ) :			\
      ( (c) >= 'A' && (c) <= 'F' ) ? ( (c) - 'A' + 0x0a ) : -1 )

public long Hex2ID( char *hex )
{
  int highH, lowH, highM, lowM, highL, lowL;

  if( 0 > (highH = X2I( hex[ 0 ] ))
     || 0 > (lowH = X2I( hex[ 1 ] ))
     || 0 > (highM = X2I( hex[ 2 ] ))
     || 0 > (lowM = X2I( hex[ 3 ] ))
     || 0 > (highL = X2I( hex[ 4 ] ))
     || 0 > (lowL = X2I( hex[ 5 ] )) )
    return -1;

  return ( highH << 20 ) | ( lowH << 16 )
    | ( highM << 12 ) | ( lowM << 8 )
      | ( highL << 4 ) | lowL;
}

private char *hexTable = "0123456789ABCDEF";

public void ID2Hex( long num, char *hex )
{
  hex[ 0 ] = hexTable[ ( num & 0xf00000 ) >> 20 ];
  hex[ 1 ] = hexTable[ ( num & 0x0f0000 ) >> 16 ];
  hex[ 2 ] = hexTable[ ( num & 0x00f000 ) >> 12 ];
  hex[ 3 ] = hexTable[ ( num & 0x000f00 ) >> 8 ];
  hex[ 4 ] = hexTable[ ( num & 0x0000f0 ) >> 4 ];
  hex[ 5 ] = hexTable[ num & 0x00000f ];
}

/**********************************************************************/

/*
 * メモリの配置, ファイルの読み出し
 */

public content_t *ContentAlloc( char *id, char *data, int length )
{
  content_t *content;

  if( NULL == (content = (content_t *)malloc( sizeof( content_t ) )) )
    return NULL;

  strncpy( content->id, id, 6 );
  content->id[ 6 ] = '\0';

  if( NULL == (content->data = (char *)malloc( 1 + length )) ){
    free( content );
    return NULL;
  }
  memcpy( content->data, data, length );
  content->data[ length ] = '\0';
  content->length = length;

  content->next = NULL;

  return content;
}

public boolean_t ContentsFree( content_t *content )
{
  content_t *next;

  while( content ){
    next = content->next;
    free( content->data );
    free( content );
    content = next;
  }

  return TRUE;
}

public object_t *ObjectAlloc( char *id, char *file, time_t timeStamp )
{
  object_t *object;

  if( NULL == (object = (object_t *)malloc( sizeof( object_t ) )) )
    return NULL;

  object->contents = NULL;

  strncpy( object->id, id, 6 );
  object->id[ 6 ] = '\0';
  object->file = TokenAlloc( file );
  object->timeStamp = timeStamp;

  object->new = FALSE;
  object->updated = FALSE;
  object->die = FALSE;
  object->hot = FALSE;
  object->alive = FALSE;

  object->next = NULL;
  object->prev = NULL;

  return object;
}

public boolean_t ObjectFree( object_t *object )
{
  if( NULL != object->contents )
    ContentsFree( object->contents );
  free( object->file );
  free( object );

  return TRUE;
}

private boolean_t CatalogFree( object_t *object )
{
  object_t *next;

  while( object ){
    next = object->next;
    ObjectFree( object );
    object = next;
  }

  return TRUE;
}

private field_t *FieldAlloc( char *id, char *alias, char type )
{
  field_t *field;

  if( NULL == (field = (field_t *)malloc( sizeof( field_t ) )) )
    return NULL;

  strncpy( field->id, id, 6 );
  field->id[ 6 ] = '\0';
  field->alias = TokenAlloc( alias );
  field->type = type;

  field->next = NULL;

  return field;
}

private boolean_t HeadingFree( field_t *field )
{
  field_t *next;

  while( field ){
    next = field->next;
    free( field->alias );
    free( field );
    field = next;
  }

  return TRUE;
}

public category_t *CategoryAlloc( char *id, char *directory, char *name )
{
  category_t *category;

  if( NULL == (category = (category_t *)malloc( sizeof( category_t ) )) )
    return NULL;

  category->modified = FALSE;

  strncpy( category->id, id, 6 );
  category->id[ 6 ] = '\0';
  category->directory = TokenAlloc( directory );
  category->name = TokenAlloc( Str2Local( name ) );

  category->heading = NULL;
  category->catalog = NULL;

  category->next = NULL;

  return category;
}

private boolean_t CategoriesFree( category_t *category )
{
  category_t *next;

  while( category ){
    next = category->next;
    free( category->directory );
    HeadingFree( category->heading );
    CatalogFree( category->catalog );
    free( category );
    category = next;
  }

  return TRUE;
}

private folder_t *FolderAlloc( char *top, char codingSystem )
{
  folder_t *folder;

  if( NULL == (folder = (folder_t *)malloc( sizeof( folder_t ) )) )
    return NULL;

  /*
   * top directory の保存
   */
  if( NULL == (folder->top = TokenAlloc( top )) ){
    free( folder );
    return NULL;
  }
  folder->codingSystem = codingSystem;

  return folder;
}

public boolean_t FolderFree( folder_t *folder )
{
  free( folder->top );
  CategoriesFree( folder->categories );
  free( folder );

  return TRUE;
}

/**********************************************************************/

/*
 * ファイルの読み出し
 */

private field_t *LoadHeading( char *path )
{
  FILE *fp;
  char *ptr, buf[ BUF_SIZE ];
  char *split[ SPLIT_COUNT ];
  field_t *headingRoot, *headingLast = NULL, *field;

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  for( headingRoot = NULL ; ; ){
    if( '\0' == fgets( buf, BUF_SIZE, fp ) )
      break;
    if( '#' == *buf )
      continue;

    if( 3 > Split( buf, split, SPLIT_COUNT ) || !IsID( split[ 0 ] ) ){
      printf( "caleidlink: invalid heading (%s) in (%s)\n",
	     buf, path );
      continue;
    }

    if( !EqualID( split[ 0 ], split[ 1 ] ) && IsID( split[ 1 ] ) ){
      /*
       * エイリアスはフィールド ID と異なる ID に見えてはいけない.
       */
      printf( "caleidlink: field alias looks like field ID (%s) in (%s)\n",
	     split[ 1 ], path );
      continue;
    }

    ptr = Str2Local( split[ 1 ] );
    if( NULL == (field = FieldAlloc( split[ 0 ], ptr, *split[ 2 ] )) ){
      fprintf( stderr,
	      "caleidlink: cannot alloc field (%s,%s,%s) in (%s)\n",
	      split[ 0 ], ptr, split[ 2 ], path );
      continue;
    }

    /*
     * リストの最後に追加
     */
    if( NULL == headingRoot ){
      /*
       * 見出しが空
       */
      headingRoot = headingLast = field;
    } else {
      headingLast->next = field;
      headingLast = field;
    }
  }

  fclose( fp );

  return headingRoot;
}

private content_t *LoadContents( char *path, field_t *heading )
{
  FILE *fp;
  content_t *contentsRoot = NULL, *contentsLast = NULL, *content;
  field_t *field;
  char *id, *alias, *str;
  int length, fields;
  int ch;
  boolean_t flagWarning = TRUE;
  char *split[ SPLIT_COUNT ];
  char *ptr, buf[ BUF_SIZE ], data[ DATA_SIZE ];

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  for( ; ; ){
    if( NULL == fgets( data, DATA_SIZE, fp ) )
      break;
    if( '#' == *data )
      continue;
    length = strlen( data );
    if( length > 0 && 0x0a == data[ length - 1 ] )
      data[ length - 1 ] = '\0';

    while( 0x20 == (ch = fgetc( fp )) || 0x09 == ch ){
      if( NULL == fgets( buf, BUF_SIZE, fp ) ){
	ch = EOF;
	break;
      }
      length = strlen( buf );
      if( length > 0 && 0x0a == buf[ length - 1 ] )
	buf[ length - 1 ] = '\0';

      if( strlen( data ) + length + 2 < DATA_SIZE ){
	strcat( data, "\r" );
	strcat( data, buf );
      } else if( TRUE == flagWarning ){
	flagWarning = FALSE;
	fprintf( stderr, "warning: too long string: truncated (%s)\n", path );
      }
    }
    ungetc( ch, fp );

    if( 2 > (fields = SplitField( data, split )) ){
      /*
       * 無効なフィールド.
       */
      fprintf( stderr,
	      "caleidlink: invalid content (%s) in (%s)\n",
	      data, path );
      continue;
    }
    if( 2 == fields ){
      /* old: alias, data */
      id = Str2Local( split[ 0 ] );
      alias = id;
      str = split[ 1 ];
    } else {
      /* new: id, alias, data */
      id = split[ 0 ];
      alias = Str2Local( split[ 1 ] );
      str = split[ 2 ];
    }

    /*
     * 見出し Lookup.
     */
    length = strlen( alias );
    for( field = heading ; field ; field = field->next ){
      if( !strncmp( field->alias, alias, length ) ){
	break;
      }
    }
    if( NULL == field ){
      /*
       * エイリアスが定義されていない場合は ID を直接さがす.
       */
      for( field = heading ; field ; field = field->next ){
	if( !strncmp( field->id, id, length ) ){
	  break;
	}
      }
      if( NULL == field ){
	/*
	 * このエイリアスはヘディングの中に定義されていない.
	 */
	printf( "caleidlink: not found %s in heading list in (%s)\n",
		alias, path );
	continue;
      }
    }

    switch( field->type ){
    case F_BINARY:
      if( strlen( str ) > ( 4 * BUF_SIZE / 3 ) ){
	field->id[ 6 ] = '\0';
	fprintf( stderr,
		"warning: content length is too long ID=%s (%s)\n",
		field->id, path );
	continue;
      }
      length = DecodeBase64( str, data );
      if( length > BUF_SIZE / 2 ){
	fprintf( stderr, "warning: too long base64 (%s)\n", path );
	length = BUF_SIZE / 2;
      }
      length = Bin2Shifted( data, length, buf );
      ptr = buf;
      break;
    case F_DATE:
      {
	int year = 0, month = 0, day = 0;
	int len;
	char *base;

#define Year2000( year )	( (year) > 68 ? (year) + 1900 : (year) + 2000 )
#define IsDigit( c )		( (c) >= '0' && (c) <= '9' )

	base = str;

	ptr = base;
	while( *ptr && IsDigit( *ptr ) )
	  ptr++;
	if( '\0' == *ptr || ' ' == *ptr || 0x09 == *ptr ){
	  /*
	   * 全て数字?
	   * 19970101 か 970101 などが想定される.
	   */
	  *ptr = '\0';
	  if( 8 == (len = strlen( base )) ){
	    strncpy( data, base, 4 );
	    data[ 4 ] = '\0';
	    year = atoi( data );
	    base += 4;
	  } else if( 6 == len ){
	    strncpy( data, base, 2 );
	    data[ 2 ] = '\0';
	    year = atoi( data );
	    year = Year2000( year );
	    base += 2;
	  }

	  strncpy( data, base, 2 );
	  data[ 2 ] = '\0';
	  month = atoi( data );
	  base += 2;

	  strncpy( data, base, 2 );
	  data[ 2 ] = '\0';
	  day = atoi( data );
	} else {
	  /*
	   * デリミタあり?
	   */
	  len = ptr - base;
	  strncpy( data, base, len );
	  data[ len ] = '\0';
	  year = atoi( data );
	  if( 2 == len ){
	    year = Year2000( year );
	  }

	  ptr++;
	  base = ptr;
	  while( *ptr && IsDigit( *ptr ) )
	    ptr++;
	  if( '\0' != *ptr ){
	    len = ptr - base;
	    strncpy( data, base, len );
	    data[ len ] = '\0';
	    month = atoi( data );

	    ptr++;
	    base = ptr;
	    while( *ptr && IsDigit( *ptr ) )
	      ptr++;

	    len = ptr - base;
	    strncpy( data, base, len );
	    data[ len ] = '\0';
	    day = atoi( data );
	  }
	}

	if( 0 == year ){
	  month = 1997;
	  fprintf( stderr, "date format is wrong: %s\n", path );
	}
	if( month < 1 || month > 12 ){
	  month = 1;
	  fprintf( stderr, "date format is wrong: %s\n", path );
	}
	if( day < 1 || day > 31 ){
	  day = 1;
	  fprintf( stderr, "date format is wrong: %s\n", path );
	}

	sprintf( data, "%04d%02d%02d  ", year, month, day );
	ptr = data;
	length = strlen( ptr );
      }
      break;
    case F_TIME:
      {
	int hour = -1, minute = -1;
	int len;
	char *base;

	base = str;

	ptr = base;
	while( *ptr && IsDigit( *ptr ) )
	  ptr++;
	if( '\0' == *ptr || ' ' == *ptr || 0x09 == *ptr ){
	  /*
	   * 全て数字?
	   * 0101 などが想定される.
	   */
	  *ptr = '\0';
	  if( 4 == (len = strlen( base )) ){
	    strncpy( data, base, 2 );
	    data[ 2 ] = '\0';
	    hour = atoi( data );
	    base += 2;
	  } else if( 3 == len ){
	    hour = *base - '0';
	    base++;
	  }

	  strncpy( data, base, 2 );
	  data[ 2 ] = '\0';
	  minute = atoi( data );
	} else {
	  /*
	   * デリミタあり?
	   */
	  len = ptr - base;
	  strncpy( data, base, len );
	  data[ len ] = '\0';
	  hour = atoi( data );

	  ptr++;
	  base = ptr;
	  while( *ptr && IsDigit( *ptr ) )
	    ptr++;

	  len = ptr - base;
	  strncpy( data, base, len );
	  data[ len ] = '\0';
	  minute = atoi( data );
	}

	if( hour < 0 || hour > 23 ){
	  hour = 0;
	  fprintf( stderr, "time format is wrong: %s\n", path );
	}
	if( minute < 0 || minute > 59 ){
	  minute = 0;
	  fprintf( stderr, "time format is wrong: %s\n", path );
	}

	sprintf( data, "%02d%02d", hour, minute );
	ptr = data;
	length = strlen( ptr );
      }
      break;
    case F_STRING:
    default:
      ptr = Str2Local( str );
      length = strlen( ptr );

      {
	int idx, moji = 0;
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	boolean_t flagKanji = FALSE;
#endif

	for( idx = 0 ; idx < length ; idx++ ){
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	  if( TRUE == flagKanji ){
	    flagKanji = FALSE;
	    moji++;
#if INTERNAL_ENCODING == SHIFT_JIS
	  } else if( IsShiftJisByte1( ptr[ idx ] ) ){
#endif
#if INTERNAL_ENCODING == BIG_FIVE
	  } else if( IsBig5Byte1( ptr[ idx ] ) ){
#endif
	    flagKanji = TRUE;
	  } else {
#endif
	    moji++;
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	  }
#endif
	}

#define CALEID_CONTENT_LIMIT	970

	if( moji > CALEID_CONTENT_LIMIT ){
	  field->id[ 6 ] = '\0';
	  fprintf( stderr, "warning: long string (%d) (%s) in (%s)\n", moji, field->id, path );
	}
      }
    }

    if( NULL == (content = ContentAlloc( field->id, ptr, length )) ){
      fprintf( stderr, "LoadContents(): cannot alloc content\n" );
      continue;
    }

    /*
     * リストの最後に追加
     */
    if( NULL == contentsRoot ){
      /*
       * コンテンツが空
       */
      contentsRoot = contentsLast = content;
    } else {
      contentsLast->next = content;
      contentsLast = content;
    }
  }

  fclose( fp );

  return contentsRoot;
}

private content_t *LoadDataContents( char *path )
{
  FILE *fp;
  content_t *contentsRoot = NULL, *contentsLast = NULL, *content;
  char *field;
  int length;
  char buf[ BUF_SIZE ], data[ BUF_SIZE ];

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  while( !feof( fp ) ){
    length = fread( buf, 1, 512, fp );
    if( feof( fp ) )
      field = DAT_ADDIN_LAST;
    else
      field = DAT_ADDIN;

    if( length > BUF_SIZE / 2 ){
      fprintf( stderr, "warning: too long base64 (%s)\n", path );
      length = BUF_SIZE / 2;
    }
    length = Bin2Shifted( buf, length, data );

    if( NULL == (content = ContentAlloc( field, data, length )) ){
      fprintf( stderr, "LoadContents(): cannot alloc content\n" );
      continue;
    }

    /*
     * リストの最後に追加
     */
    if( NULL == contentsRoot ){
      /*
       * コンテンツが空
       */
      contentsRoot = contentsLast = content;
    } else {
      contentsLast->next = content;
      contentsLast = content;
    }
  }

  fclose( fp );

  return contentsRoot;
}

private content_t *LoadPlainContents( char *path, FILE *fp,
				      boolean_t flagWarning )
{
  content_t *contentsRoot = NULL, *contentsLast = NULL, *content;
  char *ptr = NULL, *field;
  int length, diff;
  int moji = 0;
  char buf[ BUF_SIZE ], data[ DATA_SIZE ], tmp[ DATA_SIZE ];

  data[ 0 ] = '\0';
  while( !feof( fp ) || '\0' != *data ){
    for( ; ; ){
      if( NULL == fgets( buf, BUF_SIZE - 1, fp ) ){
	strcpy( tmp, data );
	data[ 0 ] = '\0';
	break;
      }
      length = strlen( buf );
      if( length > 0 && 0x0a == buf[ length - 1 ] )
	buf[ length - 1 ] = '\0';

      ptr = Str2Local( buf );
      length = strlen( ptr );

      ptr[ length ] = '\r';
      length++;
      ptr[ length ] = '\0';

      {
	int idx;
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	boolean_t flagKanji = FALSE;
#endif

	for( idx = 0 ; idx < length ; idx++ ){
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	  if( TRUE == flagKanji ){
	    flagKanji = FALSE;
	    moji++;
#if INTERNAL_ENCODING == SHIFT_JIS
	  } else if( IsShiftJisByte1( ptr[ idx ] ) ){
#endif
#if INTERNAL_ENCODING == BIG_FIVE
	  } else if( IsBig5Byte1( ptr[ idx ] ) ){
#endif
	    flagKanji = TRUE;
	  } else {
#endif
	    moji++;
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
	  }
#endif
	}
      }

      if( strlen( data ) + length < DATA_SIZE - 1 ){
	strcat( data, ptr );
	continue;
      } else {
	length = strlen( data );
	diff = DATA_SIZE - 1 - length;
	strncpy( data + length, ptr, diff );
	data[ DATA_SIZE - 1 ] = '\0';
	strcpy( tmp, data );
	strcpy( data, ptr + diff );
	break;
      }
    }

    if( !feof( fp ) || '\0' != *data )
      field = DAT_PLAIN;
    else
      field = DAT_PLAIN_LAST;

/*
    fprintf( stderr, "Content:: %s: [%s]\n", field, tmp );
    fprintf( stderr, "Data:: [%s]\n", data );
*/

    if( NULL == (content = ContentAlloc( field, tmp, strlen( tmp ) )) ){
      fprintf( stderr, "LoadContents(): cannot alloc content\n" );
      continue;
    }

    /*
     * リストの最後に追加
     */
    if( NULL == contentsRoot ){
      /*
       * コンテンツが空
       */
      contentsRoot = contentsLast = content;
    } else {
      contentsLast->next = content;
      contentsLast = content;
    }
  }

#define CALEID_PLAIN_LIMIT	1023

  if( moji > CALEID_PLAIN_LIMIT && TRUE == flagWarning )
    fprintf( stderr, "warning: long string (%d) in (%s)\n", moji, path );

  return contentsRoot;
}

private content_t *LoadMemoContents( char *path )
{
  FILE *fp;
  content_t *content;

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  /*
   * 本体の作成
   */
  content = LoadPlainContents( path, fp, TRUE );

  fclose( fp );

  return content;
}

private content_t *LoadSheetHeader( char *path, FILE *fp )
{
  content_t *content;
  char *ptr, *field;
  int lines = 0;
  int length;
  char buf[ BUF_SIZE ], data[ DATA_SIZE ];

  data[ 0 ] = '\0'; /* ここに格納 */
  for( lines = 0 ; lines < 3 ; ){
    if( NULL == fgets( buf, BUF_SIZE, fp ) )
      break;
    length = strlen( buf );
    if( length > 0 && 0x0a == buf[ length - 1 ] ){
      buf[ length - 1 ] = '\0';
    }

    ptr = Str2Local( buf );
    length = strlen( ptr );

    if( 0 == lines && length > 32 ){
      fprintf( stderr, "warning: too long title in sheet header (32bytes): truncated in (%s)\n", path );
      ptr[ 32 ] = '\0';
    }

    if( strlen( data ) + length + 2 < DATA_SIZE ){
      strcat( data, ptr );
      strcat( data, "\r" );
      lines++;
    } else {
      fprintf( stderr, "warning: too long string in sheet: truncated in (%s)\n", path );
      return NULL;
    }
  }

  if( 3 != lines ){
    fprintf( stderr, "error: easy sheet header is corrupted in (%s)\n", path );
    return NULL;
  }

  field = "000010"; /* タイトル */

  if( NULL == (content = ContentAlloc( field, data, strlen( data ) )) ){
    fprintf( stderr, "LoadSheetHeader(): cannot alloc content\n" );
    return NULL;
  }

  return content;
}

private content_t *LoadSheetContents( char *path )
{
  FILE *fp;
  content_t *content;

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  /*
   * ヘッダーの生成
   */
  if( NULL == (content = LoadSheetHeader( path, fp )) ){
    fclose( fp );
    return NULL;
  }

  /*
   * 本体の作成
   */
  content->next = LoadPlainContents( path, fp, FALSE );

  fclose( fp );

  return content;
}

public boolean_t FolderLoadObject( folder_t *folder,
				  category_t *category, object_t *object )
{
  char buf[ BUF_SIZE ];

  if( NULL != object->contents )
    return TRUE;

  /*
   * ファイル名の組み立て
   */
  strcpy( buf, folder->top );
  strcat( buf, "/" );
  strcat( buf, category->directory );
  strcat( buf, "/" );
  strcat( buf, object->file );

  /*
   * ファイル内容の読み込み
   */
  if( EqualID( category->id, CAT_ADDIN ) ){		/* addin */
    object->contents = LoadDataContents( buf );
  } else if( EqualID( category->id, CAT_SHEET ) ){	/* sheet */
    object->contents = LoadSheetContents( buf );
  } else if( EqualID( category->id, CAT_NOTE_MEMO )	/* memo */
	    || EqualID( category->id, CAT_NOTE_DATA ) ){/* data */
    object->contents = LoadMemoContents( buf );
  } else {
    object->contents = LoadContents( buf, category->heading );
  }

  if( NULL == object->contents )
    return FALSE;
  else
    return TRUE;
}

private object_t *LoadObject( folder_t *folder, category_t *category,
			     char *id, char *file, time_t timeStamp )
{
  object_t *object;
  struct stat sb;
  char buf[ BUF_SIZE ];

  if( NULL == (object = ObjectAlloc( id, file, timeStamp )) )
    return NULL;

  /*
   * ファイル名の組み立て
   */
  strcpy( buf, folder->top );
  strcat( buf, "/" );
  strcat( buf, category->directory );
  strcat( buf, "/" );
  strcat( buf, file );

  if( 0 > stat( buf, &sb ) ){
    /*
     * ファイルが無い.
     */
    object->die = TRUE;		/* ファイルが削除されたことを意味する */
  } else {
    if( sb.st_mtime > object->timeStamp ){
      category->modified = TRUE;
      object->timeStamp = sb.st_mtime;
      object->updated = TRUE;	/* ファイルが更新されたことを意味する */
      FolderLoadObject( folder, category, object );
    }
  }

  return object;
}

private object_t *LoadCatalog( folder_t *folder,
			      category_t *category, char *path )
{
  FILE *fp;
  char buf[ BUF_SIZE ];
  char dir[ BUF_SIZE ];
  char *split[ SPLIT_COUNT ];
  object_t *catalogRoot = NULL, *catalogLast = NULL, *object;
  DIR *dirp;
  struct dirent *dp;
  int length;
  char *ptr;

  if( NULL != (fp = fopen( path, "r" )) ){
    for( catalogRoot = NULL ; ; ){
      if( NULL == fgets( buf, BUF_SIZE, fp ) )
	break;
      if( '#' == *buf )
	continue;

      if( 3 > Split( buf, split, SPLIT_COUNT ) || !IsID( split[ 0 ] ) ){
	fprintf( stderr,
		"caleidlink: invalid catalog (%s) in (%s)\n",
		buf, path );
	continue;
      }

      if( NULL == (object = LoadObject( folder,
				       category,
				       split[ 0 ],	/* id */
				       split[ 1 ],	/* file */
				       atol( split[ 2 ] ) )) ){ /* time stamp */
	fprintf( stderr,
		"caleidlink: cannot load object (%s,%s) in (%s)\n",
		split[ 0 ], split[ 1 ], path );
	continue;
      }

      /*
       * リストの最後に追加
       */
      if( NULL == catalogRoot ){
	/*
	 * カタログが空
	 */
	catalogRoot = catalogLast = object;
	catalogRoot->prev = object;
      } else {
	catalogLast->next = object;
	object->prev = catalogLast;
	catalogLast = object;
	catalogRoot->prev = object;
      }
    }

    fclose( fp );
  }

  /*
   * 新規オブジェクトの追加
   */
  strcpy( dir, folder->top );
  strcat( dir, "/" );
  strcat( dir, category->directory );

  if( NULL == (dirp = opendir( dir )) ){
/*
    fprintf( stderr, "caleidlink: cannot open directory (%s)\n", dir );
*/
  } else {
    while( NULL != (dp = readdir( dirp )) ){
/*
      if( !( DT_REG == dp->d_type || DT_LNK == dp->d_type ) )
	continue;
*/
      if( !strcmp( dp->d_name, "." ) || !strcmp( dp->d_name, ".." ) )
	continue;
      for( object = catalogRoot ; object ; object = object->next ){
	if( !strcmp( dp->d_name, object->file ) ){
	  break;
	}
      }
      if( NULL == object ){
	/*
	 * ファイル名はオブジェクト ID に見えてはいけない.
	 */
	if( IsID( dp->d_name ) ){
	  fprintf( stderr,
		  "caleidlink: filename looks like object ID (%s/%s)\n",
		  dir, dp->d_name );
	  continue;
	}
	/*
	 * カレイドで消去されたファイルと #*, *~ は含めない.
	 */
	length = strlen( dp->d_name );
	if( '#' == dp->d_name[ 0 ]
	   || ( length > 0 && '~' == dp->d_name[ length - 1 ] ) ){
	  continue;
	} else if( NULL != (ptr = strrchr( dp->d_name, '.' )) ){
	  if( !strcmp( ptr, EXT_BACKUP ) || !strcmp( ptr, EXT_ORIGINAL ) )
	    continue;
	}
	/*
	 * 新規オブジェクトをリストの最後に追加.
	 */
	if( NULL == (object = LoadObject( folder,
					 category,
					 NEW_OBJECT_ID,	/* id */
					 dp->d_name,	/* file */
					 0 )) ){	/* time stamp */
	  fprintf( stderr,
		  "caleidlink: cannot load new object in (%s/%s)\n",
		  dir, dp->d_name );
	} else {
	  object->new = TRUE;		/* 新規オブジェクト */
	  if( NULL == catalogRoot ){
	    /*
	     * カタログが空
	     */
	    catalogRoot = catalogLast = object;
	    catalogRoot->prev = object;
	  } else {
	    catalogLast->next = object;
	    object->prev = catalogLast;
	    catalogLast = object;
	    catalogRoot->prev = object;
	  }
	}
      }
    }
    closedir( dirp );
  }

  return catalogRoot;
}

private category_t *LoadCategories( folder_t *folder )
{
  FILE *fp;
  struct stat sb;
  category_t *categoriesRoot, *categoriesLast = NULL, *category;
  char *split[ SPLIT_COUNT ];
  char path[ BUF_SIZE ], *dir;
  char buf[ BUF_SIZE ];

  strcpy( path, folder->top );
  strcat( path, CONF_DIRECTORY );
  dir = path + strlen( path );

  strcpy( dir, "categories" );

  if( NULL == (fp = fopen( path, "r" )) )
    return NULL;

  for( categoriesRoot = NULL ; ; ){
    if( NULL == fgets( buf, BUF_SIZE, fp ) )
      break;
    if( '#' == *buf )
      continue;

    if( 3 > Split( buf, split, SPLIT_COUNT ) || !IsID( split[ 0 ] ) ){
      fprintf( stderr,
	      "caleidlink: invalid category (%s) in (%s)\n",
	      buf, path );
      continue;
    }

      
    if( NULL == (category = CategoryAlloc( split[ 0 ], split[ 1 ], split[ 2 ] )) ){
      fprintf( stderr,
	      "caleidlink: cannot alloc category (%s,%s)\n",
	      split[ 0 ], split[ 1 ] );
      continue;
    }

    /*
     * 見出しの読み込み
     */
    strcpy( dir, category->directory );
    strcat( dir, ".heading" );

    /* ひょっとすると NULL が返る. */
    category->heading = LoadHeading( path );

    /*
     * カタログの読み込み
     */
    strcpy( dir, category->directory );
    strcat( dir, ".catalog" );

    /* ひょっとすると NULL が返る. */
    category->catalog = LoadCatalog( folder,
				    category,
				    path );

    if( NULL != category->catalog ){
      /*
       * カタログ存在時, カテゴリのディレクトリ・存在チェック.
       */
      strcpy( buf, folder->top );
      strcat( buf, "/" );
      strcat( buf, category->directory );
      if( 0 > stat( buf, &sb ) || S_IFDIR != ( sb.st_mode & S_IFMT ) ){
	fprintf( stderr,
		"caleidlink: invalid directory (%s) in (%s%scategories)\n",
		category->directory,
		folder->top,
		CONF_DIRECTORY );
	CatalogFree( category->catalog );
	continue;
      }
    }

    /*
     * リストの最後に追加
     */
    if( NULL == categoriesRoot ){
      /*
       * カテゴリが空
       */
      categoriesRoot = categoriesLast = category;
    } else {
      categoriesLast->next = category;
      categoriesLast = category;
    }
  }

  fclose( fp );

  return categoriesRoot;
}

/*
 * フォルダ抽象の読み込み.
 * 識別は top ディレクトリです. 同じ top に異なるフォルダは作れません.
 */
public folder_t *FolderLoad( char *top, char codingSystem )
{
  folder_t *folder;
  FILE *fp;
  char path[ BUF_SIZE ], *dir;
  char buf[ BUF_SIZE ];

  if( NULL == (folder = FolderAlloc( top, codingSystem )) )
    return NULL;

  /*
   * harmful for CGI use
   */
  /*
  printf( "Loading: %s\n", top );
  */

  strcpy( path, top );
  strcat( path, CONF_DIRECTORY );
  dir = path + strlen( path );

  /*
   * ホスト ID の読み込み
   */
  strcpy( dir, "host.id" );
  if( NULL == (fp = fopen( path, "r" )) || NULL == fgets(buf, BUF_SIZE, fp) ){
    strncpy( folder->hostId, HOST_ID, 4 ); 
  } else {
    strncpy( folder->hostId, buf, 4 );
  }
  if( NULL != fp )
    fclose( fp );

  /*
   * セッション ID の読み込み
   */
  strcpy( dir, "session.id" );
  if( NULL == (fp = fopen( path, "r" )) || NULL == fgets(buf, BUF_SIZE, fp) ){
    folder->lastObjectID = 0;
    memset( folder->id, 0x20, 24 );
  } else {
    strncpy( folder->id, buf, 24 );
  }
  if( NULL != fp )
    fclose( fp );

  /*
   * オブジェクト ID の読み込み
   */
  strcpy( dir, "object.id" );
  if( NULL == (fp = fopen( path, "r" )) || NULL == fgets(buf, BUF_SIZE, fp) ){
    folder->lastObjectID = 0;
    memset( folder->id, 0x20, 24 );
  } else {
    folder->lastObjectID = Hex2ID( buf );
  }
  if( NULL != fp )
    fclose( fp );

  /*
   * カテゴリの読み込み
   */
  folder->categories = LoadCategories( folder );

  if( NULL == folder->categories ){
    folder->lastObjectID = 0;
    memset( folder->id, 0x20, 24 );
  }

/*  printf( "Loading done: %s\n", top );*/

  return folder;
}

/**********************************************************************/

public void DateToStr( char *data, char *str )
{
  strncpy( str, data, 4 );
  strncpy( str + 5, data + 4, 2 );
  strncpy( str + 8, data + 6, 2 );
  str[ 4 ] = '/';
  str[ 7 ] = '/';
  str[ 10 ] = '\0';
}

public void TimeToStr( char *data, char *str )
{
  strncpy( str, data, 2 );
  strncpy( str + 3, data + 2, 2 );
  str[ 2 ] = ':';
  str[ 5 ] = '\0';
}

public boolean_t SaveObject( category_t *category,
			    object_t *object, FILE *fp,
			    char codingSystem )
{
  content_t *content;
  field_t *field, *fieldPrev;
  char type, pendingChar = '\0';
  boolean_t flagPending = FALSE;
  boolean_t flagPlain = FALSE;
  boolean_t flagHeading = FALSE;
  int length;
  char *alias, *dat, *ptr, buf[ BUF_SIZE ], data[ BUF_SIZE ];

  category->id[ 6 ] = '\0';
  if( EqualID( category->id, "600000" )		/* sheet */
     || EqualID( category->id, "000000" )	/* memo */
     || EqualID( category->id, "00F000" ) ){	/* data */
    flagPlain = TRUE;
  }

  for( content = object->contents ; content ; content = content->next ){
    /*
     * バラバラに来るため, 順序をつけがたいので, とりあえず, こうしてます.
     */
    fieldPrev = NULL;
    for( field = category->heading ; field ; field = field->next ){
      if( EqualID( field->id, content->id ) ){
	break;
      }
      fieldPrev = field;
    }
    if( NULL == field ){
      /*
       * heading の最後に新しい field を追加.
       */
      content->id[ 6 ] = '\0';
      if( TRUE == flagPlain || EqualID( content->id, DAT_PLAIN ) ){
	type = F_CONTINUE;
      } else if( EqualID( content->id, DAT_QFORM )
		|| EqualID( content->id, DAT_QFORM_LAST )
		){
	type = F_BINARY;
      } else if( EqualID( content->id, DAT_BEGIN_DATE )
		|| EqualID( content->id, DAT_END_DATE )
		|| EqualID( content->id, DAT_EXEC_DATE )
		|| EqualID( content->id, DAT_ALARM_DATE ) ){
	type = F_DATE;
      } else if( EqualID( content->id, DAT_BEGIN_TIME )
		|| EqualID( content->id, DAT_END_TIME )
		|| EqualID( content->id, DAT_ALARM_TIME ) ){
	type = F_TIME;
      } else {
	type = F_STRING;
      }
      if( NULL == (field = FieldAlloc( content->id, content->id, type )) ){
	fprintf( stderr, "caleidlink: cannot alloc field\n" );
	continue;
      }
      if( NULL == fieldPrev ){
	category->heading = field;
      } else {
	fieldPrev->next = field;
      }
    }
    switch( field->type ){
    case F_BINARY:
      length = Shifted2Bin( content->data, content->length, buf );
      EncodeBase64( buf, length, data );
      fprintf( fp, "%s %s %s\n", field->id, field->alias, data );
      break;
    case F_DATE:
      alias = TokenAlloc( Local2Str( field->alias, codingSystem ) );
      DateToStr( content->data, data );
      fprintf( fp, "%s %s %s\n", field->id, alias, data );
      free( alias );
      break;
    case F_TIME:
      alias = TokenAlloc( Local2Str( field->alias, codingSystem ) );
      strncpy( data, content->data, 2 );
      strncpy( data + 3, content->data + 2, 2 );
      data[ 2 ] = ':';
      data[ 5 ] = '\0';
      fprintf( fp, "%s %s %s\n", field->id, alias, data );
      free( alias );
      break;
    case F_STRING:
    case F_CONTINUE:
    default:
      if( TRUE == flagPending ){
	char *ptr;

	flagPending = FALSE;
	ptr = content->data;
	content->data = (char *)malloc( 2 + content->length );
	content->data[ 0 ] = pendingChar;
	memcpy( content->data + 1, ptr, content->length );
	content->length++;
	content->data[ content->length ] = '\0';
	free( ptr );
      }
#if defined(INTERNAL_ENCODING) && (INTERNAL_ENCODING == SHIFT_JIS || INTERNAL_ENCODING == BIG_FIVE)
      {
	int idx;
	boolean_t flagKanji = FALSE;

	for( idx = 0 ; idx < content->length ; idx++ ){
	  if( TRUE == flagKanji ){
	    flagKanji = FALSE;
#if INTERNAL_ENCODING == SHIFT_JIS
	  } else if( IsShiftJisByte1( content->data[ idx ] ) ){
#endif
#if INTERNAL_ENCODING == BIG_FIVE
	  } else if( IsBig5Byte1( content->data[ idx ] ) ){
#endif
	    flagKanji = TRUE;
	  }
	}
	if( TRUE == flagKanji ){
	  content->length--;
	  flagPending = TRUE;
	  pendingChar = content->data[ content->length ];
	  content->data[ content->length ] = '\0';
	}
      }
#endif

      if( F_CONTINUE == field->type ){
	field_t *find;

	for( find = category->heading ; find ; find = find->next ){
	  if( EqualID( find->id, "000000" ) )
	    break;
	}
	if( NULL != find ){
	  alias = TokenAlloc( Local2Str( find->alias, codingSystem ) );
	} else {
	  alias = TokenAlloc( Local2Str( field->alias, codingSystem ) );
	}
      } else {
	alias = TokenAlloc( Local2Str( field->alias, codingSystem ) );
      }

      dat = content->data;
      if( NULL == (ptr = strchr( dat, '\r' )) ){
	dat = Local2Str( dat, codingSystem );
	if( FALSE == flagPlain && FALSE == flagHeading ){
	  if( EqualID( field->id, "000001" ) )
	    flagHeading = TRUE;
	  else
	    flagHeading = FALSE;
	  fprintf( fp, "%s %s ", field->id, alias );
	}
	fprintf( fp, "%s", dat );
	if( F_CONTINUE != field->type )
	  fprintf( fp, "\n" );
      } else {
	*ptr = '\0';
	dat = Local2Str( dat, codingSystem );
	if( FALSE == flagPlain && FALSE == flagHeading ){
	  if( EqualID( field->id, "000001" ) )
	    flagHeading = TRUE;
	  else
	    flagHeading = FALSE;
	  fprintf( fp, "%s %s ", field->id, alias );
	}
	fprintf( fp, "%s\n", dat );
	dat = ptr + 1;
	while( NULL != (ptr = strchr( dat, '\r' )) ){
	  *ptr = '\0';
	  dat = Local2Str( dat, codingSystem );
	  if( FALSE == flagPlain )
	    fprintf( fp, "\t" );
	  fprintf( fp, "%s\n", dat );
	  dat = ptr + 1;
	}
	dat = Local2Str( dat, codingSystem );
	if( FALSE == flagPlain )
	  fprintf( fp, "\t" );
	fprintf( fp, "%s", dat );
	if( F_CONTINUE != field->type )
	  fprintf( fp, "\n" );
      }
      free( alias );
    }
  }

  return TRUE;
}

public boolean_t SaveDataObject( object_t *object, FILE *fp )
{
  content_t *content;
  int length;
  char buf[ BUF_SIZE ];

  for( content = object->contents ; content ; content = content->next ){
    length = Shifted2Bin( content->data, content->length, buf );
    fwrite( buf, 1, length, fp );
  }

  return TRUE;
}

private boolean_t SaveCategory( folder_t *folder, category_t *category )
{
  object_t *object;
  field_t *field;
  FILE *fp;
  struct stat sb;
  char *ptr, path[ BUF_SIZE ], *dir;

  /*
   * オブジェクトの書き出し
   */
  strcpy( path, folder->top );
  strcat( path, "/" );
  strcat( path, category->directory );
  strcat( path, "/" );

  dir = path + strlen( path );

  MakeDir( path );

  for( object = category->catalog ; object ; object = object->next ){
    if( TRUE == object->hot ){
      object->hot = FALSE;
      strcpy( dir, object->file );
      if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
	fprintf( stderr, "caleidlink: cannot open (%s)\n", path );
	continue;
      }
      if( EqualID( category->id, "700000" ) ){	/* addin */
	SaveDataObject( object, fp );
      } else {
	SaveObject( category, object, fp, folder->codingSystem );
      }
      fclose( fp );

      if( 0 > stat( path, &sb ) ){
	/*
	 * ファイルが無い. はずはない.
	 */
	fprintf( stderr, "caleidlink: not found (%s)\n", path );
      } else {
	object->timeStamp = sb.st_mtime;
      }
    }
  }

  /*
   * 見出しの書き出し
   */
  strcpy( path, folder->top );
  strcat( path, CONF_DIRECTORY );
  strcat( path, category->directory );
  dir = path + strlen( path );

  strcpy( dir, ".heading" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    fprintf( stderr, "caleidlink: cannot open heading file (%s)\n", path );
    return FALSE;
  }

  for( field = category->heading ; field ; field = field->next ){
    field->id[ 6 ] = '\0';
    ptr = Local2Str( field->alias, folder->codingSystem );
    fprintf( fp, "%s %s %c\n", field->id, ptr, field->type );
  }

  fclose( fp );

  /*
   * カタログの書き出し
   */
  strcpy( dir, ".catalog" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    fprintf( stderr, "caleidlink: cannot open catalog file (%s)\n", path );
    return FALSE;
  }

  for( object = category->catalog ; object ; object = object->next ){
    object->id[ 6 ] = '\0';
    fprintf( fp, "%s %s %ld\n", object->id, object->file, (long)object->timeStamp );
  }

  return TRUE;
}

/*
 * フォルダ抽象の書き出し
 * 必要となるディレクトリは勝手に作ります (作ろうと試みます).
 */
public boolean_t FolderSave( folder_t *folder )
{
  category_t *category;
  FILE *fp;
  char path[ BUF_SIZE ], *dir;
  char buf[ BUF_SIZE ];

  if( NULL == folder )
    return FALSE;

  printf( "Saving: %s\n", folder->top );

  strcpy( path, folder->top );
  strcat( path, CONF_DIRECTORY );

  dir = path + strlen( path );

  MakeDir( path );

  /*
   * セッション ID の書き出し
   */
  strcpy( dir, "session.id" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    return FALSE;
  }

  strncpy( buf, folder->id, 24 );
  buf[ 24 ] = '\0';

  fprintf( fp, "%s\n", buf );

  fclose( fp );

  /*
   * ホスト ID の書き出し
   */
  strcpy( dir, "host.id" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    return FALSE;
  }

  strncpy( buf, folder->hostId, 4 );
  buf[ 4 ] = '\0';

  fprintf( fp, "%s\n", buf );

  fclose( fp );

  /*
   * オブジェクト ID の書き出し
   */
  strcpy( dir, "object.id" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    return FALSE;
  }

  ID2Hex( folder->lastObjectID, buf );
  buf[ 6 ] = '\0';

  fprintf( fp, "%s\n", buf );

  fclose( fp );

  /*
   * カテゴリーの書き出し
   */
  strcpy( dir, "categories" );

  if( Exist( path ) || NULL == (fp = fopen( path, "w" )) ){
    return FALSE;
  }

  for( category = folder->categories ; category ; category = category->next ){
    category->id[ 6 ] = '\0';
    fprintf( fp, "%s %s %s\n",
	    category->id,
	    category->directory,
	    Local2Str( category->name, folder->codingSystem ) );
  }

  fclose( fp );

  /*
   * 各カテゴリのカタログを出力
   */
  for( category = folder->categories ; category ; category = category->next ){
    if( TRUE == category->modified )
      SaveCategory( folder, category );
  }

  /*
  printf( "Saving: %s done\n", folder->top );
  */

  return TRUE;
}

/**********************************************************************/

public boolean_t FolderDeleteFile( folder_t *folder,
				  category_t *category, object_t *object )
{
  char src[ BUF_SIZE ], dst[ BUF_SIZE ];

  strcpy( src, folder->top );
  strcat( src, "/" );
  strcat( src, category->directory );
  strcat( src, "/" );
  strcat( src, object->file );

  strcpy( dst, src );
  strcat( dst, EXT_BACKUP );

  if( 0 > rename( src, dst ) )
    return FALSE;
  else
    return TRUE;
}

/**********************************************************************/

public boolean_t FolderCatalogCheck( object_t *catalog )
{
  char buf[ BUF_SIZE ];

  for( ; catalog ; catalog = catalog->next ){
    strcpy( buf, catalog->id );
    buf[ 6 ] = '\0';
    fprintf( stderr, "%s %s\n", buf, catalog->file );
  }

  return TRUE;
}

public boolean_t FolderCheck( folder_t *folder )
{
  category_t *category;
  object_t *object;
  content_t *content;

  for( category = folder->categories ; category ; category = category->next ){
    fprintf( stderr, "Category: %s\n", category->id );
    for( object = category->catalog ; object ; object = object->next ){
      fprintf( stderr, "Catalog:\n" );
      for( content = object->contents ; content ; content = content->next ){
	fprintf( stderr, "%s %s\n", content->id, content->data );
      }
    }
  }

  return TRUE;
}
