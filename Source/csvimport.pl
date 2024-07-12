#! /usr/bin/perl

########################################################################
#
# csvimport.pl
#
# All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
#
# $Id: csvimport.pl,v 1.8 2001/02/06 11:47:50 nrt Exp $
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

# ���ϥե������Ⱦ�ѥ������ʤ��ޤޤ�ʤ����Ȥ��ǧ���Ʋ�����.
$file = '';

# euc-jp �˥����ɷ��Ѵ����륳�ޥ��
$EUCconv = "lv -Oe";
#$EUCconv = "nkf -e"; # ?

$file = '';
$title = 'CSV';

for( @ARGV ){
  if( '' eq $file ){
    $file = $_;
  } else {
    $title = &TruncateStr( $_, 16 );
  }
}

if( '' eq $file ){
  die "Usage: $0 Excel_CSV_filename CSV_Title\n";
}

open( CSV, "$EUCconv $file |" ) || die "cannot open $file\n";

@fieldLength = 0;
$lines = 0;
$columns = 0;

while( <CSV> ){
  $lines++;
  chop;

  @fields = &csvSplit( $_ );
  $dim = 0;
  for( @fields ){
    if( length >= $fieldLength[ $dim ] ){
      $fieldLength[ $dim ] = length;
    }
    $dim++;
    if( $dim > $columns ){
      $columns = $dim;
    }
  }
}

close( CVS );

####

open( FILE, "| $EUCconv | cat" ) || die "cannot exec $SJconv\n";

if( $columns > 26 ){
  print STDERR "warning: MAX columns exceeded (26)\n";
  $columns = 26;
}
if( $lines > 100 ){
  print STDERR "warning: MAX lines exceeded (100)\n";
  $lines = 100;
}

print FILE "$title\n";
print FILE "$lines,$columns\n";
$first = 1;
for( $dim = 0 ; $dim < $columns ; $dim++ ){
  $_ = $fieldLength[ $dim ];
  if( $_ > 20 ){
    $_ = 20;
  } elsif( 4 > $_ ){
    $_ = 4;
  } elsif( 20 >= $_ && $_ >= 4 && 0 == $_ % 2 ){
  } else {
    $_--;
  }
  if( 1 == $first ){
    $first = 0;
# C0: 3����ڤꤷ�ʤ�
# C1: 3����ڤꤹ��
# W?: ��� (4,6,8,10,12,14,16,18,20)
# R0: �������ʲ���ɽ��
# R2: ��������̤ޤ�ɽ��
# RF: ������������ʤ�
    print FILE "C0W", $_, "R2";
  } else {
    print FILE ",C0W", $_, "R2";
  }
}
print FILE "\n";

open( CSV, "$EUCconv $file |" ) || die "cannot open $file\n";

$lines = 0;
$cells = 0;
$flagWarning = 0;

while( <CSV> ){
  $lines++;
  last if $lines > 100;
  chop;
  @fields = &csvSplit( $_ );
  for( $dim = 0 ; $dim < $columns ; $dim++ ){
    if( '' ne $fields[ $dim ] ){
      $cells++;
      if( 0 == $flagWarning && $cells > 570 ){
	$flagWarning = 1;
	print STDERR "warning: MAX cells exceeded (570) in $lines lines\n";
      }
    }
    $fields[ $dim ] = &TruncateStr( $fields[ $dim ], 24 );
    if( '' eq $fields[ $dim ]
       || $fields[ $dim ] =~ /^=.*$/
       || $fields[ $dim ] =~ /^[.0-9]+$/ ){
    } else {
      $fields[ $dim ] =~ s/"/""/g;
      $fields[ $dim ] = '"' . $fields[ $dim ] . '"';
    }
    if( 0 == $dim ){
      print FILE $fields[ $dim ];
    } else {
      print FILE ",", $fields[ $dim ];
    }
  }
  print FILE "\n";
}

close( CSV );

close( FILE );

exit 0;

sub csvSplit {
  local( $arg ) = @_;

# ���������ܸ�� euc-jp ���Ϥ��Ʋ�����. shift_jis ���Բ�.

  @res = ( );
  $str = '';
  $quoted = 0;

  for( $ptr = 0 ; $ptr < length( $arg ) ; $ptr++ ){
    $ch = substr( $arg, $ptr, 1 );
    if( '"' eq $ch ){
      # ������
      if( '"' eq substr( $arg, $ptr + 1, 1 ) ){
	# '"' ���켫�Ȥ�ɽ��
	$str .= '""';
	$ptr++;
      } elsif( 0 == $quoted ){
	# quoted field �γ���
	$quoted = 1;
#	$str .= '"';
      } else {
	# quoted field �ν�λ
	$quoted = 0;
#	$str .= '"';
      }
    } elsif( 0 == $quoted && ',' eq $ch ){
      # �����
      push( @res, $str );
      $str = '';
    } else {
      $str .= $ch;
    }
  }

  push( @res, $str );

  return @res;
}

sub TruncateStr {
  local( $str, $len ) = @_;

# ���������ܸ�� euc-jp ���Ϥ��Ʋ�����. shift_jis ���Բ�.

  $res = '';
  $ptr = 0;

  while( $len > 0 ){
    $ch = substr( $str, $ptr, 1 );
    if( unpack( "C", $ch ) & 0x80 ){
      $res .= substr( $str, $ptr, 2 );
      $ptr += 2;
    } else {
      $res .= $ch;
      $ptr++;
    }
    $len--;
  }

  return $res;
}
