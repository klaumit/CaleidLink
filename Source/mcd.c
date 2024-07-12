/*
 * mcd.c
 * All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
 * $Id: mcd.c,v 1.4 1999/01/11 06:36:55 nrt Exp $
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

#include <dos.h>

#include <import.h>
#include <begin.h>
#include <mcd.h>

private void _asm_PUSH(char *);
private void _asm_POP(char *);
private int _asm_DSEG(char *);

#define push() _asm_PUSH("\n\tpush cx\n\tpush dx")
#define pop() _asm_POP("\n\tpop dx\n\tpop cx")

#define dseg() _asm_DSEG("\n\tmov ax, ds")

typedef struct {
  int sendBufSize;
  char far *sendBuf;
  int receiveBufSize;
  char far *receiveBuf;
} mcd_modify_buf_t;

#define GET_BUF  0
#define SET_BUF  1

private unsigned (far *mcdLineStat)( int, ... );
private void (far *mcdBufStat)( int, ... );
private unsigned (far *mcdBlockRead)( char far *, unsigned, ... );
private void (far *mcdBreakSig)( int, ... );
private void (far *mcdModifyBuffer)( int, mcd_modify_buf_t far *, ... );
private int (far *mcdGetc)( void, ... );
private int (far *mcdPutc)( int, ... );
private unsigned (far *mcdGetSbufChars)( void, ... );
private unsigned (far *mcdGetSbufFree)( void, ... );
private unsigned (far *mcdGetRbufChars)( void, ... );
private void (far *mcdSetSpeed)( int, unsigned long, ... );
private unsigned (far *mcdLineCtrl)( unsigned, ... );

private int res;
private union REGS regs;

private char far *mcdBlock;

typedef struct {
  int sBufChars;
  int rBufChars;
  int xOffRecved;
  int xOffSent;
} buffer_status;

private buffer_status far bufferStatus;

public int Axopen( char *path )
{
  int handle;

  struct {
    int no;
    unsigned far *offs;
    unsigned segment;
    int tablesize;
  } tag;

  regs.x.ax = 0x3d02;
  regs.x.dx = (int)path;
  handle = int86( 0x21, &regs, &regs );

  if( !regs.x.cflag ){
    tag.no = 21;
    regs.x.ax = 0x4402;
    regs.x.bx = handle;
    regs.x.cx = 10;
    regs.x.dx = (int)&tag;
    int86( 0x21, &regs, &regs );
    if( !regs.x.cflag ){
      mcdLineStat = MK_FP( tag.segment, tag.offs[ 2 ] );
      mcdBufStat = MK_FP( tag.segment, tag.offs[ 3 ] );
      mcdBlockRead = MK_FP( tag.segment, tag.offs[ 4 ] );
      mcdBreakSig = MK_FP( tag.segment, tag.offs[ 6 ] );
      mcdModifyBuffer = MK_FP( tag.segment, tag.offs[ 7 ] );
      mcdGetc = MK_FP( tag.segment, tag.offs[ 8 ] );
      mcdPutc = MK_FP( tag.segment, tag.offs[ 9 ] );
      mcdGetSbufChars = MK_FP( tag.segment, tag.offs[ 10 ] );
      mcdGetSbufFree = MK_FP( tag.segment, tag.offs[ 11 ] );
      mcdGetRbufChars = MK_FP( tag.segment, tag.offs[ 12 ] );
      mcdSetSpeed = MK_FP( tag.segment, tag.offs[ 15 ] );
      mcdLineCtrl = MK_FP( tag.segment, tag.offs[ 18 ] );

      mcdBlock = (char far *)MK_FP( dseg(), axBuf );

      return handle;
    }
  }
  return -1;
}

public void Axclose( int handle )
{
  regs.h.ah = 0x3e;
  regs.x.bx = handle;
  int86( 0x21, &regs, &regs );
}

public char Axin()
{
  push();
  res = mcdGetc();
  pop();
  return (char)res;
}

public unsigned Axgets()
{
  push();
  res = mcdBlockRead( mcdBlock, BLOCK_SIZE );
  pop();
  return res;
}

public boolean_t CanGet()
{
  push();
  res = mcdGetRbufChars();
  pop();
  return res ? TRUE : FALSE;
}

public boolean_t Rest()
{
  push();
  res = mcdGetSbufChars();
  pop();
  return res ? TRUE : FALSE;
}

public boolean_t CanPut()
{
  push();
  res = mcdGetSbufFree();
  pop();
  return res ? TRUE : FALSE;
}

public void Axout( char c )
{
  push();
  mcdPutc( (int)c );
  pop();
}

public void SendBreak()
{
  push();
  mcdBreakSig( 1 );
  pop();
}

public boolean_t Connected()
{
  push();
  res = mcdLineStat( 0 );
  pop();
  return 0 != (res & 0x2000) ? TRUE : FALSE;
}

public boolean_t Rts()
{
  push();
  res = mcdLineStat( 0 );
  pop();
  return 0 != (res & 0x0020) ? TRUE : FALSE;
}

public boolean_t DTRoff()
{
  push();
  mcdLineCtrl( 2 );
  pop();
}

public boolean_t DTRon()
{
  push();
  mcdLineCtrl( 3 );
  pop();
}

public boolean_t XoffSent()
{
  push();
  mcdBufStat( 0, &bufferStatus );
  pop();

  return bufferStatus.xOffSent != 0 ? TRUE : FALSE;
}

public void Xoff()
{
  push();
  mcdBufStat( 1, 0x0100 );
  pop();
}

public void Xon()
{
  push();
  mcdBufStat( 1, 0x0200 );
  pop();
}

public void SetSpeed( unsigned long speed )
{
  push();
  mcdSetSpeed( 1, speed );
  pop();
}
