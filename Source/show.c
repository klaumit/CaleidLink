/*
 * show.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: show.c,v 1.11 2003/06/11 03:13:50 nrt Exp $
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
#include <string.h>

#include <import.h>
#include <folder.h>
#include <protocol.h>
#include <begin.h>
#include <show.h>

#define BUF_SIZE 	1024

static char outputCodingSystem;

private boolean_t ShowContent( category_t *category, object_t *object )
{
  if( EqualID( category->id, "700000" ) ){	/* addin */
    SaveDataObject( object, stdout );
  } else {
    SaveObject( category,
	       object, stdout,
	       outputCodingSystem );
  }

  return TRUE;
}

private char delimiterChar = ',';

private void ShowDelimiter()
{
  printf( "%c", delimiterChar );
}

private boolean_t ShowContentHeading( object_t *object, char *fieldID )
{
  content_t *content;
  char *dat, *ptr, buf[ BUF_SIZE ];

  if( EqualID( fieldID, DAT_BEGIN_DATE ) || EqualID( fieldID, DAT_END_DATE )
     || EqualID( fieldID, DAT_EXEC_DATE ) || EqualID( fieldID, DAT_ALARM_DATE ) ){
    for( content = object->contents ; content ; content = content->next ){
      if( EqualID( content->id, fieldID ) ){
	DateToStr( content->data, buf );
	ShowDelimiter();
	printf( "%s", buf );
	return TRUE;
      }
    }
  } else if( EqualID( fieldID, DAT_BEGIN_TIME ) || EqualID( fieldID, DAT_END_TIME )
	    || EqualID( fieldID, DAT_ALARM_TIME ) ){
    for( content = object->contents ; content ; content = content->next ){
      if( EqualID( content->id, fieldID ) ){
	TimeToStr( content->data, buf );
	ShowDelimiter();
	printf( "%s", buf );
	return TRUE;
      }
    }
  } else {
    for( content = object->contents ; content ; content = content->next ){
      if( EqualID( content->id, fieldID ) ){
	dat = Local2Str( content->data, outputCodingSystem );
	if( NULL != (ptr = strchr( dat, 0x0d )) )
	  *ptr = '\0';
	ShowDelimiter();
	printf( "%s", dat );
	return TRUE;
      }
    }
  }

  return FALSE;
}

private void ShowHeading( object_t *object, field_t *heading )
{
  printf( "%s", object->file );
  for( ; heading ; heading = heading->next ){
    if( EqualID( heading->id, DAT_QFORM ) || EqualID( heading->id, DAT_QFORM_LAST ) ){
      continue;
    }
    if( TRUE == ShowContentHeading( object, heading->id ) ){
      if( EqualID( heading->id, DAT_PLAIN ) )
	break;
    } else {
      if( EqualID( heading->id, DAT_PLAIN ) )
	continue;
      ShowDelimiter();
    }
  }
  printf( "\n" );
}

private boolean_t ShowObject( folder_t *folder, char *categoryName, char *objectName )
{
  category_t *category;
  object_t *object;

  for( category = folder->categories ; category ; category = category->next ){
    category->id[ 6 ] = '\0';
    if( EqualID( categoryName, category->id ) || !strcmp( categoryName, category->directory ) ){
      for( object = category->catalog ; object ; object = object->next ){
	if( EqualID( objectName, object->id ) || !strcmp( objectName, object->file ) ){
	  if( TRUE == FolderLoadObject( folder, category, object ) )
	    ShowContent( category, object );
	  return TRUE;
	}
      }
      break;
    }
  }

  return FALSE;
}

private boolean_t ShowCategoryList( folder_t *folder, char *categoryName )
{
  category_t *category;
  object_t *object;

  for( category = folder->categories ; category ; category = category->next ){
    category->id[ 6 ] = '\0';
    if( EqualID( categoryName, category->id ) || !strcmp( categoryName, category->directory ) ){
      for( object = category->catalog ; object ; object = object->next ){
	if( TRUE == FolderLoadObject( folder, category, object ) ){
	  ShowHeading( object, category->heading );
	  ContentsFree( object->contents );
	  object->contents = NULL;
	}
      }
      return TRUE;
    }
  }

  return FALSE;
}

private boolean_t ShowCategoryNames( folder_t *folder )
{
  category_t *category;

  for( category = folder->categories ; category ; category = category->next ){
    category->id[ 6 ] = '\0';
    printf( "%s%c%s%c%s\n",
	   category->id,
	   delimiterChar,
	   category->directory,
	   delimiterChar,
	   Local2Str( category->name, outputCodingSystem ) );
  }

  return TRUE;
}

public boolean_t ShowFolder( char *file, char *category, char *object,
			    char codingSystem, char delimiter )
{
  folder_t *folder;

  outputCodingSystem = codingSystem;
  delimiterChar = delimiter;

  if( NULL == (folder = FolderLoad( file, codingSystem )) ){
    fprintf( stderr, "caleidlink: cannot open folder (%s)\n", file );
    return FALSE;
  }

  if( NULL == category ){
    ShowCategoryNames( folder );
  } else if( NULL == object ){
    if( FALSE == ShowCategoryList( folder, category ) ){
      fprintf( stderr, "no such category\n" );
      return FALSE;
    }
  } else {
    if( FALSE == ShowObject( folder, category, object ) ){
      fprintf( stderr, "no such object\n" );
      return FALSE;
    }
  }

  return TRUE;
}
