#!/usr/bin/perl

########################################################################
#
# gif2quickform.pl
#
# All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
#
# $Id: gif2quickform.pl,v 1.8 2001/03/16 08:41:12 nrt Exp $
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

# EM-10/RX-50/PV
$SMALL_WIDTH = 147;
$SMALL_HEIGHT = 104;

# CALEID
$LARGE_WIDTH = 292;
$LARGE_HEIGHT = 172;

$width = $SMALL_WIDTH;
$height = $SMALL_HEIGHT;

$tmp = "/tmp/gif2quickform.$$";

for( @ARGV ){
  if( /^-l/ ){
    $width = $LARGE_WIDTH;
    $height = $LARGE_HEIGHT;
  } elsif( /^-s/ ){
    $width = $SMALL_WIDTH;
    $height = $SMALL_HEIGHT;
  } elsif( /^-c/ ){
    $cutting = 1;
    $xbase = 0;
    $ybase = 0;
  } elsif( /^-a/ ){
    $aspect = 1;
  } elsif( '' eq $infile ){
    $infile = $_;
  } elsif( '' eq $outfile ){
    $outfile = $_;
  }
}

if( '' eq $infile ){
  die "Usage: $0 [-cut] [-large] gif_filename [quickform_filename]\n";
}

if( '' ne $outfile ){
  open( STDOUT, "> $outfile" ) || die "cannot open $outfile\n";
}

$width_word = int( $width / 32 );
if( 0 != $width % 32 ){
  $width_word++;
}

$image_size = $width_word * 4 * $height;
$file_size = $image_size + 62;

`anytopnm $infile > $tmp 2> /dev/null`;

if( 0 != $? ){
  `djpeg -pnm $infile > $tmp 2> /dev/null`;
}

if( 0 != $? ){
  unlink( $tmp );
  die "maybe $infile isn't gif or jpeg...\n";
}

$proc = "cat $tmp";

if( 0 == $aspect || 3 * $width == 4 * $height ){
  $newwidth = $width;
  $newheight = $height;
} elsif( $width / $height > 4 / 3 ) {
  $newwidth = $height * 4 / 3;
  $newheight = $height;
} else {
  $newwidth = $width;
  $newheight = $width / 4 * 3;
}

if( $cutting ){
  $cut = "| pnmcut $xbase $ybase $newwidth $newheight ";
  $proc .= $cut;
} else {
  $scale = "| pnmscale -xsize $newwidth -ysize $newheight ";
  $proc .= $scale;
}

if( $newwidth < $width ){
  $diff = $width - $newwidth;
  $proc .= "| pnmpad -white -r$diff ";
}
if( $newheight < $height ){
  $diff = $height - $newheight;
  $proc .= "| pnmpad -white -t$diff ";
}

$proc .= "| ppmtopgm | pgmtopbm | pbmtopgm 1 1 ";
$proc .= "2> /dev/null";

$content = `$proc`;

unlink( $tmp );

if( $? ){
  die "pnm process failed";
}

$content =~ s/^.+\n.+\n.+\n//;

for( $y = 0 ; $y < $height ; $y++ ){
  $line[ $y ] = '';
  $x = 0;
  $high = $width_word * 32;
  while( $x < $high ){
    $ch = 0;
    for( $bit = 7 ; $bit >= 0 ; $bit-- ){
      if( $x < $width ){
	$dot = unpack( "C", substr( $content, 0, 1 ) );
	substr( $content, 0, 1 ) = '';
      } else {
	$dot = 1;
      }
      $x++;
      if( 0 == $dot ){
	$ch |= (1 << $bit);
      }
    }
    $line[ $y ] .= pack( "C", $ch );
  }
}

########################################################################

@preamble = (
	     0x42, 0x4d, 0x5e, 0x08, 0x00, 0x00, 0x00, 0x00,	#  0- 7
	     0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x28, 0x00,	#  8-15
	     0x00, 0x00, 0x93, 0x00, 0x00, 0x00, 0x68, 0x00,	# 16-23
	     0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,	# 24-31
	     0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00,	# 32-39
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	# 40-47
	     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,	# 48-55
	     0xff, 0x00, 0x00, 0x00, 0x00, 0x00			# 56-61
	     );

$preamble[ 2 ] = $file_size & 0xff;	# little endian
$preamble[ 3 ] = $file_size >> 8;

$preamble[ 18 ] = $width & 0xff;
$preamble[ 19 ] = $width >> 8;

$preamble[ 22 ] = $height & 0xff;
$preamble[ 23 ] = $height >> 8;

$preamble[ 34 ] = $image_size & 0xff;
$preamble[ 35 ] = $image_size >> 8;

$content = '';

for( $i = 0 ; $i < 62 ; $i++ ){
  $content .= pack( "C", $preamble[ $i ] );
}

for( $y = $height - 1; $y >= 0 ; $y-- ){
  $content .= $line[ $y ];
}

########################################################################

$len = 0;
while( $len < $file_size ){
  $line = '';
  for( $i = 0 ; $i < 512 ; $i++ ){
    $line .= substr( $content, 0, 1 );
    substr( $content, 0, 1 ) = '';
    $len++;
    last if $len == $file_size;
  }
  $res = &EncodeBase64( $line );
  if( $len == $file_size ){
    print "102000 102000 $res\n";
  } else {
    print "102001 102001 $res\n";
  }
}

exit( 0 );

sub EncodeBase64 {
  @base64char = (
		 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		 'w', 'x', 'y', 'z', '0', '1', '2', '3',
		 '4', '5', '6', '7', '8', '9', '+', '/'
		 );

  local( $str ) = @_;
  local( $ch, $ptr, $len, $res );

  $len = length( $str );
  $res = '';

  for( $ptr = 0 ; $ptr < $len ; $ptr++ ){
    $ch = unpack("C", substr( $str, $ptr, 1 )) >> 2;
    $res .= $base64char[ $ch ];
    $ch = ( unpack("C", substr( $str, $ptr, 1 )) & 0x03 ) << 4;

    $ptr++;
    if( $ptr < $len ){
      $ch |= ( unpack("C", substr( $str, $ptr, 1 )) & 0xf0 ) >> 4;
    }
    $res .= $base64char[ $ch ];

    if( $ptr >= $len ){
      $res .= '=';
      $res .= '=';
      last;
    }
    $ch = ( unpack("C", substr( $str, $ptr, 1 )) & 0x0f ) << 2;

    $ptr++;
    if( $ptr < $len ){
      $ch |= ( unpack("C", substr( $str, $ptr, 1 )) & 0xc0 ) >> 6;
    }
    $res .= $base64char[ $ch ];

    if( $ptr >= $len ){
      $res .= '=';
      last;
    }
    $ch = unpack("C", substr( $str, $ptr, 1 )) & 0x3f;

    $res .= $base64char[ $ch ];
  }

  return $res;
}
