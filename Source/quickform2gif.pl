#!/usr/bin/perl

########################################################################
#
# quickform2gif.pl
#
# All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
#
# $Id: quickform2gif.pl,v 1.7 2001/03/16 08:41:12 nrt Exp $
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice unmodified, this list of conditions, and the following
#    disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
########################################################################

for( @ARGV ){
  if( '' eq $infile ){
    $infile = $_;
  } elsif( '' eq $outfile ){
    $outfile = $_;
  }
}

if( '' eq $infile ){
  die "Usage: $0 quickform_filename [gif_filename]\n";
}

if( '' ne $outfile ){
  open( STDOUT, "> $outfile" ) || die "cannot open $outfile\n";
}

open( FILE, "$infile" ) || die "cannot open $infile\n";

while( <FILE> ){
  chop;
  if( /^102001 / || /^102000/ ){
    ( $heading1, $heading2, $str ) = split( /[ ]+/ );
    if( '' eq $str ){
      $str = $heading2;
    }
    $content .= &DecodeBase64( $str );
  }
}

close( FILE );

$preamble = substr( $content, 0, 62 );
$content = substr( $content, 62, length( $content ) - 62 );

#for( $i = 0 ; $i < 62 ; $i++ ){
#  printf( "%c", unpack( "C", substr( $preamble, $i, 1 ) ) );
#}
#exit( 0 );

# little endian

$width = (unpack("C", substr( $preamble, 19, 1 )) << 8) | unpack("C", substr( $preamble, 18, 1 ));
$height = (unpack("C", substr( $preamble, 23, 1 )) << 8) | unpack("C", substr( $preamble, 22, 1 ));

$width_word = int( $width / 32 );
if( 0 != $width % 32 ){
  $width_word++;
}

for( $y = 0 ; $y < $height ; $y++ ){
  $line[ $y ] = '';
  $x = 0;
  $high = $width_word * 32;
  while( $x < $high ){
    $byte = unpack("C", substr( $content, 0, 1 ));
    substr( $content, 0, 1 ) = '';
    for( $bit = 7 ; $bit >= 0 ; $bit-- ){
      if( $x < $width ){
	if( $byte & (1 << $bit) ){
	  $line[ $y ] .= "\x00";
	} else {
	  $line[ $y ] .= "\x01";
	}
      }
      $x++;
    }
  }
}

$proc = "| ppmtogif -interlace -transparent rgb:FF/FF/FF ";
if( '' ne $outfile ){
  $proc .= " > $outfile";
}
$proc .= " 2> /dev/null";

open( PROC, $proc ) || die "cannot exec pnm process\n";

print PROC "P5\n";
print PROC "$width\n";
print PROC "$height\n";
print PROC "1\n";

for( $y = $height - 1 ; $y >= 0 ; $y-- ){
  print PROC $line[ $y ];
}

close( PROC );

exit( 0 );

sub DecodeBase64 {
  @BASE64 = (
#  0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # 0
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # 1
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, # 2
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, # 3
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, # 4
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, # 5
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, # 6
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, # 7
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # 8
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # 9
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # a
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # b
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # c
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # d
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, # e
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  # f
  );

  local( $base64 ) = @_;

  $res = '';
  $ptr = 0;
  $idx = 0;
  for( $val = 1 ; $val >= 0 ; ){
    $ch = unpack( "C", substr( $base64, $ptr++, 1 ) );
    if( 0 <= ($val = $BASE64[ $ch ]) ){
      $acc = $val << 10;
      $ch = unpack( "C", substr( $base64, $ptr++, 1 ) );
      if( 0 <= ($val = $BASE64[ $ch ]) ){
	$acc |= $val << 4;
	$res .= pack( "C", $acc >> 8 );
	$acc = ( $acc & 0x00ff ) << 8;
	$ch = unpack( "C", substr( $base64, $ptr++, 1 ) );
	if( 0 <= ($val = $BASE64[ $ch ]) ){
	  $acc |= $val << 6;
	  $res .= pack( "C", $acc >> 8 );
	  $acc = ( $acc & 0x00ff ) << 8;
	  $ch = unpack( "C", substr( $base64, $ptr++, 1 ) );
	  if( 0 <= ($val = $BASE64[ $ch ]) ){
	    $acc |= $val << 8;
	    $res .= pack( "C", $acc >> 8 );
	  }
	}
      }
    }
  }

  return $res;
}
