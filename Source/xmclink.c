/*
 * xmclink.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: xmclink.c,v 1.24 2003/06/11 03:14:24 nrt Exp $
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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/BulletinB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawnB.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <Xm/DialogS.h>
#include <X11/xpm.h>

#include <import.h>
#include <decode.h>
#include <encode.h>
#include <protocol.h>
#include <folder.h>
#include <base64.h>
#include <util.h>
#include <version.h>
#include <begin.h>

#define ROLLING_INTERVAL 50

#define EDIT_HEIGHT	400

#define CATALOG_WIDTH	400
#define CATALOG_HEIGHT	400

#define CIRCLE_WIDTH	8

#define CONFIG_FILE	".clink"
#define CODING_SYSTEM	"e"	/* i:iso-2022-jp, e:euc-japan, s:shit-jis */

#define BUF_SIZE 	1024
#define SPLIT_COUNT	16

#define SYNCHRONIZE	0
#define BACKUP		1
#define RESTORE		2

#define MODES		3
#define SPEEDS		4

private Widget shell;

private XmString modeString = NULL;
private XmString folderString;

private Widget speedWidget[ SPEEDS ];
private Widget modeWidget[ MODES ];
private Widget syncModeWidget[ 2 ];
private Widget imageSizeWidget[ 2 ];
private Widget autoSaveWidget[ 2 ];

private Widget animWidget;
private GC animGC;

private Widget msgWidget;
private Widget modeLabelWidget;
private Widget folderLabelWidget;

private int pixIndex = 0;
#define PIX_COUNT	17

typedef struct {
  char *name;
  int speed;
} speed_name_t;

private speed_name_t speedName[ SPEEDS ] = {
  { "4800bps", 4800 },
  { "9600bps", 9600 },
  { "19200bps", 19200 },
  { "38400bps", 38400 }
};

#define DEFAULT_FOLDER	"caleid"
#define DEFAULT_DEVICE	"/dev/ttyS0"
#define DEFAULT_SPEED	38400
#define DEFAULT_BACKUP	"backup"

private char *modeLabel[ MODES ] = {
  "HotSync",
  "Backup",
  "Restore"
};

private XtSignalId sigId;
private XtInputId inpId;

private int child = 0;
private int childFd = -1;
private int childStatus;

private int mode;
private int newMode;
private char *folderName;
private char *backupName;
private char *deviceName;
private long speed;
private int autoSave = 0;

#define MANUAL_SAVE	1

#define IMAGE_SMALL	0
#define IMAGE_LARGE	1

#define PASSIVE_SYNC		1
#ifdef ACTIVE_SYNC
#define DEFAULT_SYNCMODE	0
#define DEFAULT_IMAGE_SIZE	IMAGE_SMALL
#else
#define DEFAULT_SYNCMODE	PASSIVE_SYNC
#define DEFAULT_IMAGE_SIZE	IMAGE_LARGE
#endif

private int syncMode = DEFAULT_SYNCMODE;
private int imageSize = DEFAULT_IMAGE_SIZE;

private folder_t *folder = NULL;

private boolean_t flagTerminate = FALSE;

#define CHILD_KILLED	0

#ifndef PIXMAP_DIRECTORY
#define PIXMAP_DIRECTORY	"/usr/X11R6/include/X11/pixmaps"
#endif

Pixmap pixClink[ PIX_COUNT ];
Pixmap pixEmboss;

/**********************************************************************/

private void RefreshCategories( void );

/**********************************************************************/

private boolean_t ForkProcess()
{
  int pid;
  int fds[ 2 ];

  if( 0 > pipe( fds ) ){
    fprintf( stderr, "xmclink: cannot create pipe\n" );
    exit( 1 );
  }

  switch( pid = fork() ){
  case -1:
    fprintf( stderr, "xmclink: cannot fork\n" );
    exit( 1 );
  case 0:
    {
      /* child */

      int argc;
      char *argv[ 16 ];
      char buf[ BUF_SIZE ];

      close( fds[ 0 ] );

      close( 1 );
      dup( fds[ 1 ] );
      close( fds[ 1 ] );

      close( 0 );
/*      close( 2 );*/

      argc = 0;

      argv[ argc++ ] = "caleidlink";

      if( PASSIVE_SYNC == syncMode )
	argv[ argc++ ] = TokenAlloc( "-p" );
      else
	argv[ argc++ ] = TokenAlloc( "-a" );

      sprintf( buf, "-d%s", deviceName );
      argv[ argc++ ] = TokenAlloc( buf );

      sprintf( buf, "-s%ld", speed );
      argv[ argc++ ] = TokenAlloc( buf );

      argv[ argc++ ] = "-c" CODING_SYSTEM;

      argv[ argc++ ] = "-f";

      switch( mode ){
      case SYNCHRONIZE:
	argv[ argc++ ] = "-S";
	argv[ argc++ ] = folderName;
	break;
      case BACKUP:
	argv[ argc++ ] = "-B";
	argv[ argc++ ] = backupName;
	break;
      case RESTORE:
	argv[ argc++ ] = "-R";
	argv[ argc++ ] = backupName;
	break;
      }

      argv[ argc ] = NULL;

      if( 0 > execvp( "caleidlink", argv ) ){
	fprintf( stderr, "xmclink: cannot exec\n" );
	exit( 1 );
      }
    }
    break;
  default:
    /* parent */

    close( fds[ 1 ] );

    child = pid;
    childFd = fds[ 0 ];
  }

  return TRUE;
}

/**********************************************************************/

private void AnimSet( int index )
{
  Dimension width, height;

  XtVaGetValues( animWidget,
		XmNwidth,	&width,
		XmNheight,	&height,
		NULL );

  XCopyArea( XtDisplay( animWidget ),
	    pixClink[ index ],
	    XtWindow( animWidget ),
	    animGC,
	    0, 0,
	    64, 48,
	    ( width - 64 ) / 2,
	    ( height - 48 ) / 2 );
}

private XtIntervalId rollingCaleidId;
private int rollingCaleidAlive = 0;

private void AnimStop()
{
  if( rollingCaleidAlive ){
    XtRemoveTimeOut( rollingCaleidId );
    rollingCaleidAlive = 0;
  }
  pixIndex = 0;
  AnimSet( pixIndex );
}

private void RollingCaleid( XtPointer clientData, XtIntervalId *id )
{
  if( PIX_COUNT <= ++pixIndex )
    pixIndex = 1;
  AnimSet( pixIndex );

  rollingCaleidId = XtAppAddTimeOut( XtWidgetToApplicationContext( shell ),
				     ROLLING_INTERVAL,
				     RollingCaleid,
				     NULL );
  rollingCaleidAlive = 1;
}

private void AnimStart()
{
  pixIndex = 0;
  RollingCaleid( NULL, NULL );
}

private void ReadFromClink( XtPointer clientData, int *fid, XtInputId *id )
{
  private int ptr = 0;
  private char line[ BUF_SIZE ];
  int i, len;
  char buf[ BUF_SIZE ];

  if( 0 == child || 0 > (len = read( *fid, buf, BUF_SIZE )) ){
    fprintf( stderr, "xmclink: cannot read data from caleidlink.\n" );
    exit( 1 );
  }

  for( i = 0 ; i < len ; i++ ){
    if( 0x0a == buf[ i ] || 0x0d == buf[ i ] ){
      line[ ptr ] = '\0';
      ptr = 0;
      if( '\0' != *line )
	XmTextFieldSetString( msgWidget, line );
      if( PASSIVE_SYNC == syncMode ){
	if( !strncmp( line, "Ready", 5 ) ){
	  if( rollingCaleidAlive ){
	    XtRemoveTimeOut( rollingCaleidId );
	    rollingCaleidAlive = 0;
	  }
	  pixIndex = 1;
	  AnimSet( pixIndex );
	} else if( 0 == rollingCaleidAlive ){
	  AnimStart();
	}
      }
    } else {
      line[ ptr++ ] = buf[ i ];
      if( BUF_SIZE - 2 == ptr )
	ptr--;
    }
  }
}

/**********************************************************************/

private void Refresh()
{
  int i;
  Arg args[ 1 ];

  mode = newMode;
  if( NULL != modeString )
    XmStringFree( modeString );
  modeString = XmStringCreateLocalized( modeLabel[ mode ] );

  XtVaSetValues( modeLabelWidget,
		 XmNlabelString,	modeString,
		 NULL );

  switch( mode ){
  case SYNCHRONIZE:
    XmTextFieldSetString( folderLabelWidget, folderName );
    break;
  case BACKUP:
  case RESTORE:
    XmTextFieldSetString( folderLabelWidget, backupName );
    break;
  }

  for( i = 0 ; i < MODES ; i++ ){
    if( i == mode ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( modeWidget[ i ], args, 1 );
    } else {
      XtSetArg( args[ 0 ], XmNset, False );
      XtSetValues( modeWidget[ i ], args, 1 );
    }
  }

  for( i = 0 ; i < SPEEDS ; i++ ){
    if( speedName[ i ].speed == speed ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( speedWidget[ i ], args, 1 );
    } else {
      XtSetArg( args[ 0 ], XmNset, False );
      XtSetValues( speedWidget[ i ], args, 1 );
    }
  }

  for( i = 0 ; i < 2 ; i++ ){
    if( i == syncMode ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( syncModeWidget[ i ], args, 1 );
    } else {
      XtSetArg( args[ 0 ], XmNset, False );
      XtSetValues( syncModeWidget[ i ], args, 1 );
    }
    if( i == imageSize ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( imageSizeWidget[ i ], args, 1 );
    } else {
      XtSetArg( args[ 0 ], XmNset, False );
      XtSetValues( imageSizeWidget[ i ], args, 1 );
    }
    if( i == autoSave ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( autoSaveWidget[ i ], args, 1 );
    } else {
      XtSetArg( args[ 0 ], XmNset, False );
      XtSetValues( autoSaveWidget[ i ], args, 1 );
    }
  }

  if( PASSIVE_SYNC == syncMode ){
    if( 0 == child )
      ForkProcess();

    if( 0 != child && -1 != childFd ){
      inpId = XtAppAddInput( XtWidgetToApplicationContext( animWidget ),
			     childFd,
			     (XtPointer)XtInputReadMask,
			     ReadFromClink,
			     NULL );
      AnimStart();
    }
  }
}

/**********************************************************************/

private boolean_t KillChild()
{
  if( 0 != child ){
    XmTextFieldSetString( msgWidget, "Killing child..." );
    kill( child, SIGTERM );
    AnimStop();
  }

  return TRUE;
}

private void FolderReload()
{
  if( folder )
    FolderFree( folder );
  folder = FolderLoad( folderName, EUC_JAPAN );
  RefreshCategories();

  if( PASSIVE_SYNC == syncMode )
    KillChild();
}

private void SignalHandler( XtPointer clientData, XtSignalId *id )
{
  int status;

  status = *(int *)clientData;

  if( -1 != childFd ){
    XtRemoveInput( inpId );
    close( childFd );
    childFd = -1;
    FolderReload();
  }

  child = 0;

  if( PASSIVE_SYNC == syncMode ){
    if( ( 0 == status || 15 == status ) && FALSE == flagTerminate ){
      /*
      XmTextFieldSetString( msgWidget, "Restarting caleidlink now" );
      */
      Refresh(); /* fork child in current mode */
      newMode = SYNCHRONIZE;
    } else {
      /*
      XmTextFieldSetString( msgWidget, "Child was stopped" );
      */
      AnimStop();
    }
  } else {
    AnimStop();
    newMode = SYNCHRONIZE;
    Refresh(); /* does not fork child */
  }
}

private void ChildHandler( int arg )
{
  int status;

  if( child != wait( &status ) )
    return;

  if( WIFEXITED( status ) )
    childStatus = WEXITSTATUS( status );
  else
    childStatus = -1;

/*  printf( "status is %d\n", childStatus );*/

  XtNoticeSignal( sigId );
}

private void KillHandler( int arg )
{
  if( 0 != child )
    kill( child, SIGTERM );

  exit( arg );
}

private void DestroyCallback( Widget w, XtPointer clientData,
			      XtPointer callData )
{
  KillHandler( 1 );
}

/**********************************************************************/

private boolean_t LoadConfig( char *config )
{
  FILE *fp;
  char *home = NULL, *file;
  char *split[ SPLIT_COUNT ];
  char buf[ BUF_SIZE ];

  home = getenv( "HOME" );

  if( '/' == *config ){
    file = TokenAlloc( config );
  } else {
    if( NULL == home ){
      if( NULL == (file = (char *)malloc( strlen( config ) + 1 )) )
	perror( "xmclink" ), exit( -1 );
      strcpy( file, config );
    } else {
      if( NULL == (file = (char *)malloc( strlen( home ) + strlen( CONFIG_FILE ) + 2 )) )
	perror( "xmclink" ), exit( -1 );
      strcpy( file, home );
      strcat( file, "/" );
      strcat( file, CONFIG_FILE );
    }
  }

  child = 0;
  mode = newMode = SYNCHRONIZE;
  deviceName = TokenAlloc( DEFAULT_DEVICE );
  speed = DEFAULT_SPEED;
  backupName = TokenAlloc( DEFAULT_BACKUP );

  if( NULL != home ){
    strcpy( buf, home );
    strcat( buf, "/" );
    strcat( buf, DEFAULT_FOLDER );
  } else {
    strcpy( buf, DEFAULT_FOLDER );
  }
  folderName = TokenAlloc( buf );

  if( NULL != (fp = fopen( file, "r" )) ){
    while( NULL != fgets( buf, BUF_SIZE, fp ) ){
      if( '#' == *buf )
	continue;

      if( 2 > Split( buf, split, SPLIT_COUNT ) ){
	fprintf( stderr,
		"xmclink: invalid field (%s) in (%s)\n",
		buf, file );
	continue;
      }

      if( !strcmp( split[ 0 ], "folder:" ) ){
	free( folderName );
	folderName = TokenAlloc( split[ 1 ] );
      } else if( !strcmp( split[ 0 ], "device:" ) ){
	free( deviceName );
	deviceName = TokenAlloc( split[ 1 ] );
      } else if( !strcmp( split[ 0 ], "speed:" ) ){
	speed = atoi( split[ 1 ] );
	switch( speed ){
	case 4800:
	case 9600:
	case 19200:
	case 38400:
	  break;
	default:
	  fprintf( stderr,
		  "xmclink: invalid speed %ldbps. 38400bps was selected\n",
		  speed );
	  speed = 38400;
	}
      } else if( !strcmp( split[ 0 ], "syncmode:" ) ){
	if( !strcmp( split[ 1 ], "passivesync" ) )
	  syncMode = PASSIVE_SYNC;
	else
	  syncMode = 0;
      } else if( !strcmp( split[ 0 ], "image:" ) ){
	if( !strcmp( split[ 1 ], "large" ) )
	  imageSize = IMAGE_LARGE;
	else
	  imageSize = IMAGE_SMALL;
      } else if( !strcmp( split[ 0 ], "autosave:" ) ){
	if( !strcmp( split[ 1 ], "False" ) )
	  autoSave = MANUAL_SAVE;
	else
	  autoSave = 0;
      }
    }
  }

  free( file );

  folderString = XmStringCreateLocalized( folderName );

  return TRUE;
}

private boolean_t SaveConfig()
{
  FILE *fp;
  char *home, *file;

  if( NULL == (home = getenv( "HOME" )) ){
    if( NULL == (file = (char *)malloc( strlen( CONFIG_FILE ) + 1 )) )
      perror( "xmclink" ), exit( -1 );
    strcpy( file, CONFIG_FILE );
  } else {
    if( NULL == (file = (char *)malloc( strlen( home ) + strlen( CONFIG_FILE ) + 2 )) )
      perror( "xmclink" ), exit( -1 );
    strcpy( file, home );
    strcat( file, "/" );
    strcat( file, CONFIG_FILE );
  }

  if( Exist( file ) || NULL == (fp = fopen( file, "w" )) ){
    free( file );
    fprintf( stderr, "xmclink: cannot open (%s)\n", file );
    return FALSE;
  }

  fprintf( fp, "folder: %s\n", folderName );
  fprintf( fp, "device: %s\n", deviceName );
  fprintf( fp, "speed: %ld\n", speed );
  fprintf( fp, "syncmode: %s\n",
	   ( PASSIVE_SYNC == syncMode ) ? "passivesync" : "activesync" );
  fprintf( fp, "image: %s\n",
	   ( IMAGE_LARGE == imageSize ) ? "large" : "small" );
  fprintf( fp, "autosave: %s\n",
	   ( MANUAL_SAVE == autoSave ) ? "False" : "True" );

  fclose( fp );
  free( file );

  return TRUE;
}

/**********************************************************************/

private void FolderReloadCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  FolderReload();
}

private void ExitCallback( Widget w, XtPointer clientData, XtPointer callData )
{
  flagTerminate = TRUE;
  KillChild();

  SaveConfig();

  exit( 0 );
}

private void CreateFilePane( Widget parent )
{
  Widget cascade, submenu;
  Widget buttonReload, buttonExit;

  submenu = XmCreatePulldownMenu( parent, "fileSubmenu", NULL, 0 );
  cascade = XtVaCreateManagedWidget( "File", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  buttonReload = XtCreateManagedWidget( "Reload",
					xmPushButtonWidgetClass,
					submenu, NULL, 0 );
  XtAddCallback( buttonReload, XmNactivateCallback,
		 FolderReloadCallback, NULL );

  /*
  buttonCreate = XtCreateManagedWidget( "Create",
					xmPushButtonWidgetClass,
					submenu, NULL, 0 );
  XtAddCallback( buttonCreate, XmNactivateCallback, SaveCallback, NULL );
  */

  XtCreateManagedWidget( "separator", xmSeparatorWidgetClass,
			 submenu, NULL, 0 );

  buttonExit = XtCreateManagedWidget( "Exit",
				      xmPushButtonWidgetClass,
				      submenu, NULL, 0 );
  XtAddCallback( buttonExit, XmNactivateCallback, ExitCallback, NULL );
}

/**********************************************************************/

/*
 * エディット
 */

typedef struct WIDGET_LIST_T {
  Widget widget;
  struct WIDGET_LIST_T *next;
} widget_list_t;

typedef struct OBJECT_LIST_T {
  widget_list_t *content_list;
  struct OBJECT_LIST_T *next;
} object_list_t;

typedef struct {
  category_t *category;
  Widget button, popup, list;
  Widget label, heading;
  int dirty;
} category_widget_t;

#define CATEGORY_WIDGET_MAX		32

private Widget editSubmenu = NULL;
private category_widget_t categoryWidget[ CATEGORY_WIDGET_MAX ];

#define ALIAS_LEN	32

typedef struct TEXT_LIST_T {
  char id[ 8 ];
  char alias[ ALIAS_LEN ];
  Widget lbl, txt, scroll, draw;
  struct TEXT_LIST_T *next;
} text_list_t;

typedef struct {
  char *file_name;
  int plain;
  Widget popup, draw;
  Drawable drawable;
  GC gc;
  int width, height;
  text_list_t *textList;
  int changed;
} text_data_t;

private int objectEditIndex = 0;

private void TextDataFree( text_data_t *textData )
{
  text_list_t *textList, *tmp;

  //fprintf( stderr, "freeing: %s\n", textData->file_name );

  free( textData->file_name );
  if( textData->draw )
    XFreePixmap( XtDisplay( textData->draw ), textData->drawable );

  for( textList = textData->textList ; textList ; ){
    tmp = textList;
    textList = textList->next;
    free( tmp );
  }

  free( textData );
}

private void EditDestroyCallback( Widget w, XtPointer clientData,
				  XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;

  TextDataFree( textData );
}

private char preamble[] = {
  0x42, 0x4d, 0x5e, 0x08, 0x00, 0x00, 0x00, 0x00,	/*  0- 7 */
  0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x28, 0x00,	/*  8-15 */
  0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x68, 0x00,	/* 16-23 */
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,	/* 24-31 */
  0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00,	/* 32-39 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 40-47 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,	/* 48-55 */
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00			/* 56-61 */
};

private int ObjectSave( char *file, text_data_t *textData )
{
  text_list_t *textList;
  FILE *fp;
  int i, len;
  char *str;
  struct stat sb;
  char buf[ BUF_SIZE ];

  //fprintf( stderr, "save:[%s]\n", file );

  if( 0 == stat( file, &sb ) ){
    strcpy( buf, file );
    strcat( buf, ".org" );
    if( 0 > rename( file, buf ) ){
      fprintf( stderr, "xmclink: rename failed\n" );
      return -2;
    }
  }

  if( NULL == (fp = fopen( file, "w" )) ){
    fprintf( stderr, "xmclink: cannot open %s\n", file );
    return -1;
  }

  textList = textData->textList;
  for( ; textList ; textList = textList->next ){
    str = NULL;

    if( textList->txt ){
      if( XmIsText( textList->txt ) )
	str = XmTextGetString( textList->txt );
      else
	str = XmTextFieldGetString( textList->txt );

      if( NULL == str )
	continue;
      if( '\0' == *str ){
	XtFree( (XtPointer)str );
	continue;
      }

      if( 0 == textData->plain ){
	fputs( textList->id, fp );
	fputc( ' ', fp );
	fputs( Local2Str( textList->alias, EUC_JAPAN ), fp );
	fputc( ' ', fp );
      }
      len = strlen( str );
      for( i = 0 ; i < len ; i++ ){
	if( 0 == textData->plain && '\n' == str[ i ] ){
	  fputc( '\n', fp );
	  fputc( '\t', fp );
	} else {
	  fputc( str[ i ], fp );
	}
      }
      fputc( '\n', fp );

      XtFree( (XtPointer)str );
    } else if( textList->draw ){
      XImage *image;
      int x, y, width, height, width_word, high, ch, bit, ptr;
      int image_size, file_size;
      char *buf, *data;

      width = textData->width;
      height = textData->height;

      width_word = width / 32;
      if( 0 != width % 32 )
	width_word++;

      image_size = width_word * 4 * height;
      file_size = image_size + 62;

      buf = (char *)malloc( file_size );
      memcpy( buf, preamble, 62 );
      data = (char *)malloc( image_size );

      buf[ 2 ] = file_size & 0xff;	/* little endian */
      buf[ 3 ] = file_size >> 8;

      buf[ 18 ] = width & 0xff;
      buf[ 19 ] = width >> 8;

      buf[ 22 ] = height & 0xff;
      buf[ 23 ] = height >> 8;

      buf[ 34 ] = image_size & 0xff;
      buf[ 35 ] = image_size >> 8;

      image = XGetImage( XtDisplay( textData->draw ),
			 textData->drawable,
			 0, 0,
			 width,
			 height,
			 1,
			 XYPixmap );

      ptr = 0;
      for( y = 0 ; y < height ; y++ ){
	x = 0;
	high = width_word * 32;
	while( x < high ){
	  ch = 0;
	  for( bit = 7 ; bit >= 0 ; bit-- ){
	    if( x < width ){
	      if( 0 == XGetPixel( image, x, y ) )
		ch |= (1 << bit);
	    }
	    x++;
	  }
	  data[ ptr++ ] = ch;
	}
      }

      for( y = 0 ; y < height ; y++ ){
	memcpy( buf + 62 + width_word * 4 * y,
		data + width_word * 4 * ( height - y - 1 ),
		width_word * 4 );
      }

      ptr = 0;
      while( ptr < file_size ){
	high = file_size - ptr;
	if( high > 512 )
	  high = 512;
	EncodeBase64( buf + ptr, high, data );
	ptr += high;
	if( ptr >= file_size )
	  fprintf( fp, "102000 102000 %s\n", data );
	else
	  fprintf( fp, "102001 102001 %s\n", data );
      }

      free( data );
      free( buf );

      XDestroyImage( image );
    }
  }

  fclose( fp );

  return 0;
}

private void FailedToSaveOkCallback( Widget w, XtPointer clientData,
				     XtPointer callData )
{
  Widget parent = (Widget)clientData;

  if( parent )
    w = parent;

  XtDestroyWidget( w );
}

private void FailedToSave( Widget w, char *file, int flag_destroy )
{
  /* file save failed. return to editting */
  Widget dialog;
  XmString title, xms;
  Arg args[ 2 ];
  char name[ BUF_SIZE ], buf[ BUF_SIZE ];

  objectEditIndex++;

  title = XmStringCreateLocalized( "Failed!" );
  sprintf( buf, "Failed to save '%s'.", file );
  xms = XmStringCreateLocalized( buf );
  XtSetArg( args[ 0 ], XmNdialogTitle, title );
  XtSetArg( args[ 1 ], XmNmessageString, xms );
  sprintf( name, "cancel%d", objectEditIndex );
  dialog = XmCreateInformationDialog( w, name, args, 2 );
  XmStringFree( title );
  XmStringFree( xms );

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

  XtAddCallback( dialog, XmNokCallback,
		 FailedToSaveOkCallback,
		 ( flag_destroy ? (XtPointer)w : NULL ) );

  XtManageChild( dialog );
}

private void FileSaveOkCallback( Widget w, XtPointer clientData,
				 XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;
  Widget text;
  char *file;

  text = XtNameToWidget( w, "Text" );
  file = XmTextFieldGetString( text );

  if( 0 > ObjectSave( file, textData ) ){
    /* file save failed. return to editting */
    FailedToSave( w, file, TRUE );
    return;
  }

  FolderReload();

  XtFree( file );

  XtDestroyWidget( textData->popup );
}

private void FileSaveCancelCallback( Widget w, XtPointer clientData,
				     XtPointer callData )
{
  XtDestroyWidget( w );
}

private void EditOkCallback( Widget w, XtPointer clientData,
			     XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;
  Widget dialog;
  int len;
  XmString title;
  Arg args[ 2 ];

  if( 0 == textData->changed ){
    XtDestroyWidget( textData->popup );
    return;
  }

  len = strlen( textData->file_name );
  if( len > 0 && '/' == textData->file_name[ len - 1 ] ){
    XmString xms;
    int retry = 0, file = 0;

    if( MANUAL_SAVE != autoSave ){
      struct stat sb;
      char buf[ BUF_SIZE ];

      for( retry = 0 ; retry < 10 ; retry++ ){
	file = random() % 10000;
	strcpy( buf, textData->file_name );
	sprintf( buf + len, "auto%04d", file );

	if( 0 != stat( buf, &sb ) ){
	  free( textData->file_name );
	  textData->file_name = TokenAlloc( buf );
	  break;
	} else {
	  file = 0;
	}
      }
      /*
       * same seed?
       */
      {
	struct timeval tv;

	gettimeofday( &tv, NULL );
	srandom( tv.tv_usec );
      }
    }

    if( 0 == file ){
      title = XmStringCreateLocalized( "Save" );
      XtSetArg( args[ 0 ], XmNdialogTitle, title );
      dialog = XmCreateFileSelectionDialog( w, "Save", args, 1 );
      XmStringFree( title );

      XtUnmanageChild( XmFileSelectionBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

      xms = XmStringCreateLocalized( textData->file_name );
      XtVaSetValues( dialog, XmNdirectory, xms, NULL );
      XmStringFree( xms );

      XtAddCallback( dialog, XmNokCallback,
		     FileSaveOkCallback, (XtPointer)textData );
      XtAddCallback( dialog, XmNcancelCallback,
		     FileSaveCancelCallback, (XtPointer)textData );

      XtManageChild( dialog );
      return;
    }
  }

  if( 0 > ObjectSave( textData->file_name, textData ) ){
    FailedToSave( w, textData->file_name, FALSE );
    return;
  }
  FolderReload();

  XtDestroyWidget( w );
}

private void EditCancelOkCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;

  XtDestroyWidget( textData->popup );
  XtDestroyWidget( w );
}

private void EditCancelCancelCallback( Widget w, XtPointer clientData,
				       XtPointer callData )
{
  XtDestroyWidget( w );
}

private void EditCancelCallback( Widget w, XtPointer clientData,
				 XtPointer callData )
{
  XmString title, xms;
  text_data_t *textData = (text_data_t *)clientData;
  Widget dialog;
  Arg args[ 2 ];
  char name[ BUF_SIZE ], buf[ BUF_SIZE ];

  if( 0 == textData->changed ){
    XtDestroyWidget( textData->popup );
    return;
  }

  objectEditIndex++;

  title = XmStringCreateLocalized( "Cancel?" );
  sprintf( buf, "Do you really want to discard changes on file '%s' ?", textData->file_name );
  xms = XmStringCreateLocalized( buf );
  XtSetArg( args[ 0 ], XmNdialogTitle, title );
  XtSetArg( args[ 1 ], XmNmessageString, xms );
  sprintf( name, "cancel%d", objectEditIndex );
  dialog = XmCreateQuestionDialog( w, name, args, 2 );
  XmStringFree( title );
  XmStringFree( xms );

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

  XtAddCallback( dialog, XmNokCallback,
		 EditCancelOkCallback, (XtPointer)clientData );
  XtAddCallback( dialog, XmNcancelCallback,
		 EditCancelCancelCallback, (XtPointer)clientData );

  XtManageChild( dialog );
}

private void cr2lf( char *crs, char *lfs )
{
  if( NULL == crs )
    return;

  for( ; *crs ; crs++, lfs++ ){
    if( '\r' == *crs )
      *lfs = '\n';
    else
      *lfs = *crs;
  }

  *lfs = '\0';
}

private void DataChangedCallback( Widget w, XtPointer clientData,
				  XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;

  textData->changed = TRUE;
}

private void RedrawCallback( Widget w,
			     XtPointer clientData, XtPointer callData )
{
  text_data_t *textData = (text_data_t *)clientData;

  XCopyArea( XtDisplay( textData->draw ), textData->drawable,
	     XtWindow( textData->draw ), textData->gc,
	     0, 0, textData->width, textData->height,
	     6, 6 );
}

private int mouse_button = 0;
private int last_x = -1, last_y = -1;

private void MousePress( Widget w,
			 XtPointer clientData,
			 XEvent *event,
			 Boolean *flag )
{
  mouse_button = event->xbutton.button;
}

private void MouseRelease( Widget w,
			   XtPointer clientData,
			   XEvent *event,
			   Boolean *flag )
{
  last_x = last_y = -1;
}

private void MouseDrag( Widget w,
			XtPointer clientData,
			XEvent *event,
			Boolean *flag )
{
  text_data_t *textData = (text_data_t *)clientData;
  int x, y;
  Display *dpy;

  if( event->xmotion.x < 6 || event->xmotion.x >= (6 + textData->width)
      || event->xmotion.y < 6 || event->xmotion.y >= (6 + textData->height) ){
    last_x = last_y = -1;
    return;
  }

  dpy = XtDisplay( textData->draw );
  x = event->xmotion.x - 6;
  y = (textData->height - event->xmotion.y - 1) + 6;

  //fprintf( stderr, "x:%d y:%d (%d)\n", x, y, event->xbutton.button );

  if( Button1 != mouse_button ){
    XSetForeground( dpy, textData->gc,
		    WhitePixelOfScreen(XtScreen(textData->draw)) );
    XSetBackground( dpy, textData->gc,
		    BlackPixelOfScreen(XtScreen(textData->draw)) );
    if( Button3 == mouse_button ){
      XDrawPoint( dpy, textData->drawable, textData->gc,
		  x - 1, textData->height - y - 1 );
      XDrawPoint( dpy, textData->drawable, textData->gc,
		  x + 1, textData->height - y - 1 );
      XDrawPoint( dpy, textData->drawable, textData->gc,
		  x, textData->height - y - 1 - 1 );
      XDrawPoint( dpy, textData->drawable, textData->gc,
		  x, textData->height - y - 1 + 1 );

      XDrawPoint( dpy, textData->drawable, textData->gc,
		  x, textData->height - y - 1 );
    } else {
      XFillArc( dpy, textData->drawable, textData->gc,
		x - CIRCLE_WIDTH / 2,
		textData->height - y - 1 - CIRCLE_WIDTH / 2,
		CIRCLE_WIDTH,
		CIRCLE_WIDTH,
		0, 64*360 );
    }
  } else {
    if( -1 == last_x ){
      last_x = x;
      last_y = textData->height - y - 1;
    }

    XDrawLine( dpy, textData->drawable, textData->gc,
	       last_x, last_y,
	       x, textData->height - y - 1 );

    last_x = x;
    last_y = textData->height - y - 1;
  }
  if( Button1 != mouse_button ){
    XSetForeground( dpy, textData->gc,
		    BlackPixelOfScreen(XtScreen(textData->draw)) );
    XSetBackground( dpy, textData->gc,
		    WhitePixelOfScreen(XtScreen(textData->draw)) );
  }

  textData->changed = TRUE;

  XCopyArea( XtDisplay( textData->draw ), textData->drawable,
	     XtWindow( textData->draw ), textData->gc,
	     0, 0, textData->width, textData->height,
	     6, 6 );
}

private void DrawSetup( text_data_t *textData,
			Widget draw, object_t *object )
{
  content_t *content;
  char *data = NULL, *buf = NULL;
  int size = 0, len;
  int width, height, width_word, x, y, high;
  int ptr, byte, bit;
  Display *dpy;
  Pixmap drawable;
  GC gc;
  XGCValues values;

  textData->draw = draw;
  textData->drawable = '\0';

  if( NULL == object ){
    if( IMAGE_LARGE == imageSize ){
      width = 292;
      height = 172;
    } else {
      width = 147;
      height = 104;
    }
    buf = data = NULL;
  } else {
    for( content = object->contents ; content ; content = content->next ){
      if( EqualID( content->id, "102001" )
	  || EqualID( content->id, "102000" ) ){
	data = (char *)realloc( data, size + content->length );
	memcpy( data + size, content->data, content->length );
	size += content->length;
      }
    }

    buf = (char *)malloc( size );
    len = Shifted2Bin( data, size, buf );
    free( data );
    data = buf;

    if( NULL == data || len < 62 )
      return;

    //fprintf( stderr, "len:%d\n", len );

    width = (data[ 19 ] << 8) | data[ 18 ]; /* little endian */
    height = (data[ 23 ] << 8) | data[ 22 ];

    if( 292 == width )
      imageSize = IMAGE_LARGE;
    else
      imageSize = IMAGE_SMALL;

    data = data + 62;
  }

  textData->width = width;
  textData->height = height;

  //fprintf( stderr, "width:%d height:%d\n", width, height );

  width_word = width / 32;
  if( 0 != width % 32 )
    width_word++;

  XtVaSetValues( draw,
		 XmNtraversalOn, False,
		 XmNshadowType, XmSHADOW_ETCHED_OUT,
		 XmNshadowThickness, 4,
		 NULL );

  values.foreground = WhitePixelOfScreen(XtScreen(draw));
  values.background = BlackPixelOfScreen(XtScreen(draw));

  dpy = XtDisplay( draw );
  drawable = XCreatePixmap( dpy, DefaultRootWindow( dpy ),
			    width, height,
			    DefaultDepthOfScreen( XtScreen( draw ) ) );
  textData->drawable = drawable;
  gc = XtGetGC( draw, GCForeground | GCBackground, &values );
  textData->gc = gc;

  XFillRectangle( dpy, drawable, gc, 0, 0, width, height );

  XSetForeground( dpy, gc, BlackPixelOfScreen(XtScreen(draw)) );
  XSetBackground( dpy, gc, WhitePixelOfScreen(XtScreen(draw)) );

  if( buf ){
    ptr = 0;
    for( y = 0 ; y < height ; y++ ){
      x = 0;
      high = width_word * 32;
      while( x < high ){
	byte = data[ ptr++ ];
	for( bit = 7 ; bit >= 0 ; bit-- ){
	  if( x < width ){
	    if( (byte & (1 << bit)) ){
	      //fprintf( stderr, "x:%d, y:%d\n", x, y );
	      XDrawPoint( dpy, drawable, gc, x, height - y - 1 );
	    }
	  }
	  x++;
	}
      }
    }

    free( buf );
  }

  //fprintf( stderr, "width:%d, height:%d\n", width, height );

  XtVaSetValues( draw,
		 XmNwidth, width + 12,
		 XmNheight, height + 12,
		 NULL );

  XtAddCallback( draw, XmNexposeCallback,
		 RedrawCallback, (XtPointer)textData );
  XtAddEventHandler( draw, ButtonPressMask, FALSE,
		     MousePress, (XtPointer)textData );
  XtAddEventHandler( draw, ButtonMotionMask, FALSE,
		     MouseDrag, (XtPointer)textData );
  XtAddEventHandler( draw, ButtonReleaseMask, FALSE,
		     MouseRelease, (XtPointer)textData );
}

private void ObjectEdit( Widget w, category_t *category, object_t *object )
{
  XmString xms;
  Widget popup, work, form, label, txt, window, scroll;
  content_t *content;
  field_t *heading;
  text_data_t *textData;
  text_list_t *textList, *textListLast = NULL;
  int length, rows, count = 0;
  int width, height, text_height, diff, total_height, max_width;
  struct stat sb;
  Arg args[ 4 ];
  char name[ BUF_SIZE ], buf[ CODE_SIZE ];

  if( NULL == (textData = (text_data_t *)malloc( sizeof( text_data_t ) )) )
    return;
  textData->textList = NULL;
  textData->changed = FALSE;

  if( EqualID( category->id, CAT_NOTE_MEMO )		/* memo */
      || EqualID( category->id, CAT_NOTE_DATA ) )	/* data */
    textData->plain = 2;
  else if( EqualID( category->id, CAT_SHEET ) )		/* sheet */
    textData->plain = 1;
  else
    textData->plain = 0;

  length = strlen( folder->top ) + 1 /* '/' */
    + strlen( category->directory ) + 1 /* '/' */
    + 1 /* NULL termination */;

  if( object )
    length += strlen( object->file );

  if( NULL == (textData->file_name = (char *)malloc( length )) )
    perror( "xmclink" ), exit( -1 );

  strcpy( textData->file_name, folder->top );
  strcat( textData->file_name, "/" );
  strcat( textData->file_name, category->directory );
  strcat( textData->file_name, "/" );

  if( object )
    strcat( textData->file_name, object->file );
  else {
    if( 0 > stat( textData->file_name, &sb ) )
      mkdir( textData->file_name, 0700 );
  }

  objectEditIndex++;

  sprintf( name, "edit%d", objectEditIndex );
  XtSetArg( args[ 0 ], XmNinitialResourcesPersistent, False );
  XtSetArg( args[ 1 ], XmNautoUnmanage, False );
  popup = XmCreateMessageDialog( shell, name, args, 2 );
  textData->popup = popup;
  textData->draw = NULL;

  xms = XmStringCreateLocalized( textData->file_name );
  XtVaSetValues( popup,
		 XmNdialogTitle, xms,
		 NULL );
  XmStringFree( xms );

  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_MESSAGE_LABEL ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_SYMBOL_LABEL ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_HELP_BUTTON ) );

  form = XtCreateManagedWidget( "form", xmFormWidgetClass,
				popup,
				NULL, 0 );

  label = XtVaCreateManagedWidget( "title", xmLabelWidgetClass,
				   form,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_FORM,
				   NULL );

  xms = XmStringCreateLocalized( Local2Str( category->name, EUC_JAPAN ) );
  if( xms ){
    XtVaSetValues( label,
		   XmNlabelString, xms,
		   NULL );
    XmStringFree( xms );
  }

  XtSetArg( args[ 0 ], XmNscrollingPolicy, XmAUTOMATIC );
  window = XmCreateScrolledWindow( form, "scrolled", args, 1 );

  XtVaSetValues( window,
		 XmNtopAttachment, XmATTACH_WIDGET,
		 XmNtopWidget, label,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_FORM,
		 NULL );

  work = XtVaCreateManagedWidget( "work", xmBulletinBoardWidgetClass,
				  window,
				  //XmNresizePolicy,	XmRESIZE_NONE,
				  XmNmarginWidth, 	0,
				  XmNmarginHeight,	0,
				  NULL );

  for( heading = category->heading ; heading ; heading = heading->next ){
    if( EqualID( heading->id, "000001" )
	|| EqualID( heading->id, "102001" ) )
      continue;
    if( object ){
      for( content = object->contents ; content ; content = content->next ){
	if( EqualID( heading->id, content->id ) )
	  break;
      }
    } else {
      content = NULL;
    }
    count++;

    if( NULL== (textList = (text_list_t *)malloc( sizeof( text_list_t ) )) )
      perror( "xmclink" ), exit( -1 );
    textList->next = NULL;
    textList->txt = NULL;
    textList->scroll = NULL;
    textList->draw = NULL;

    if( NULL == textListLast )
      textData->textList = textList;
    else
      textListLast->next = textList;
    textListLast = textList;

    if( 1 >= textData->plain ){
      sprintf( name, "label%d_%d", objectEditIndex, count );
      label = XtCreateManagedWidget( name, xmLabelWidgetClass,
				     work,
				     NULL, 0 );
      textList->lbl = label;

      strncpy( textList->id, heading->id, 6 );
      textList->id[ 6 ] = '\0';
      strncpy( textList->alias, heading->alias, ALIAS_LEN - 1 );
      textList->alias[ ALIAS_LEN - 1 ] = '\0';

      if( EqualID( heading->id, "102000" ) ){
	xms = XmStringCreateLocalized( "Image" );
      } else {
	//fprintf( stderr, "[%s]\n", Local2Str( heading->alias, EUC_JAPAN ) );

	xms = XmStringCreateLocalized( Local2Str( heading->alias, EUC_JAPAN ) );
      }

      XtVaSetValues( label,
		     XmNlabelString, xms,
		     NULL );
      XmStringFree( xms );
    }

    if( EqualID( heading->id, "102000" )
	|| EqualID( heading->id, "102001" ) ){
      if( NULL == textData->draw ){
	sprintf( name, "draw%d_%d", objectEditIndex, count );
	textList->draw = XtCreateManagedWidget( name,
						xmDrawnButtonWidgetClass,
						work,
						NULL, 0 );
	DrawSetup( textData, textList->draw, object );
      }
      continue;
    } else if( EqualID( heading->id, DAT_BEGIN_DATE )
	       || EqualID( heading->id, DAT_END_DATE )
	       || EqualID( heading->id, DAT_EXEC_DATE )
	       || EqualID( heading->id, DAT_ALARM_DATE ) ){

      sprintf( name, "text%d_%d", objectEditIndex, count );
      txt = XtVaCreateManagedWidget( name, xmTextFieldWidgetClass,
				     work,
				     XmNcolumns,	6,
				     XmNmaxLength,	10,
				     XmNbackground, WhitePixelOfScreen( XtScreen( work ) ),
				     NULL );
      if( content && content->data ){
	DateToStr( content->data, buf );
	XmTextFieldSetString( txt, Local2Str( buf, EUC_JAPAN ) );
      }
    } else if( EqualID( heading->id, DAT_BEGIN_TIME )
	       || EqualID( heading->id, DAT_END_TIME )
	       || EqualID( heading->id, DAT_ALARM_TIME ) ){

      sprintf( name, "text%d_%d", objectEditIndex, count );
      txt = XtVaCreateManagedWidget( name, xmTextFieldWidgetClass,
				     work,
				     XmNcolumns,	3,
				     XmNmaxLength,	5,
				     XmNbackground, WhitePixelOfScreen( XtScreen( work ) ),
				     NULL );
      if( content && content->data ){
	TimeToStr( content->data, buf );
	XmTextFieldSetString( txt, Local2Str( buf, EUC_JAPAN ) );
      }
    } else {
      if( EqualID( heading->id, "000000" ) )
	rows = 10;
      else
	rows = 2;

      sprintf( name, "scroll%d_%d", objectEditIndex, count );
      scroll = XmCreateScrolledWindow( work, name, NULL, 0 );
      textList->scroll = scroll;

      sprintf( name, "text%d_%d", objectEditIndex, count );
      txt = XtVaCreateManagedWidget( name, xmTextWidgetClass,
				     scroll,
				     XmNeditMode,	XmMULTI_LINE_EDIT,
				     XmNcolumns,	20,
				     XmNrows,		rows,
				     XmNmaxLength,	2048,
				     XmNwordWrap,	True,
				     XmNscrollHorizontal, False,
				     XmNbackground, WhitePixelOfScreen( XtScreen( work ) ),
				     NULL );
      XtVaSetValues( scroll,
		     XmNworkWindow, txt,
		     NULL );

      if( content && content->data ){
	cr2lf( content->data, buf );
	XmTextSetString( txt, Local2Str( buf, EUC_JAPAN ) );
      }

      XtManageChild( textList->scroll );
    }
    textList->txt = txt;
    XtAddCallback( txt, XmNvalueChangedCallback,
		   DataChangedCallback, (XtPointer)textData );
  }

  XtAddCallback( popup, XmNokCallback,
		 EditOkCallback, (XtPointer)textData );
  XtAddCallback( popup, XmNcancelCallback,
		 EditCancelCallback, (XtPointer)textData );
  XtAddCallback( popup, XmNdestroyCallback,
		 EditDestroyCallback, (XtPointer)textData );

  XtVaSetValues( window,
		 XmNworkWindow, work,
		 NULL );

  XtManageChild( window );
  XtManageChild( popup );

  if( 1 >= textData->plain ){
    max_width = 0;
    for( textList = textData->textList ; textList ; textList = textList->next ){
      width = 0;
      XtVaGetValues( textList->lbl,
		     XmNwidth, &width,
		     NULL );
      if( width > max_width )
	max_width = width;
    }

    total_height = 0;
    for( textList = textData->textList ; textList ; textList = textList->next ){
      width = 0;
      XtVaGetValues( textList->lbl,
		     XmNwidth, &width,
		     NULL );
      height = 0;
      XtVaGetValues( textList->lbl,
		     XmNheight, &height,
		     NULL );
      text_height = 0;
      if( textList->draw ){
	XtVaGetValues( textList->draw,
		       XmNheight, &text_height,
		       NULL );
      } else if( textList->scroll ){
	XtVaGetValues( textList->scroll,
		       XmNheight, &text_height,
		       NULL );
      } else {
	XtVaGetValues( textList->txt,
		       XmNheight, &text_height,
		       NULL );
      }
      diff = ( text_height - height ) >> 1;
      width = max_width - width;
      XtVaSetValues( textList->lbl,
		     XmNx, width,
		     XmNy, total_height + diff,
		     NULL );
      if( textList->draw ){
	XtVaSetValues( textList->draw,
		       XmNx, max_width,
		       XmNy, total_height,
		       NULL );
      } else if( textList->scroll ){
	XtVaSetValues( textList->scroll,
		       XmNx, max_width,
		       XmNy, total_height,
		       NULL );
      } else {
	XtVaSetValues( textList->txt,
		       XmNx, max_width,
		       XmNy, total_height,
		       NULL );
      }
      total_height += text_height;
    }
  }

  width = height = count = diff = 0;
  XtVaGetValues( work,
		 XmNwidth, &width,
		 XmNheight, &height,
		 NULL );
  XtVaGetValues( XtNameToWidget( window, "VertScrollBar" ),
		 XmNwidth, &count,
		 XmNshadowThickness, &diff,
		 NULL );

  //fprintf( stderr, "width:%d, height:%d, count:%d, diff:%d\n", width, height, count, diff );

  if( height > EDIT_HEIGHT ){
    XtVaSetValues( window,
		   XmNwidth, width + diff * 6 + count,
		   XmNheight, EDIT_HEIGHT,
		   NULL );
  } else {
    XtVaSetValues( window,
		   XmNwidth, width + diff * 2,
		   XmNheight, height + diff * 2,
		   NULL );
  }
}

private void ObjectCreateCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  int index;
  category_t *category;

  index = (int)clientData;
  category = categoryWidget[ index ].category;

  ObjectEdit( categoryWidget[ index ].popup, category, NULL );
}

private object_t *ObjectSelect( int index )
{
  int *posp = 0, pos;
  int count;
  category_t *category;
  object_t *object;

  category = categoryWidget[ index ].category;

  XtVaGetValues( categoryWidget[ index ].list,
		 XmNselectedPositions, &posp,
		 NULL );
  if( 0 == posp )
    return NULL;

  pos = *posp;

  count = 0;
  for( object = category->catalog ; object ; object = object->next ){
    if( object->die )
      continue;
    count++;
  }
  count -= pos;

  for( object = category->catalog ; object ; object = object->next ){
    if( object->die )
      continue;
    if( count <= 0 )
      break;
    count--;
  }

  return object;
}

private void ObjectEditCallback( Widget w, XtPointer clientData,
				 XtPointer callData )
{
  int index;
  category_t *category;
  object_t *object;

  index = (int)clientData;
  category = categoryWidget[ index ].category;

  object = ObjectSelect( index );

  if( NULL == object )
    return;

  ObjectEdit( categoryWidget[ index ].popup, category, object );
}

private void ObjectDeleteOkCallback( Widget w, XtPointer clientData,
				     XtPointer callData )
{
  char *file = (char *)clientData;
  char buf[ BUF_SIZE ];
  struct stat sb;

  if( 0 == stat( file, &sb ) ){
    strcpy( buf, file );
    strcat( buf, ".org" );
    rename( file, buf );
  }

  free( file );

  FolderReload();

  XtDestroyWidget( w );
}

private void ObjectDeleteCancelCallback( Widget w, XtPointer clientData,
					 XtPointer callData )
{
  free( clientData );
  XtDestroyWidget( w );
}

private void ObjectDeleteCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  int index;
  category_t *category;
  object_t *object;
  Widget dialog;
  XmString title, xms;
  int length;
  Arg args[ 2 ];
  char *file, name[ BUF_SIZE ], buf[ BUF_SIZE ];

  index = (int)clientData;
  category = categoryWidget[ index ].category;

  object = ObjectSelect( index );

  if( NULL == object )
    return;

  objectEditIndex++;

  //fprintf( stderr, "delete: %s\n", object->file );

  length = strlen( folder->top ) + 1 /* '/' */
    + strlen( category->directory ) + 1 /* '/' */
    + strlen( object->file )
    + 1 /* NULL termination */;

  file = (char *)malloc( length );
  strcpy( file, folder->top );
  strcat( file, "/" );
  strcat( file, category->directory );
  strcat( file, "/" );
  strcat( file, object->file );

  title = XmStringCreateLocalized( "Delete?" );
  sprintf( buf, "Do you really want to delete file '%s' ?", file );
  xms = XmStringCreateLocalized( buf );
  XtSetArg( args[ 0 ], XmNdialogTitle, title );
  XtSetArg( args[ 1 ], XmNmessageString, xms );
  sprintf( name, "delete%d", objectEditIndex );
  dialog = XmCreateQuestionDialog( w, name, args, 2 );
  XmStringFree( title );
  XmStringFree( xms );

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

  XtAddCallback( dialog, XmNokCallback,
		 ObjectDeleteOkCallback, (XtPointer)file );
  XtAddCallback( dialog, XmNcancelCallback,
		 ObjectDeleteCancelCallback, (XtPointer)file );

  XtManageChild( dialog );
}

private void CatalogCloseCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  int i;

  i = (int)clientData;

  XtUnmanageChild( categoryWidget[ i ].popup );
}

private void SetLabel( category_t *category, Widget label )
{
  XmString xms;
  char *category_name;

  category_name = Local2Str( category->name, EUC_JAPAN );

  xms = XmStringCreateLocalized( category_name );
  XtVaSetValues( label,
		 XmNlabelString, xms,
		 NULL );
  XmStringFree( xms );
}

private void SetHeadings( category_t *category , Widget txt )
{
  field_t *heading;
  int len, ptr;
  int flag_first;
  char buf[ BUF_SIZE ];

  if( EqualID( category->id, CAT_NOTE_MEMO )		/* memo */
      || EqualID( category->id, CAT_NOTE_DATA ) ){	/* data */
    XmTextFieldSetString( txt, "Plain text" );
    return;
  }

  len = 0;
  flag_first = 1;
  buf[ 0 ] = '\0';
  for( heading = category->heading ; heading ; heading = heading->next ){
    if( EqualID( heading->id, "000001" )
	|| EqualID( heading->id, "102000" )
	|| EqualID( heading->id, "102001" )
	)
      continue;
    if( flag_first ){
      flag_first = 0;
    } else {
      buf[ len++ ] = ',';
      buf[ len ] = '\0';
    }
    for( ptr = 0 ; ptr < 40 && heading->alias[ ptr ] ; ptr++ ){
      if( '\r' == heading->alias[ ptr ] || '\n' == heading->alias[ ptr ] )
	break;
      buf[ len ] = heading->alias[ ptr ];
      if( ':' != buf[ len ] )
	len++;
    }
    buf[ len ] = '\0';
  }

  XmTextFieldSetString( txt, Local2Str( buf, EUC_JAPAN ) );
}

private void SetList( category_t *category , Widget list )
{
  XmString xms;
  field_t *heading;
  object_t *object;
  content_t *content;
  int len, ptr;
  int flag_first;
  int rows = 0;
  char buf[ BUF_SIZE ];

  XtUnmanageChild( list );

  XtVaGetValues( list,
		 XmNitemCount,  &rows,
		 NULL );
  XmListDeleteItemsPos( list, rows, 1 );

  buf[ 0 ] = '\0';
  rows = 0;
  for( object = category->catalog ; object ; object = object->next ){
    if( object->die )
      continue;
    rows++;
    len = 0;
    flag_first = 0;
    strcpy( buf, object->file );
    len = strlen( buf );

    if( NULL == object->contents )
      FolderLoadObject( folder, category, object );

    for( heading = category->heading ; heading ; heading = heading->next ){
      if( EqualID( heading->id, "000001" )
	  || EqualID( heading->id, "102000" )
	  || EqualID( heading->id, "102001" )
	  )
	continue;
      for( content = object->contents ; content ; content = content->next ){
	if( EqualID( heading->id, content->id ) )
	  break;
      }
      if( flag_first ){
	flag_first = 0;
      } else {
	buf[ len++ ] = ',';
	buf[ len ] = '\0';
      }
      if( NULL == content )
	continue;

      if( EqualID( content->id, DAT_BEGIN_DATE )
	  || EqualID( content->id, DAT_END_DATE )
	  || EqualID( content->id, DAT_EXEC_DATE )
	  || EqualID( content->id, DAT_ALARM_DATE ) ){
	DateToStr( content->data, buf + len );
	len += 10;
      } else if( EqualID( content->id, DAT_BEGIN_TIME )
		 || EqualID( content->id, DAT_END_TIME )
		 || EqualID( content->id, DAT_ALARM_TIME ) ){
	TimeToStr( content->data, buf + len );
	len += 5;
      } else {
	for( ptr = 0 ; ptr < 40 && content->data[ ptr ] ; ptr++ ){
	  if( '\r' == content->data[ ptr ] || '\n' == content->data[ ptr ] )
	    break;
	  buf[ len++ ] = content->data[ ptr ];
	}
	buf[ len ] = '\0';
      }
    }

    xms = XmStringCreateLocalized( Local2Str( buf, EUC_JAPAN ) );
    XmListAddItemUnselected( list, xms, 1 );
    XmStringFree( xms );
  }

#if 0
  XtVaSetValues( list,
		 XmNvisibleItemCount,	rows,
		 NULL );
#endif

  XmListSelectPos( list, 1, FALSE );

  XtManageChild( list );
}

private void CatalogOpenCallback( Widget w, XtPointer clientData,
				  XtPointer callData )
{
  category_t *category;
  int i;
  Arg args[ 4 ];
  Widget popup, mainWindow, work, menu, submenu, cascade, list;
  Widget label, heading;
  Widget reload, create, edit, delete, close;
  char name[ BUF_SIZE ];
  XmString xms;

  i = (int)clientData;

  category = categoryWidget[ i ].category;

  if( categoryWidget[ i ].popup ){
    if( !XtIsManaged( categoryWidget[ i ].popup ) ){
      if( categoryWidget[ i ].dirty ){
	categoryWidget[ i ].dirty = 0;
	SetLabel( category, categoryWidget[ i ].label );
	SetHeadings( category, categoryWidget[ i ].heading );
	SetList( category, categoryWidget[ i ].list );
      }
      XtManageChild( categoryWidget[ i ].popup );
    }
    return;
  }

  XtSetArg( args[ 0 ], XmNmarginWidth, 0 );
  XtSetArg( args[ 1 ], XmNmarginHeight, 0 );
  sprintf( name, "category%d", i );
  popup = XmCreateMessageDialog( w, name, args, 2 );
  categoryWidget[ i ].popup = popup;

  sprintf( name, "%s/%s", folder->top, category->directory );
  xms = XmStringCreateLocalized( name );
  XtVaSetValues( popup,
		 XmNdialogTitle, xms,
		 XmNdefaultButtonType, XmDIALOG_NONE,
		 NULL );
  XmStringFree( xms );

  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_CANCEL_BUTTON ) );
  //XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_DEFAULT_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_HELP_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_MESSAGE_LABEL ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_OK_BUTTON ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_SEPARATOR ) );
  XtUnmanageChild( XmMessageBoxGetChild( popup, XmDIALOG_SYMBOL_LABEL ) );

  XtSetArg( args[ 0 ], XmNheight, CATALOG_HEIGHT );
  mainWindow = XtCreateManagedWidget( "mainWindow", xmMainWindowWidgetClass,
				      popup,
				      args, 1 );

  menu = XmCreateMenuBar( mainWindow, "menu", NULL, 0 );

  submenu = XmCreatePulldownMenu( menu, "submenu", NULL, 0 );
  cascade = XtVaCreateManagedWidget( "File", xmCascadeButtonWidgetClass,
				     menu,
				     XmNsubMenuId, submenu,
				     NULL );

  reload = XtCreateManagedWidget( "Reload", xmPushButtonWidgetClass,
				  submenu, NULL, 0 );
  XtAddCallback( reload, XmNactivateCallback,
		 FolderReloadCallback, NULL );

  XtCreateManagedWidget( "separator", xmSeparatorWidgetClass,
			 submenu, NULL, 0 );

  create = XtCreateManagedWidget( "Create", xmPushButtonWidgetClass,
				  submenu, NULL, 0 );
  XtAddCallback( create, XmNactivateCallback,
		 ObjectCreateCallback, (XtPointer)i );

  edit = XtCreateManagedWidget( "Edit", xmPushButtonWidgetClass,
				submenu, NULL, 0 );
  XtAddCallback( edit, XmNactivateCallback,
		 ObjectEditCallback, (XtPointer)i );

  delete = XtCreateManagedWidget( "Delete", xmPushButtonWidgetClass,
				  submenu, NULL, 0 );
  XtAddCallback( delete, XmNactivateCallback,
		 ObjectDeleteCallback, (XtPointer)i );

  XtCreateManagedWidget( "separator", xmSeparatorWidgetClass,
			 submenu, NULL, 0 );

  close = XtCreateManagedWidget( "Close", xmPushButtonWidgetClass,
				 submenu, NULL, 0 );
  XtAddCallback( close, XmNactivateCallback,
		 CatalogCloseCallback, (XtPointer)i );

  XtManageChild( menu );

  work = XtCreateManagedWidget( "work", xmFormWidgetClass,
				mainWindow,
				NULL, 0 );

  label = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
				   work,
				   XmNtopAttachment, XmATTACH_FORM,
				   XmNleftAttachment, XmATTACH_FORM,
				   NULL );
  categoryWidget[ i ].label = label;

  SetLabel( category, label );

  heading = XtVaCreateManagedWidget( "heading", xmTextFieldWidgetClass,
				     work,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, label,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     NULL );
  categoryWidget[ i ].heading = heading;
  XtVaSetValues( heading,
		 XmNtraversalOn, False,
		 XmNeditable, False,
		 XmNcursorPositionVisible, False,
		 XmNverifyBell, False,
		 NULL );

  SetHeadings( category, heading );

  XtSetArg( args[ 0 ], XmNlistSizePolicy, XmCONSTANT/*XmRESIZE_IF_POSSIBLE*/ );
  list = XmCreateScrolledList( work, "list", args, 1 );
  categoryWidget[ i ].list = list;

  XtVaSetValues( list,
		 XmNwidth, CATALOG_WIDTH,
		 NULL );

  XtVaSetValues( XtParent( list ),
		 XmNtopAttachment, XmATTACH_WIDGET,
		 XmNtopWidget, heading,
		 XmNleftAttachment, XmATTACH_FORM,
		 XmNrightAttachment, XmATTACH_FORM,
		 XmNbottomAttachment, XmATTACH_FORM,
		 NULL );

  XtAddCallback( list, XmNdefaultActionCallback,
		 ObjectEditCallback, (XtPointer)i );

  SetList( category, list );

  XtVaSetValues( mainWindow,
		 XmNmenuBar, menu,
		 XmNworkWindow, work,
		 NULL );

  XtManageChild( popup );
}

private void RefreshCategories( void )
{
  int i;
  category_t *category;
  char name[ BUF_SIZE ];

  for( i = 0, category = folder->categories
	 ; i < CATEGORY_WIDGET_MAX && category
	 ; i++, category = category->next ){
    if( EqualID( category->id, CAT_ADDIN ) )
      continue;
    categoryWidget[ i ].category = category;
    if( categoryWidget[ i ].button ){
      SetLabel( category, categoryWidget[ i ].button );
      if( categoryWidget[ i ].popup ){
	if( XtIsManaged( categoryWidget[ i ].popup ) ){
	  categoryWidget[ i ].dirty = 0;
	  SetLabel( category, categoryWidget[ i ].label );
	  SetHeadings( category, categoryWidget[ i ].heading );
	  SetList( category, categoryWidget[ i ].list );
	} else {
	  categoryWidget[ i ].dirty = 1;
	}
      }
    } else {
      sprintf( name, "button%d", i );
      categoryWidget[ i ].button =
	XtCreateManagedWidget( name,
			       xmPushButtonWidgetClass,
			       editSubmenu, NULL, 0 );
      SetLabel( category, categoryWidget[ i ].button );
      XtAddCallback( categoryWidget[ i ].button, XmNactivateCallback,
		     CatalogOpenCallback, (XtPointer)i );
      categoryWidget[ i ].dirty = 0;
    }
  }
  for( ; i < CATEGORY_WIDGET_MAX ; i++ ){
    if( categoryWidget[ i ].popup ){
      categoryWidget[ i ].dirty = 1;
      XtUnmanageChild( categoryWidget[ i ].popup );
    }
  }
}

private void CreateCategoryPane( Widget parent )
{
  Widget cascade;

  editSubmenu = XmCreatePulldownMenu( parent, "editSubmenu", NULL, 0 );
  cascade = XtVaCreateManagedWidget( "Category", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, editSubmenu,
				     NULL );

  RefreshCategories();
}

/**********************************************************************/

/*
 * モード
 */

private char *modeName[ MODES ] = {
  "Synchronize",
  "Backup",
  "Restore"
};

private void BackupOkCallback( Widget w, XtPointer clientData,
			       XtPointer callData )
{
  Widget text;
  char *file;

  XtUnmanageChild( w );

  text = XtNameToWidget( w, "Text" );
  file = XmTextFieldGetString( text );

  free( backupName );
  backupName = TokenAlloc( file );

  if( PASSIVE_SYNC == syncMode )
    KillChild();
  else
    Refresh();
}

private void BackupCancelCallback( Widget w, XtPointer clientData,
				  XtPointer callData )
{
  newMode = SYNCHRONIZE;

  XtUnmanageChild( w );

  if( PASSIVE_SYNC == syncMode )
    KillChild();
  else
    Refresh();
}

private void ModeCallback( Widget w, XtPointer clientData, XtPointer callData )
{
  private Widget backupDialog = NULL;
  private Widget restoreDialog = NULL;
  XmToggleButtonCallbackStruct *cbs;
  XmString title;
  Arg args[ 2 ];

  cbs = (XmToggleButtonCallbackStruct *)callData;

  if( cbs->set ){
    newMode = (int)clientData;

    switch( newMode ){
    case SYNCHRONIZE:
      if( PASSIVE_SYNC == syncMode )
	KillChild();
      else
	Refresh();
      break;
    case BACKUP:
      if( !backupDialog ){
	title = XmStringCreateLocalized( "Backup" );
	XtSetArg( args[ 0 ], XmNdialogTitle, title );
	backupDialog = XmCreateFileSelectionDialog( w, "Backup", args, 1 );
	XmStringFree( title );

	XtUnmanageChild( XmFileSelectionBoxGetChild( backupDialog, XmDIALOG_HELP_BUTTON ) );

	XtAddCallback( backupDialog, XmNokCallback,
		      BackupOkCallback, NULL );
	XtAddCallback( backupDialog, XmNcancelCallback,
		      BackupCancelCallback, NULL );
      }

      XtManageChild( backupDialog );

      break;
    case RESTORE:
      if( !restoreDialog ){
	title = XmStringCreateLocalized( "Restore" );
	XtSetArg( args[ 0 ], XmNdialogTitle, title );
	restoreDialog = XmCreateFileSelectionDialog( w, "Restore", args, 1 );
	XmStringFree( title );

	XtUnmanageChild( XmFileSelectionBoxGetChild( restoreDialog, XmDIALOG_HELP_BUTTON ) );

	XtAddCallback( restoreDialog, XmNokCallback,
		      BackupOkCallback, NULL );
	XtAddCallback( restoreDialog, XmNcancelCallback,
		      BackupCancelCallback, NULL );
      }

      XtManageChild( restoreDialog );

      break;
    }
  }
}

private void CreateModePane( Widget parent )
{
  Widget cascade, submenu;
  Arg args[ 1 ];
  int i;

  XtSetArg( args[ 0 ], XmNradioBehavior, True );
  submenu = XmCreatePulldownMenu( parent, "modeSubmenu", args, 1 );
  cascade = XtVaCreateManagedWidget( "Mode", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  for( i = 0 ; i < MODES ; i++ ){
    modeWidget[ i ] = XtVaCreateManagedWidget( modeName[ i ],
					      xmToggleButtonWidgetClass,
					      submenu,
					      XmNvisibleWhenOff, True,
					      NULL );
    XtAddCallback( modeWidget[ i ], XmNvalueChangedCallback,
		  ModeCallback, (XtPointer)i );

    if( i == mode ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( modeWidget[ i ], args, 1 );
    }
  }
}

/**********************************************************************/

/*
 * スピード
 */

private void SpeedCallback( Widget w, XtPointer clientData,
			    XtPointer callData )
{
  XmToggleButtonCallbackStruct *cbs;

  cbs = (XmToggleButtonCallbackStruct *)callData;

  if( cbs->set ){
    speed = (long)clientData;
    if( PASSIVE_SYNC == syncMode )
      KillChild();
    else
      Refresh();
  }
}

private void CreateSpeedPane( Widget parent )
{
  Widget cascade, submenu;
  Arg args[ 1 ];
  int i;

  XtSetArg( args[ 0 ], XmNradioBehavior, True );
  submenu = XmCreatePulldownMenu( parent, "speedSubmenu", args, 1 );
  cascade = XtVaCreateManagedWidget( "Speed", xmCascadeButtonWidgetClass,
				    parent,
				    XmNsubMenuId, submenu,
				    NULL );

  for( i = 0 ; i < SPEEDS ; i++ ){
    speedWidget[ i ] = XtVaCreateManagedWidget( speedName[ i ].name,
					       xmToggleButtonWidgetClass,
					       submenu,
					       XmNvisibleWhenOff, True,
					       NULL );
    XtAddCallback( speedWidget[ i ], XmNvalueChangedCallback,
		  SpeedCallback, (XtPointer)speedName[ i ].speed );

    if( speedName[ i ].speed == speed ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( speedWidget[ i ], args, 1 );
    }
  }
}

/**********************************************************************/

/*
 * シンクロナイズモード
 */

private void SyncModeCallback( Widget w, XtPointer clientData,
			       XtPointer callData )
{
  XmToggleButtonCallbackStruct *cbs;
  int old_mode = syncMode;

  cbs = (XmToggleButtonCallbackStruct *)callData;

  if( cbs->set ){
    syncMode = (long)clientData;
    if( PASSIVE_SYNC == syncMode ){
      if( PASSIVE_SYNC != old_mode )
	Refresh();
    } else {
      if( PASSIVE_SYNC == old_mode )
	KillChild();
    }
  }
}

private void CreateSyncModePane( Widget parent )
{
  Widget cascade, submenu;
  Arg args[ 1 ];
  int i;

  XtSetArg( args[ 0 ], XmNradioBehavior, True );
  submenu = XmCreatePulldownMenu( parent, "syncModeSubmenu", args, 1 );
  cascade = XtVaCreateManagedWidget( "Sync Mode", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  syncModeWidget[ 0 ] = XtVaCreateManagedWidget( "Active Sync",
						 xmToggleButtonWidgetClass,
						 submenu,
						 XmNvisibleWhenOff, True,
						 NULL );

  syncModeWidget[ 1 ] = XtVaCreateManagedWidget( "Passive Sync",
						 xmToggleButtonWidgetClass,
						 submenu,
						 XmNvisibleWhenOff, True,
						 NULL );

  for( i = 0 ; i < 2 ; i++ ){
    XtAddCallback( syncModeWidget[ i ], XmNvalueChangedCallback,
		   SyncModeCallback, (XtPointer)i );
    if( i == syncMode ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( syncModeWidget[ i ], args, 1 );
    }
  }
}

/**********************************************************************/

private void ImageSizeCallback( Widget w, XtPointer clientData,
				XtPointer callData )
{
  XmToggleButtonCallbackStruct *cbs;

  cbs = (XmToggleButtonCallbackStruct *)callData;

  if( cbs->set ){
    imageSize = (long)clientData;
  }
}

private void CreateImageSizePane( Widget parent )
{
  Widget cascade, submenu;
  Arg args[ 1 ];
  int i;

  XtSetArg( args[ 0 ], XmNradioBehavior, True );
  submenu = XmCreatePulldownMenu( parent, "imageSizeSubmenu", args, 1 );
  cascade = XtVaCreateManagedWidget( "Image Size", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  imageSizeWidget[ 0 ] = XtVaCreateManagedWidget( "Small (147x104)",
						  xmToggleButtonWidgetClass,
						  submenu,
						  XmNvisibleWhenOff, True,
						  NULL );

  imageSizeWidget[ 1 ] = XtVaCreateManagedWidget( "Large (292x172)",
						  xmToggleButtonWidgetClass,
						  submenu,
						  XmNvisibleWhenOff, True,
						  NULL );

  for( i = 0 ; i < 2 ; i++ ){
    XtAddCallback( imageSizeWidget[ i ], XmNvalueChangedCallback,
		   ImageSizeCallback, (XtPointer)i );
    if( i == imageSize ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( imageSizeWidget[ i ], args, 1 );
    }
  }
}

/**********************************************************************/

private void AutoSaveCallback( Widget w, XtPointer clientData,
			       XtPointer callData )
{
  XmToggleButtonCallbackStruct *cbs;

  cbs = (XmToggleButtonCallbackStruct *)callData;

  if( cbs->set ){
    autoSave = (long)clientData;
  }
}

private void CreateAutoSavePane( Widget parent )
{
  Widget cascade, submenu;
  Arg args[ 1 ];
  int i;

  XtSetArg( args[ 0 ], XmNradioBehavior, True );
  submenu = XmCreatePulldownMenu( parent, "autoSaveSubmenu", args, 1 );
  cascade = XtVaCreateManagedWidget( "Auto Save", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  autoSaveWidget[ 0 ] = XtVaCreateManagedWidget( "True",
						 xmToggleButtonWidgetClass,
						 submenu,
						 XmNvisibleWhenOff, True,
						 NULL );

  autoSaveWidget[ 1 ] = XtVaCreateManagedWidget( "False",
						 xmToggleButtonWidgetClass,
						 submenu,
						 XmNvisibleWhenOff, True,
						 NULL );

  for( i = 0 ; i < 2 ; i++ ){
    XtAddCallback( autoSaveWidget[ i ], XmNvalueChangedCallback,
		   AutoSaveCallback, (XtPointer)i );
    if( i == autoSave ){
      XtSetArg( args[ 0 ], XmNset, True );
      XtSetValues( autoSaveWidget[ i ], args, 1 );
    }
  }
}

/**********************************************************************/

/*
 * オプション
 */

private void DeviceOkCallback( Widget w, XtPointer clientData,
			       XtPointer callData )
{
  char *str;

  free( deviceName );
  str = XmTextFieldGetString( (Widget)clientData );
  deviceName = TokenAlloc( str );
  XtFree( (XtPointer)str );

  if( PASSIVE_SYNC == syncMode )
    KillChild();
  else
    Refresh();
}

private void DeviceCancelCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  XmTextFieldSetString( (Widget)clientData, deviceName );
}

private Widget CreateDeviceDialog()
{
  Widget dialog, rc;
  Widget deviceWidget;
  XmString title;
  Arg args[ 2 ];

  title = XmStringCreateLocalized( "Device" );
  XtSetArg( args[ 0 ], XmNdialogTitle, title );
  dialog = XmCreateMessageDialog( shell, "Device", args, 1 );
  XmStringFree( title );

  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_SYMBOL_LABEL ) );
  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_MESSAGE_LABEL ) );
  XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

  rc = XtVaCreateManagedWidget( "rc", xmRowColumnWidgetClass,
				dialog,
				XmNpacking,	XmPACK_TIGHT,
				XmNorientation,	XmHORIZONTAL,
				NULL );

  XtCreateManagedWidget( "Device", xmLabelWidgetClass,
			 rc, NULL, 0 );
  deviceWidget = XtVaCreateManagedWidget( "Device",
					  xmTextFieldWidgetClass,
					  rc,
					  NULL );

  XmTextFieldSetString( deviceWidget, deviceName );

  XtAddCallback( dialog, XmNokCallback,
		 DeviceOkCallback, deviceWidget );
  XtAddCallback( dialog, XmNcancelCallback,
		 DeviceCancelCallback, deviceWidget );

  return dialog;
}

void DeviceOpenCallback( Widget w, XtPointer clientData, XtPointer callData )
{
  private Widget dialog = NULL;

  if( NULL == dialog )
    dialog =  CreateDeviceDialog();

  XtManageChild( dialog );
}

private void FolderOkCallback( Widget w, XtPointer clientData,
			       XtPointer callData )
{
  char *file;
  int len;
  Arg args[ 1 ];
  struct stat sb;
  Widget text;

  XtUnmanageChild( w );

  text = XtNameToWidget( w, "Text" );
  file = XmTextFieldGetString( text );

  if( 0 > stat( file, &sb ) || S_IFDIR != ( sb.st_mode & S_IFMT ) ){
    XtSetArg( args[ 0 ], XmNdirSpec, folderString );
    XtSetValues( w, args, 1 );
    XtFree( file );
  } else {
    free( folderName );
    folderName = TokenAlloc( file );
    XtFree( file );

    len = strlen( folderName );
    if( len > 2 && '/' == folderName[ len - 1 ] )
      folderName[ len - 1 ] = '\0';

    XmStringFree( folderString );
    folderString = XmStringCreateLocalized( folderName );
    XtSetArg( args[ 0 ], XmNdirSpec, folderString );
    XtSetValues( w, args, 1 );

    FolderReload();

  if( PASSIVE_SYNC == syncMode )
    KillChild();
  else
    Refresh();
  }
}

private void FolderCancelCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  Arg args[ 1 ];

  XtUnmanageChild( w );

  XtSetArg( args[ 0 ], XmNdirSpec, folderString );
  XtSetValues( w, args, 1 );
}

private void FolderOpenCallback( Widget w, XtPointer clientData,
				 XtPointer callData )
{
  private Widget dialog = NULL;
  XmString title;
  Arg args[ 2 ];

  if( NULL == dialog ){
    title = XmStringCreateLocalized( "Folder" );
    XtSetArg( args[ 0 ], XmNdialogTitle, title );
    dialog = XmCreateFileSelectionDialog( w, "Folder", args, 1 );
    XmStringFree( title );

    XtUnmanageChild( XmFileSelectionBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

    XtSetArg( args[ 0 ], XmNfileTypeMask, XmFILE_DIRECTORY );
    XtSetArg( args[ 1 ], XmNdirectory, folderString );
    XtSetValues( dialog, args, 2 );

    XtAddCallback( dialog, XmNokCallback,
		   FolderOkCallback, NULL );
    XtAddCallback( dialog, XmNcancelCallback,
		   FolderCancelCallback, NULL );
  }

  XtManageChild( dialog );
}

private void CreateOptionPane( Widget parent )
{
  Widget cascade, submenu, folderButton, deviceWidget;

  submenu = XmCreatePulldownMenu( parent, "optionSubmenu", NULL, 0 );
  cascade = XtVaCreateManagedWidget( "Options", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );

  folderButton = XtCreateManagedWidget( "Folder", xmPushButtonWidgetClass,
					submenu,
					NULL, 0 );
  XtAddCallback( folderButton, XmNactivateCallback,
		 FolderOpenCallback, NULL );

  deviceWidget = XtCreateManagedWidget( "Device", xmPushButtonWidgetClass,
					submenu,
					NULL, 0 );
  XtAddCallback( deviceWidget, XmNactivateCallback,
		 DeviceOpenCallback, NULL );

  CreateSpeedPane( submenu );
  CreateImageSizePane( submenu );
  CreateSyncModePane( submenu );
  CreateAutoSavePane( submenu );
}

/**********************************************************************/

/*
 * ヘルプ
 */

private void HelpOkCallback( Widget w, XtPointer clientData,
			     XtPointer callData )
{
  XtUnmanageChild( w );
}

private void AboutXMclinkCallback( Widget w, XtPointer clientData,
				   XtPointer callData )
{
  private Widget dialog = NULL;
  XmString str;

  if( NULL == dialog ){
    dialog = XmCreateMessageDialog( w, "About xmclink", NULL, 0 );

    XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON ) );
    XtUnmanageChild( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON ) );

    str = XmStringCreateLocalized( "About xmclink" );
    XtVaSetValues( dialog,
		   XmNdialogTitle, str,
		   NULL );
    XmStringFree( str );

    str = XmStringCreateLocalized(
	"xmclink " VERSION "\n"
	"All rights reserved. Copyright (C) 1998,2001 by NARITA Tomio.\n\n"
	"xmclink is a Motif wrapper for caleidlink.\n\n"
	"このプログラムは XmI18Nライブラリによる Motif アプリケーションの\n"
	"国際化のデモンストレーションとして実装されました.\n"
	);
    XtVaSetValues( dialog,
		   XmNmessageString, str,
		   NULL );
    XmStringFree( str );

    XtAddCallback( dialog, XmNokCallback, HelpOkCallback, NULL );
  }

  XtManageChild( dialog );
}

private void CreateHelpPane( Widget parent )
{
  Widget cascade, submenu, buttonHelp;

  submenu = XmCreatePulldownMenu( parent, "helpSubmenu", NULL, 0 );
  cascade = XtVaCreateManagedWidget( "Help", xmCascadeButtonWidgetClass,
				     parent,
				     XmNsubMenuId, submenu,
				     NULL );
  XtVaSetValues( parent,
		 XmNmenuHelpWidget, cascade,
		 NULL );

  buttonHelp = XtCreateManagedWidget( "About xmclink", xmPushButtonWidgetClass,
				      submenu, NULL, 0 );
  XtAddCallback( buttonHelp, XmNactivateCallback, AboutXMclinkCallback, NULL );
}

/**********************************************************************/

/*
 * メニューバー
 */

private Widget CreateMenuBar( Widget parent )
{
  Widget menu;

  menu = XmCreateMenuBar( parent, "menu", NULL, 0 );

  CreateFilePane( menu );
  CreateCategoryPane( menu );
  CreateModePane( menu );
  CreateOptionPane( menu );
  CreateHelpPane( menu );

  XtManageChild( menu );

  return menu;
}

/**********************************************************************/

private void AnimCallback( Widget w, XtPointer clientData, XtPointer callData )
{
  if( 0 == child ){
    if( PASSIVE_SYNC == syncMode ){
      Refresh();
    } else {
      if( 0 == child )
	ForkProcess();

      if( 0 != child && -1 != childFd ){
	inpId = XtAppAddInput( XtWidgetToApplicationContext( animWidget ),
			       childFd,
			       (XtPointer)XtInputReadMask,
			       ReadFromClink,
			       NULL );
	AnimStart();
      }
    }
  } else {
    KillChild();
  }
}

private void DrawnCallback( Widget w, XtPointer clientData,
			    XtPointer callData )
{
  AnimSet( pixIndex );
}

private boolean_t CreateImagePanel( Widget parent )
{
  Display *dpy;
  Pixmap mask;
  XpmAttributes attr;
  int i, status;
  char file[ 256 ];
  XGCValues values;

  dpy = XtDisplay( parent );

  /* animation button */

  animWidget = XtCreateManagedWidget( "drawnButton", xmDrawnButtonWidgetClass,
				     parent, NULL, 0 );

  XtVaGetValues( animWidget,
		XmNforeground,	&values.foreground,
		XmNbackground,	&values.background,
		XmNdepth,	&attr.depth,
		XmNcolormap,	&attr.colormap,
		NULL );

  animGC = XtGetGC( animWidget, GCForeground | GCBackground, &values );

  attr.visual = DefaultVisual( dpy, DefaultScreen( dpy ) );
  attr.valuemask = XpmDepth | XpmColormap | XpmVisual;

  for( i = 0 ; i < PIX_COUNT ; i++ ){
    strcpy( file, PIXMAP_DIRECTORY );
    sprintf( file + strlen( file ), "/clink%d.xpm", i );

    status = XpmReadFileToPixmap( dpy,
				  DefaultRootWindow( dpy ),
				  file,
				  &pixClink[ i ],
				  &mask,
				  &attr );
    if( XpmSuccess != status ){
      fprintf( stderr, "xmclink: cannot load pixmap %s (status=%d)\n", file, status );
      exit( 1 );
    }

    if( mask )
      XFreePixmap( dpy, mask );
  }

  if( PIX_COUNT == i ){
    pixIndex = 0;
    XtVaSetValues( animWidget,
		   XmNtraversalOn, False,
		   XmNshadowType, XmSHADOW_ETCHED_OUT,
		   XmNshadowThickness, 4,
		   XmNwidth, 64 + 12,
		   XmNheight, 48 + 12,
		   NULL );
  } else {
    while( --i >= 0 ){
      XFreePixmap( dpy, pixClink[ i ] );
    }

    return FALSE;
  }

  XtAddCallback( animWidget, XmNexposeCallback, DrawnCallback, NULL );
  XtAddCallback( animWidget, XmNactivateCallback, AnimCallback, NULL );

  /* mode label */

  modeLabelWidget = XtCreateManagedWidget( "modeLabel", xmLabelWidgetClass,
					   parent,
					   NULL, 0 );

  if( NULL != modeString )
    XmStringFree( modeString );
  modeString = XmStringCreateLocalized( modeLabel[ mode ] );

  XtVaSetValues( modeLabelWidget,
		 XmNlabelString, modeString,
		 XmNisAligned, True,
		 XmNalignment, XmALIGNMENT_CENTER,
		 NULL );

  return TRUE;
}

private boolean_t CreateMessagePanel( Widget parent )
{
  folderLabelWidget = XtCreateManagedWidget( "folderLabel", xmTextFieldWidgetClass,
					     parent,
					     NULL, 0 );

  XtVaSetValues( folderLabelWidget,
		 XmNcolumns, 16,
		 XmNtraversalOn, False,
		 XmNeditable, False,
		 XmNcursorPositionVisible, False,
		 XmNverifyBell, False,
		 NULL );

  XmTextFieldSetString( folderLabelWidget, folderName );

  msgWidget = XtCreateManagedWidget( "Message", xmTextFieldWidgetClass,
				     parent,
				     NULL, 0 );

  XtVaSetValues( msgWidget,
		 XmNbackground, WhitePixelOfScreen( XtScreen( parent ) ),
		 XmNcolumns, 16,
		 XmNtraversalOn, False,
		 XmNeditable, False,
		 XmNcursorPositionVisible, False,
		 XmNverifyBell, False,
		 NULL );

  return TRUE;
}

/*
 * トップレベル
 */

int main( int argc, char **argv )
{
  XtAppContext app;
  Widget mainWindow, menu, work, rcLeft, rcRight;

  {
    struct timeval tv;

    gettimeofday( &tv, NULL );
    srandom( tv.tv_usec );
  }

  /*
   * 定義ファイルの読み込み & パラメータの初期化
   */
  LoadConfig( CONFIG_FILE );

  /*
   * フォルダの読み込み
   */
  folder = FolderLoad( folderName, EUC_JAPAN );

  /*
   * Xt の初期化
   */
  XtSetLanguageProc( NULL, NULL, NULL );
  shell = XtAppInitialize( &app, "XMclink", NULL, 0,
			   &argc, argv, NULL, NULL, 0 );

  XtAddCallback( shell, XmNdestroyCallback, DestroyCallback, NULL );

  sigId = XtAppAddSignal( app, SignalHandler, (XtPointer)&childStatus );

  /*
   * メインウィンドウの生成
   */
  mainWindow = XtCreateManagedWidget( "mainWindow", xmMainWindowWidgetClass,
				      shell,
				      NULL, 0 );

  menu = CreateMenuBar( mainWindow );

  work = XtCreateManagedWidget( "work", xmFormWidgetClass,
				mainWindow,
				NULL, 0 );

  /***/

  rcLeft = XtVaCreateManagedWidget( "imagePanel", xmRowColumnWidgetClass,
				    work,
				    XmNpacking,		XmPACK_TIGHT,
				    XmNorientation,	XmVERTICAL,
				    XmNtopAttachment,	XmATTACH_FORM,
				    XmNbottomAttachment,XmATTACH_FORM,
				    XmNleftAttachment,	XmATTACH_FORM,
				    NULL );
  CreateImagePanel( rcLeft );

  rcRight = XtVaCreateManagedWidget( "messagePanel", xmRowColumnWidgetClass,
				     work,
				     XmNpacking,	XmPACK_TIGHT,
				     XmNorientation,	XmVERTICAL,
				     XmNtopAttachment,	XmATTACH_FORM,
				     XmNbottomAttachment, XmATTACH_FORM,
				     XmNleftAttachment,	XmATTACH_WIDGET,
				     XmNleftWidget,	rcLeft,
				     XmNrightAttachment,XmATTACH_FORM,
				     NULL );
  CreateMessagePanel( rcRight );

  /***/

  XtVaSetValues( mainWindow,
		 XmNmenuBar, menu,
		 XmNworkWindow, work,
		 NULL );

  /*
   * ウィジットのリアライズ
   */
  XtRealizeWidget( shell );

  signal( SIGCHLD, ChildHandler );
  signal( SIGTERM, KillHandler );

  Refresh();

  XtVaSetValues( shell,
		 XmNiconPixmap, pixClink[ 1 ],
		 NULL );

#if 1
  {
    //Dimension width;
    Dimension height;

    /*
     * window resize を止める
     */
    XtVaGetValues( shell,
		   //XmNwidth, &width,
		   XmNheight, &height,
		   NULL );
    //fprintf( stderr, "width:%d, height:%d\n", width, height );
    XtVaSetValues( shell,
		   //XmNminWidth, width,
		   //XmNmaxWidth, width,
		   XmNminHeight, height,
		   XmNmaxHeight, height,
		   NULL );
  }
#endif

  XtAppMainLoop( app );

  exit( 0 );
}
