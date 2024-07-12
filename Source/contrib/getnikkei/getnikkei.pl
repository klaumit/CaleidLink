#!/usr/bin/perl

# getnikkei.pl
# All rights reserved. Copyright (C) 1998,2000 by NARITA Tomio.
# $Id: getnikkei.pl,v 1.1 2000/06/15 05:04:41 nrt Exp $

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

$tmp = '/tmp';
$stub = 'stub.html';
$cont = 'cont.html';

$depot = "$ENV{HOME}/caleid/data/";
#$depot = "./nikkei/";

$genre{ 'IT' } = 'http://it.nikkei.co.jp/it/';
$genre{ 'IT-net' } = 'http://it.nikkei.co.jp/it/net/';
$genre{ 'IT-tel' } = 'http://it.nikkei.co.jp/it/tel/';
$genre{ 'IT-har' } = 'http://it.nikkei.co.jp/it/har/';
$genre{ 'IT-sis' } = 'http://it.nikkei.co.jp/it/sis/';

# followings are not accepted so far.
#
#$genre{ 'main news' } = 'http://www.nikkei.co.jp/news/main/';
#$genre{ 'politics/economics' } = 'http://www.nikkei.co.jp/news/seikei/';
#$genre{ 'international' } = 'http://www.nikkei.co.jp/news/kaigai/';
#$genre{ 'market' } = 'http://www.nikkei.co.jp/news/market/';
#$genre{ 'industry' } = 'http://www.nikkei.co.jp/news/sangyo/';
#$genre{ 'VB' } = 'http://www.nikkei.co.jp/news/tento/';
#$genre{ 'socail' } = 'http://www.nikkei.co.jp/news/shakai/';
#$genre{ 'sports' } = 'http://www.nikkei.co.jp/news/undo/';
#$genre{ 'local' } = 'http://www.nikkei.co.jp/news/retto/';
#$genre{ 'editorial' } = 'http://www.nikkei.co.jp/news/shasetsu/';

foreach $key ( sort( keys( %genre ) ) ){
  if( -e "$tmp/$stub" ){
    `rm $tmp/$stub`;
  }

  $url = $genre{ $key };
  `wget $url -O $tmp/$stub 2> /dev/null`;

  open( STUB, "< $tmp/$stub" ) || die "cannot open $tmp/$stub\n";

  while( <STUB> ){
    chop;
    if( /A HREF="([^.]+)\.cfm\?id=([^"]+)">/ ){
      $name = $2;
      $exists{ $name } = 1;
      if( -e $depot . $name ){
	print "$name already exist\n";
      } else {
	&get_article( "${url}$1.cfm?id=", $name );
      }
    }
  }

  close( STUB );
  `rm $tmp/$stub`;
}

@files = `ls $depot`;

for( @files ){
  chop;
  if( !$exists{ $_ } ){
    print "rm $_\n";
    `rm $depot/$_`;
  }
}

exit( 0 );

sub get_article {
  local( $cgi, $name ) = @_;
  local( $url );

  $url = $cgi . $name;

  print "$url\n";

  if( -e "$tmp/$cont" ){
    `rm $tmp/$cont`;
  }

  `wget $url -O $tmp/$cont 2> /dev/null`;

  open( FILE, "< $tmp/$cont" ) || die "cannot open $tmp/$cont\n";

  $title_search = 1;
  $title_start = 0;
  $article_search = 0;
  $article_start = 0;
  while( <FILE> ){
    chop;
    if( $title_search ){
      if( /\/Ad full size/ ){
	$title_search = 0;
	$title_start = 1;
      }
    } elsif( $title_start ){
      if( /<B>(.+)<\/B>/ ){
	$title_start = 0;
	$article_search = 1;
	$title = $1;
	$content = '';
      }
    } elsif( $article_search ){
      $article_search = 0;
      $article_start = 1;
    } else {
      if( /^<table/ || /^<\/td><\/tr>/ ){
	$article = $depot . $name;
	open( ARTICLE, "> $article" ) || die "cannot open $article\n";
	print ARTICLE "$title\n";
	print ARTICLE "$content\n";
	close( ARTICLE );
	return;
      } else {
        s/<p>//;
        s/<A .+>//;
        s/<\/A>//;
	if( $article_start ){
	  $article_start = 0;
	  $content = $_;
	} else {
	  $content = $content . "\n" . $_;
	}
      }
    }
  }

  close( FILE );
  `rm $tmp/$cont`;
}
