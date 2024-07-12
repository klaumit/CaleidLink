#!/usr/bin/perl

########################################################################
#
# htclink.pl
#
# All rights reserved. Copyright (C) 1998,1999 by NARITA Tomio.
#
# $Id: htclink.pl,v 1.22 2001/03/12 09:07:35 nrt Exp $
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
# CGI �Ȥ��Ƽ¹Ԥ��� htclink.pl ���餢�ʤ��� caleidlink folder �˥�������
# ���뤿��ˤ�, htclink.pl �μ¹Ի��μ¥桼�� ID �����ʤ��Ǥʤ��
# �ʤ�ޤ���. �٤��������ϳ䰦���ޤ���, ������Ū��ã�����뤿��ˤ�
# www �����Ф� apache �����Ѥ���, ����, apache �� suEXEC ��ͭ����
# ���뤳�Ȥ��Ǥ�μ¤Ǥ�. ������, ���ʤ����̤���ˡ�ǲ��Ǥ������
# ����ǹ����ޤ���. ������, ���������ɬ����äƲ�����. CGI �Ȥ��Ƽ¹�
# ����� htclink.pl �ˤ����Ф� setuid �ӥåȤ�Ω�ƤƤϤ����ޤ���.
#
# �ޤ�, caleidlink ��¹Ԥ��뤿��ˤϥ��ꥢ��ǥХ����򥪡��ץ�Ǥ��ʤ����
# �����ʤ�����, ���Τ���ξ��ˤ�α�դ��Ƥ�������.
#
########################################################################

########################################################################
#
# htclink.pl ����ƤФ�륳�ޥ��/ɬ�פʥե�����
#
# caleidlink	caleidlink
# cgi-lib.pl	CGI �ѤΥ桼�ƥ���ƥ��ؿ�
#		(htclink.pl ��Ʊ�� directory ���֤��Ʋ�����).
# cat		�ե��������Ȥ�������ޤ�.
# rm		�����å��ե����� GIF �θŤ���Τ�������.
# ps, egrep	caleidlink �μ¹Ծ��֤��ǧ���뤿��˼¹Ԥ��ޤ�.
# ppmtogif	portable anymap �桼�ƥ���ƥ� (netpbm �ѥå�����).
# ���ǥ���	���ʤ������ꤹ�륨�ǥ��� (���� $editor ����).
#
# �ޤ�, ���������֤˴ޤޤ�� conf �ǥ��쥯�ȥ��۲���إ��󥯥����٤�
# �ե�����إ��󥹥ȡ��뤹�뤳�Ȥ򤪴��ᤤ�����ޤ�.

########################################################################
#
# �������ѿ�������

$cgi = "htclink.pl";

# ��������Ǥ��ޤ� (���������֤ˤ�ޤ�Ƥ���ޤ�).
$cgiLib = "./cgi-lib.pl";

# ���ǥ���̾ (%s �ϥե�����̾)
$editor = "mule -display $ENV{'REMOTE_HOST'}:0.0 %s";

# �����ɷ��Ѵ��ġ��� (EUC �ǽ���)
$lv = "lv -Oe";

# �¥桼���� Home directory (�� ���Ф� setuid ���ʤ�����.)
( $user, $passwd, $uid, $gid, $quota, $comment, $gcos, $ENV{ 'HOME' }, $shell ) = getpwuid( $< );

# ���� CGI �� URL. �����, IP address ǧ�ڤ� Basic ǧ�����٤�
# �����������¤򤫤���褦�ˤ��Ʋ�����. �����������¤�����������
# ����Ƥ��뤫�ɤ�����ɬ����ǧ����褦�ˤ��Ʋ�����.
$binURL = "/~${user}/htclink-bin/";
$binDir = "$ENV{'HOME'}/public_html/htclink-bin/";
$cgiURL = "${binURL}${cgi}";

# ���᡼���ե����� (clnksync.gif, clnkstop.gif) ���֤� URL
$imgURL = "/~${user}/htclink-bin/";
$imgDir = "${binDir}";

# �����å��ե������ GIF �ե�����̾
$qform = "quickform";
$qformURL = "${imgURL}${qform}.$$.gif";
$qformFile = "${imgDir}${qform}.$$.gif";
$qformPattern = "${imgDir}${qform}.*.gif";

# �ե����̾ (����ե����� .clink �����ɤ߹��ޤ�ޤ�)
$folder = "$ENV{'HOME'}/caleid";

# �ǥХ���̾ (����ե����� .clink �����ɤ߹��ޤ�ޤ�)
$device = "/dev/cuaa0";

# ���ꥢ�륹�ԡ��� (����ե����� .clink �����ɤ߹��ޤ�ޤ�)
$speed = "38400";

# clink �Υ�����¸����ե�����
$logFile = "${binDir}clink.log";

# clink �δĶ��ե�����
$confFile = "$ENV{'HOME'}/.clink";

# �ѥͥ�̵���� clink ���¹���˲��̤��ե�å��夹��ֳ� (��)
$refreshInterval = "180";

## ��ž���륤�᡼�����ե�å��夹��ֳ� (�ߥ���)
#$imageRefreshInterval = "30000";

# �ѥͥ���ե�å��夹��ֳ֤ν���� (�ߥ���)
$panelRefreshInterval = "10000";

# CGI �˵����Ķ��ѿ�
$ENV{'PATH'} = "/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin";

# CGI �ǻ��Ѥ��� clink �����
$clink = "/usr/local/bin/caleidlink";

# ������ɽ���ǰ��٤��¤٤븫�Ф��κ����
$HEADING_MAX = 10;

# �طʿ������
$bodyAttr = 'BGCOLOR="#ffffe0" TEXT="#c00090" LINK="#0090c0" VLINK="#e000a8" ALINK="#00c090"';

$tableAttr = 'BORDER="1" CELLSPACING="2" CELLPADDING="4"';

########################################################################
#
# CGI �������ɤ߹���

require "$cgiLib";

&ReadParse( *in );

########################################################################

$error = '';
$message = '';

srand( $$ );

########################################################################
#
# htclink ����ν񤭹���

if( 'set' eq $in{ 'func' } ){
  open( CONF, "> $confFile" ) || &ErrorHTML( "cannot create $confFile" );

  $folder = $in{ 'folder' };
  $device = $in{ 'device' };
  $speed = $in{ 'speed' };

  print CONF "folder: $folder\n";
  print CONF "device: $device\n";
  print CONF "speed: $speed\n";

  close( CONF );

  $message .= "����ե�����˽񤭹��ߤޤ���.<BR>";
}

########################################################################
#
# htclink ������ɤ߹���

if( -f $confFile ){
  open( CONF, "< $confFile" ) || &ErrorHTML( "cannot open $confFile" );

  while( <CONF> ){
    chop;
    next if /^#/;
    if( /folder:/ ){
      s/folder: *//;
      $folder = $_;
    } elsif( /device:/ ){
      s/device: *//;
      $device = $_;
    } elsif( /speed:/ ){
      s/speed: *//;
      $speed = $_;
    }
  }

  close( CONF );
}

########################################################################
#
# ���ޥ�ɥ饤�����

for( @ARGV ){
  if( /^-f/ ){
    s/^-f//;
    $folder = $_;
  } elsif( /^-d/ ){
    s/^-d//;
    $device = $_;
  } elsif( /^-s/ ){
    s/^-s//;
    $speed = $_;
  }
}

########################################################################
#
# ��å��ե�����ϥǥХ������Ȥ�¸�ߤ���.

$dev = $device;
$dev =~ s/\//\./g;
$pidFile = "/tmp/clink${dev}";

########################################################################
#
# tainted ���ѿ��������

if( $tainted = $in{ 'directory' } ){
  if( $tainted =~ /(^[\/a-zA-Z0-9]+$)/ ){
    $directory = $1;
  } else {
    &ErrorHTML( "Invalid Directory" );
  }
}

$category = $in{ 'category' };
$categoryURL = &URLEncode( $category );

if( $tainted = $in{ 'categoryid' } ){
  if( $tainted =~ /(^[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]$)/ ){
    $categoryid = $1;
  } else {
    &ErrorHTML( "Invalid Category ID" );
  }
}

if( $tainted = $in{ 'object' } ){
  if( $tainted =~ /(^[\.\/a-zA-Z0-9]+$)/ ){
    $object = $1;
  } else {
    &ErrorHTML( "Invalid Object Name" );
  }
}

if( $tainted = $in{ 'objectid' } ){
  if( $tainted =~ /(^[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]$)/ ){
    $objectid = $1;
  } else {
    &ErrorHTML( "Invalid Object Name" );
  }
}

if( $tainted = $in{ 'file' } ){
  if( $tainted =~ /(^[-\/.a-zA-Z0-9]+$)/ ){
    $file = $1;
  } else {
    &ErrorHTML( "Invalid File Name" );
  }
}

$panel = $in{ 'panel' };
if( '' eq $panel ){
  $panel = 'false';
}

########################################################################
#
# caleidlink ��ư���ǧ

&ProbeClink;

if( '' eq $in{ 'func' } ){
  $in{ 'func' } = 'categories';
}

########################################################################
#
# ���󥸥�����������ɽ��

#if( 'image' eq $in{ 'func' } ){
#  if( $clinkPid ){
#    $gif = "${imgURL}clnksync.gif";
#  } else {
#    $gif = "${imgURL}clnkstop.gif";
#  }
#
#  print "Pragma: no-cache\n";
#  print "Location: http://" . $ENV{ 'REMOTE_HOST' } . "$gif\n\n";
#
#  exit 0;
#}

########################################################################
#
# JavaScript

sub Script {
  local( $script ) = @_;

  print "<SCRIPT LANGUAGE=\"JavaScript\"><!--\n";
  print "@_\n";
  print "// -->\n";
  print "</SCRIPT>\n";
}

sub JavaScript {
  print "<SCRIPT LANGUAGE=\"JavaScript\"><!--\n";

# ���󥸥�������������ɥ���ɽ��

  print "function OpenIndicator() {\n";
  print "  str = new String( window.location );\n";
  print "  if( 0 <= (idx = str.indexOf( 'panel=true' )) ){\n";
  print "  } else if( 0 <= (idx = str.indexOf( 'panel=false' )) ){\n";
  print "    window.location = str.substr( 0, idx ) + 'panel=true' + str.substring( idx + 11, str.length );\n";
  print "  } else if( 0 <= str.indexOf( '?' ) ){\n";
  print "    window.location = str + '&panel=true';\n";
  print "  } else {\n";
  print "    window.location = str + '?panel=true';\n";
  print "  }\n";
  print "  win = window.open( \"${cgiURL}?func=indicator&panel=true\", \"indicator\", \"width=480,height=128,resizable=yes\" );\n";
  print "  win.focus();\n";
  print "}\n";

  print "function ReloadIndicator() {\n";
  print "  win = window.open( \"${cgiURL}?func=indicator&panel=true\", \"indicator\", \"width=480,height=128,resizable=yes\" );\n";
  print "  win.focus();\n";
  print "}\n";

# ���󥸥�������������ɥ��Υ�����

  print "function CloseIndicator() {\n";
  print "  str = new String( window.opener.location );\n";
  print "  if( 0 <= (idx = str.indexOf( 'panel=false' )) ){\n";
  print "  } else if( 0 <= (idx = str.indexOf( 'panel=true' )) ){\n";
  print "    window.opener.location = str.substr( 0, idx ) + 'panel=false' + str.substring( idx + 10, str.length );\n";
  print "  } else if( 0 <= str.indexOf( '?' ) ){\n";
  print "    window.opener.location = str + '&panel=false';\n";
  print "  } else {\n";
  print "    window.opener.location = str + '?panel=false';\n";
  print "  }\n";
  print "  self.close();\n";
  print "}\n";

# ����������ɥ���ɽ��

  print "function OpenLog() {\n";
  print "  win = window.open( \"${cgiURL}?func=logpanel\", \"log\", \"scrollbars=yes,width=640,height=320,resizable=yes\" );\n";
  print "  win.focus();\n";
  print "}\n";

# ��������ɥ��� URL ���ѹ�

  print "function OpenURL( url ) {\n";
  print "  window.opener.location = url + '&panel=true';\n";
  print "}\n";

# ��������ɥ��� URL ���ѹ�

  print "function GoURL( url ) {\n";
  print "  window.location = url;\n";
  print "}\n";

# ��ž���륤�᡼���Υ����

#  print "function ReloadRoll() {\n";
#  print "  var str = new String( document.roll.src );\n";
#  print "  document.roll.src = str.substring( 0, str.lastIndexOf( '&' ) ) + '&dummy=' + Math.random();\n";
#  print "  setTimeout( \"ReloadRoll()\", $imageRefreshInterval );\n";
#  print "}\n";

  print "function ReloadSelfWithBackoff() {\n";
  print "  str = new String( location );\n";
  print "  if( 0 <= (idx = str.indexOf( 'backoff=' )) ){\n";
  print "    backoff = 2 * parseInt( str.substring( idx + 8, str.length ), 10 );\n";
  print "    setTimeout( \"location = '\" + str.substring( 0, idx ) + \"backoff=\" + backoff + \"'\", backoff )\n";
  print "  } else {\n";
  print "    backoff = $panelRefreshInterval;\n";
  print "    setTimeout( \"location = '\" + str + \"&backoff=\" + backoff + \"'\", backoff )\n";
  print "  }\n";
  print "}\n";

  print "// -->\n";
  print "</SCRIPT>\n";
}

########################################################################
#
# ���󥸥�����������

sub DrawIndicator {
  print "<FORM>\n";

  print "<IMG SRC=\"/icons/folder.open.gif\" ALIGN=\"top\" ALT=\"\"><FONT SIZE=\"+2\"> Folder: $folder </FONT><BR>\n";

  if( $clinkPid ){
    $gif = "${imgURL}clnksync.gif";
    $alt = "caleidlink is running.";
    $anchor = "${cgiURL}?func=stop";
  } else {
    $gif = "${imgURL}clnkstop.gif";
    $alt = "caleidlink is NOT running.";
    $anchor = "${cgiURL}?func=start";
  }

  print "<TABLE ALIGN=\"left\"><TR><TD></TD></TR><TR><TD>\n";
  print "<TABLE BGCOLOR=\"#808080\"><TR><TD>";
  print "<TABLE BORDER=\"2\"><TR><TD>";

  if( 'true' ne $panel ){
    print "<A HREF=\"$anchor\">";
  }
#  $dummy = rand();
#  if( $clinkPid ){
#    print "<IMG NAME=\"roll\" SRC=\"${cgiURL}?func=image&dummy=$dummy\" ALT=\"$alt\" BORDER=\"0\">";
#  } else {
    print "<IMG SRC=\"$gif\" ALT=\"$alt\" BORDER=\"0\">";
#  }
  if( 'true' ne $panel ){
    print "</A>";
  }
  print "</TD></TR></TABLE>";
  print "</TD></TR></TABLE>\n";
  print "</TD></TR></TABLE>\n";

#  if( $clinkPid ){
#    &Script( "ReloadRoll();" );
#  }

  if( 'true' eq $panel ){
    $openMethod = "OpenURL";
  } else {
    $openMethod = "GoURL";
  }

  print "<TABLE CELLSPACING=\"2\" CELLPADDING=\"4\"><TR>";

  if( 'true' eq $panel ){
    print "<TD><IMG SRC=\"/icons/index.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Close\" onClick=\"CloseIndicator()\"></TD>\n";
  } else {
    print "<TD><IMG SRC=\"/icons/index.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Panel\" onClick=\"OpenIndicator()\"></TD>\n";
  }
  print "<TD><IMG SRC=\"/icons/folder.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Folder\" onClick=\"${openMethod}( '${cgiURL}?func=categories' )\"></A></TD>\n";
  print "<TD><IMG SRC=\"/icons/screw1.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Config\" onClick=\"${openMethod}( '${cgiURL}?func=config' )\"></A></TD>\n";
  if( 'true' eq $panel ){
    print "<TD><IMG SRC=\"/icons/alert.black.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Log\" onClick=\"OpenLog()\"></TD>\n";
  } else {
    print "<TD><IMG SRC=\"/icons/alert.black.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Log\" onClick=\"${openMethod}( '${cgiURL}?func=log' )\"></TD>\n";
  }

  print "</TR><TR>";

  print "<TD><IMG SRC=\"/icons/hand.right.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Start\" onClick=\"${openMethod}( '${cgiURL}?func=start' )\"></TD>\n";
  print "<TD><IMG SRC=\"/icons/sphere1.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Backup\" onClick=\"${openMethod}( '${cgiURL}?func=backup' )\"></TD>\n";
  print "<TD><IMG SRC=\"/icons/link.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Restore\" onClick=\"${openMethod}( '${cgiURL}?func=restore' )\"></TD>\n";
  print "<TD><IMG SRC=\"/icons/burst.gif\" ALIGN=\"bottom\" BORDER=\"0\"><INPUT TYPE=\"button\" VALUE=\"Stop\" onClick=\"${openMethod}( '${cgiURL}?func=stop' )\"></TD>\n";

  print "</TR></TABLE>";

  print "</FORM>\n";

  print "<BR CLEAR=\"all\">";
}

########################################################################
#
# ���󥸥��������ѥͥ��ɽ��

if( 'indicator' eq $in{ 'func' } ){
  print "Content-type: text/html\n\n";
  print "<HTML><HEAD><TITLE>htclink Indicator</TITLE>\n";

  &JavaScript;

  print "</HEAD>\n";

  if( $clinkPid ){
    print "<BODY $bodyAttr onLoad=\"ReloadSelfWithBackoff()\">\n";
  } else {
    print "<BODY $bodyAttr>\n";
  }

  &DrawIndicator;

  print "</BODY></HTML>\n";
  exit 0;
}

########################################################################
#
# ����ɽ��

sub DisplayLog {
  print "<FONT SIZE=\"+1\">Log</FONT>\n";

  print "<PRE>";

  open( LOGFILE, "< $logFile" ) || &Error( "cannot open $logFile" );

  while( <LOGFILE> ){
    chop;
    s/</&lt;/;
    s/>/&gt;/;
    print $_, "\n";
  }

  close( LOGFILE );

  print "</PRE>";
}

########################################################################
#
# �����ѥͥ��ɽ��

if( 'logpanel' eq $in{ 'func' } ){
  print "Content-type: text/html\n\n";
  print "<HTML><HEAD><TITLE>htclink Log</TITLE>\n";
  print "</HEAD>\n";
  print "<BODY $bodyAttr>\n";

  &Script( 'window.opener.location.reload();' );

  &DisplayLog;

  print "<HR><P ALIGN=\"right\"><A HREF=\"${cgiURL}?func=logpanel\">Reload...</A></P>\n";

  print "<CENTER><FORM>\n";
  print "<FONT COLOR=#000000><INPUT TYPE=\"button\" NAME=\"dismiss\" VALUE=\"Dismiss\" onClick=\"self.close()\"></FONT>\n";
  print "</FORM></CENTER>\n";

  print "</BODY></HTML>\n";
  exit 0;
}

########################################################################
#
# HTML �إå�

print "Content-type: text/html\n\n";

print "<HTML><HEAD><TITLE>htclink</TITLE>\n";
if( $clinkPid && 'false' eq $panel ){
  print "<META http-equiv=\"refresh\" content=\"${refreshInterval}; url=${cgiURL}?panel=$panel'\">";
} elsif( 'edit' eq $in{ 'func' } ){
  print "<META http-equiv=\"refresh\" content=\"3; url=${cgiURL}?func=object&categoryid=$categoryid&directory=$directory&category=$categoryURL&object=$object&panel=$panel\">";
} elsif( 'start' eq $in{ 'func' }
	|| 'backupstart' eq $in{ 'func' }
	|| 'restorestart' eq $in{ 'func' }
	|| 'stop' eq $in{ 'func' } ){
  print "<META http-equiv=\"refresh\" content=\"10; url=${cgiURL}?panel=$panel'\">";
}

&JavaScript;

print "</HEAD>\n";

########################################################################
#
# HTML �ܥǥ�

print "<BODY $bodyAttr>\n";

$expecting = 0;

if( 'start' eq $in{ 'func' } || 'backupstart' eq $in{ 'func' } || 'restorestart' eq $in{ 'func' } ){
  if( $clinkPid ){
    $error = "���Ǥ�caleidlink��ư���Ƥ��ޤ�.<BR>";
    if( 'true' eq $panel ){
      &Script( 'ReloadIndicator();' );
    }
  } else {
    $mode = '';
    if( 'start' eq $in{ 'func' } ){
      if( -d $folder ){
	$mode = "-S ${folder}";
      } else {
	$error .= "�ե������¸�ߤ��ޤ��� ($folder).<BR>";
      }
    } elsif( 'backupstart' eq $in{ 'func' } ){
      if( -f $file ){
	$error .= "Ʊ��̾���Υե����뤬¸�ߤ��ޤ� ($file).<BR>";
      } else {
	$mode = "-B ${file}";
      }
    } elsif( 'restorestart' eq $in{ 'func' } ){
      if( -f $file ){
	$mode = "-R ${file}";
      } else {
	$error .= "�Хå����å׺ѤߤΥե����뤬¸�ߤ��ޤ��� ($file).<BR>";
      }
    }

    if( '' ne $mode ){
      system( "caleidlink -d${device} -s${speed} -f $mode > ${logFile} 2>&1 &" );
      sleep 1;
      $expecting = 1;
      &ProbeClink;
    } else {
      if( 'true' eq $panel ){
	&Script( 'ReloadIndicator();' );
      }
    }
  }
} elsif( 'stop' eq $in{ 'func' } ){
  if( $clinkPid ){
    kill 'TERM', $clinkPid;
    sleep 1;
    $expecting = -1;
    &ProbeClink;
  } else {
    $error .= "caleidlink��ư���Ƥ��ޤ���.<BR>";
    if( 'true' eq $panel ){
      &Script( 'ReloadIndicator();' );
    }
  }
}

if( 'confirm' eq $in{ 'func' } ){
  if( ! -f $file ){
    $error .= "�Хå����å׺ѤߤΥե����뤬¸�ߤ��ޤ��� ($file).<BR>";
  }
}

if( 1 == $expecting ){
  if( 0 == $clinkPid ){
    $error .= "��ư�˼��Ԥ��ޤ���.<BR>";
  } else {
    $message .= "��ư���ޤ���.<BR>";
  }
  if( 'true' eq $panel ){
    &Script( 'ReloadIndicator();' );
  }
} elsif( -1 == $expecting ){
  if( 0 != $clinkPid ){
    $error .= "�������ߤǤ��ޤ���Ǥ���.<BR>";
  } else {
    $message .= "��ߤ��ޤ���.<BR>";
  }
  if( 'true' eq $panel ){
    &Script( 'ReloadIndicator();' );
  }
}

( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst ) = localtime;

$year = 1900 + $year;
$month = 1 + $mon;
$day = $mday;

$date = sprintf( "%02d/%02d/%02d %02d:%02d:%02d", $year, $month, $day, $hour, $min, $sec );

print "<P ALIGN=\"right\">$date<HR></P>\n";

if( 'true' ne $panel ){
  &DrawIndicator;
  print "<HR>";
}

if( '' ne $error ){
  print "<FONT COLOR=#FF0000>���顼: $error</FONT><HR>";
}

if( '' ne $message ){
  print "<FONT COLOR=#0000FF>$message</FONT><HR>";
}

########################################################################
#
# ����

if( 'config' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Config</FONT>\n";

  print "<FORM METHOD=\"post\" ACTION=\"${cgiURL}\">";
  print "<INPUT TYPE=\"hidden\" NAME=\"func\" VALUE=\"set\">\n";
  print "<INPUT TYPE=\"hidden\" NAME=\"panel\" VALUE=\"$panel\">\n";
  print "<TABLE $tableAttr>";
  print "<TR><TD>Folder</TD><TD><INPUT TYPE=\"text\" NAME=\"folder\" SIZE=\"40\" VALUE=\"$folder\"></TR>\n";
  print "<TR><TD>Device</TD><TD><INPUT TYPE=\"text\" NAME=\"device\" SIZE=\"40\" VALUE=\"$device\"></TR>\n";
  print "<TR><TD>Speed</TD><TD><SELECT NAME=\"speed\">";
  if( 4800 == $speed ){
    $s4800 = "SELECTED";
  } elsif( 9600 == $speed ){
    $s9600 = "SELECTED";
  } elsif( 19200 == $speed ){
    $s19200 = "SELECTED";
  } else {
    $s38400 = "SELECTED";
  }
  print "<OPTION value=\"4800\" $s4800>4800bps\n";
  print "<OPTION value=\"9600\" $s9600>9600bps\n";
  print "<OPTION value=\"19200\" $s19200>19200bps\n";
  print "<OPTION value=\"38400\" $s38400>38400bps\n";
  print "</SELECT></TD></TR>\n";
  print "<TR><TD ALIGN=\"center\" COLSPAN=\"2\"><INPUT TYPE=\"submit\" VALUE=\"OK\"></TD></TR>";
  print "</TABLE>";
  print "</FORM>\n";
}

########################################################################
#
# �Хå����å�

if( 'backup' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Backup</FONT>\n";

  $file = sprintf( "$folder/backup.%04d%02d%02d", $year, $month, $day );

  print "<FORM METHOD=\"post\" ACTION=\"${cgiURL}\">";
  print "<INPUT TYPE=\"hidden\" NAME=\"func\" VALUE=\"backupstart\">\n";
  print "<INPUT TYPE=\"hidden\" NAME=\"panel\" VALUE=\"$panel\">\n";
  print "<TABLE>";
  print "<TR><TD>Backup File:</TD><TD><INPUT TYPE=\"text\" NAME=\"file\" SIZE=\"40\" VALUE=\"$file\"></TD></TR>\n";
  print "<TR><TD ALIGN=\"center\" COLSPAN=\"2\"><FONT COLOR=\"#FF0000\">Do you really want to BACKUP? (Caleid =&gt; PC) </FONT><INPUT TYPE=\"submit\" VALUE=\"OK\"></TD></TR>";
  print "</TABLE>";
  print "</FORM>\n";
}

########################################################################
#
# �ꥹ�ȥ�

if( 'restore' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Restore</FONT>\n";

  $file = "$folder/backup.";

  print "<FORM METHOD=\"post\" ACTION=\"${cgiURL}\">";
  print "<INPUT TYPE=\"hidden\" NAME=\"func\" VALUE=\"confirm\">\n";
  print "<INPUT TYPE=\"hidden\" NAME=\"panel\" VALUE=\"$panel\">\n";
  print "<TABLE>";
  print "<TR><TD>Restore File:</TD><TD><INPUT TYPE=\"text\" NAME=\"file\" SIZE=\"40\" VALUE=\"$file\"></TD></TR>\n";
  print "<TR><TD ALIGN=\"center\" COLSPAN=\"2\"><FONT COLOR=\"#FF0000\">Do you really want to RESTORE? (PC =&gt; Caleid) </FONT><INPUT TYPE=\"submit\" VALUE=\"OK\"></TD></TR>";
  print "</TABLE>";
  print "</FORM>\n";
}

########################################################################
#
# �ꥹ�ȥ��γ�ǧ

if( 'confirm' eq $in{ 'func' } && -f $file ){
  print "<FONT SIZE=\"+1\">Confirm to Restore</FONT>\n";

  print "<FORM METHOD=\"post\" ACTION=\"${cgiURL}\">";
  print "<INPUT TYPE=\"hidden\" NAME=\"func\" VALUE=\"restorestart\">\n";
  print "<INPUT TYPE=\"hidden\" NAME=\"panel\" VALUE=\"$panel\">\n";
  print "<INPUT TYPE=\"hidden\" NAME=\"file\" VALUE=\"${file}\">\n";
  print "<TABLE>";
  print "<TR><TD>Restore File:</TD><TD>";

  print "<TABLE BGCOLOR=\"#D3D3D3\"><TR><TD>";
  print "<TABLE $tableAttr BGCOLOR=\"#FFFFFF\"><TR><TD>";
  print "<FONT COLOR=\"#000000\">${file}</FONT>";
  print "</TD></TR></TABLE>";
  print "</TD></TR></TABLE>\n";

  print "</TD></TR>\n";
  print "<TR><TD ALIGN=\"center\" COLSPAN=\"2\"><FONT COLOR=\"#FF0000\">Your Caleid instance will be replaced with it! (PC =&gt; Caleid) </FONT><INPUT TYPE=\"submit\" VALUE=\"OK\"></TD></TR>";
  print "</TABLE>";
  print "</FORM>\n";
}

########################################################################
#
# categories ��ɽ��

if( 'categories' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Categories</FONT>\n";

  @categories = &LoadCategories;

  print "<UL>\n";

  for( @categories ){
    ( $id, $directory, $category ) = split( ',', $_, 3 );

    $categoryURL = &URLEncode( $category );

    print "<LI> <A HREF=\"${cgiURL}?func=catalog&categoryid=$id&directory=$directory&category=$categoryURL&panel=$panel\">$category</A>\n";
  }

  print "</UL>\n";
}

########################################################################
#
# catalog ��ɽ��

if( 'catalog' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Catalog: $category</FONT>\n";

  $order = 0;

  print "<P><TABLE BORDER=\"1\" CELLSPACING=\"2\" CELLPADDING=\"4\">\n";

  $file = "${folder}/conf/${directory}.heading";

  open( FILE, "$lv $file |" ) || &Error( "cannot open $file\n" );

  $headingCount = 1;
  print "<TR><TH>�ե�����</TD>";
  while( <FILE> ){
    chop;
    ( $dummyID, $heading, $dummyType ) = split( ' ', $_, 3 );
    if( $heading =~ /:$/ ){
      $heading =~ s/:$//;
    }
    if( '000001' eq $heading || '102001' eq $heading || '102000' eq $heading ){
      next;
    }
    $headingCount++;
    if( $headingCount > $HEADING_MAX ){
      last;
    }
    if( '000000' eq $heading ){
      $heading = "����";
    }
    if( '000010' eq $heading ){
      $heading = "�����ȥ�";
    }
    print "<TH>$heading</TH>";
  }
  print "</TR>\n";

  close( FILE );

  if( $headingCount < $HEADING_MAX ){
    $HEADING_MAX = $headingCount;
  }

  @catalog = &LoadCatalog( $order );

  for( @catalog ){
    @headingContents = split( "\x1f", $_, $headingCount );
    print "<TR>";
    $contentsCount = 0;
    for( @headingContents ){
      $contentsCount++;
      if( $contentsCount > $HEADING_MAX ){
	last;
      }
      if( 1 == $contentsCount ){
	print "<TD> <A HREF=\"${cgiURL}?func=object&categoryid=$categoryid&directory=$directory&category=$categoryURL&object=$_&panel=$panel\"> $_ </A> </TD>\n";
      } else {
	if( '' eq $_ ){
	  $_ = '-';
	}
	print "<TD> $_ </TD>\n";
      }
    }
    if( $contentsCount < $HEADING_MAX ){
      for( ; $contentsCount < $HEADING_MAX ; $contentsCount++ ){
	print "<TD> - </TD>\n";
      }
    }
    print "<TR>";
  }

  print "</TABLE></P>\n";

  close( CLINK );
}

########################################################################
#
# object ��ɽ��

if( 'object' eq $in{ 'func' } || 'edit' eq $in{ 'func' } ){
  print "<FONT SIZE=\"+1\">Object: $in{'object'} in $category </FONT>\n";

  %contents = &LoadObject;

  print "<P><A HREF=\"${cgiURL}?func=edit&categoryid=$categoryid&directory=$directory&category=$categoryURL&object=$object&panel=$panel\">";
  print "<FONT SIZE=\"+1\"><IMG SRC=\"/icons/quill.gif\" BORDER=\"0\">Edit</FONT></A></P>\n";

  print "<P><TABLE $tableAttr>\n";

  foreach $index ( sort ( keys %contents ) ){
    ( $tag, $heading ) = split( ' ', $index, 2 );
    $content = $contents{ $index };

    if( '__IMAGE__' eq $heading ){
      if( '' ne $qformPattern ){
	system( "rm -f ${qformPattern}" );
      }

      $preamble = substr( $content, 0, 62 );
      $content = &Bin2Ppm( substr( $content, 62, length( $content ) - 62 ) );

      $width = (unpack("C", substr( $preamble, 19, 1 )) << 8) | unpack("C", substr( $preamble, 18, 1 ));
      $height = (unpack("C", substr( $preamble, 23, 1 )) << 8) | unpack("C", substr( $preamble, 22, 1 ));

      $width_word = int( $width / 16 );
      if( 0 != $width % 16 ){
	$width_word++;
      }

      $width_word *= 16;

      open( TMP, "| ppmtogif -interlace -transparent rgb:FF/FF/FF > ${qformFile} 2> /dev/null" ) || &Error( "cannot open $qformFile\n" );

      print TMP "P5\n";
      print TMP "$width\n";
      print TMP "$height\n";
      print TMP "1\n";

      for( $li = 0 ; $li < $height ; $li++ ){
	print TMP substr( $content, length( $content ) - $width_word, $width );
	$content = substr( $content, 0, length( $content ) - $width_word );
      }

      close( TMP );

      print "<TR> <TH> Image </TH> <TD>";
      print "<TABLE BGCOLOR=\"#D3D3D3\" ALIGN=\"left\"><TR><TD>";
      print "<TABLE BORDER=\"2\"><TR><TD>";
      print "<A HREF=\"${qformURL}\">";
      print "<IMG SRC=\"${qformURL}\" ALT=\"Quick Form GIF\" BORDER=\"0\">";
      print "</A>";
      print "</TD></TR></TABLE>";
      print "</TD></TR></TABLE>\n";
    } else {
      print "<TR> <TH> $heading </TH> <TD> $content";
    }
  }
  print "</TD> </TR>\n";

  print "</TABLE></P>\n";
}

########################################################################
#
# ����ɽ��

if( 'log' eq $in{ 'func' }
   || 'start' eq $in{ 'func' }
   || 'backupstart' eq $in{ 'func' }
   || 'restorestart' eq $in{ 'func' } ){
  &DisplayLog;
  print "<HR><P ALIGN=\"right\"><A HREF=\"${cgiURL}?func=log&panel=$panel\">Reload...</A></P>\n";
}

########################################################################
#
# ���ǥ����ε�ư

if( 'edit' eq $in{ 'func' } ){
  $file = $folder . "/" . $directory . "/" . $object;
  if( $editor =~ /%s/ ){
    system( sprintf( "$editor > /dev/null &", $file ) );
  } else {
    system( "$editor $file > /dev/null &" );
  }
}

########################################################################
#
# �ʾ�

print "</BODY></HTML>\n";

exit 0;

########################################################################
#
# clink �ε�ư�����å�

sub ProbeClink {
  $clinkPid = 0;
  $validLog = 0;

  if( -f $pidFile ){
    chop( $taintedPid = scalar( `cat $pidFile` ) );
    if( $taintedPid =~ /(^[0-9]+$)/ ){
      $taintedPid = $1;
      @res = `ps -x | egrep caleidlink | egrep -v egrep`;
      for( @res ){
	if( $taintedPid == (split)[ 0 ] ){
	  $clinkPid = $taintedPid;
#	  if( -f $logFile ){
#	    @res = `fstat $logFile`;
#	    for( @res ){
#	      if( $clinkPid == (split)[ 2 ] ){
#		$validLog = 1;
#		last;
#	      }
#	    }
#	  }
	  last;
	}
      }
    }
  }
}

########################################################################
#
# ���顼ɽ��

sub Error {
  local( $msg ) = @_;

  print "<FONT COLOR=#FF0000> ���顼: $msg </FONT>\n";

  print "</BODY></HTML>\n";

  exit 1;
}

sub ErrorHTML {
  local( $msg ) = @_;

  print "Content-type: text/html\n\n";

  print "<HTML><HEAD><TITLE>Caleidlink Error</TITLE></HEAD>\n";
  print "<BODY $bodyAttr>\n";

  &Error( $msg );
}

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

sub Bin2Ppm {
  local( $bin ) = @_;

  $res = '';

  for( $ptr = 0 ; $ptr < length( $bin ) ; $ptr++ ){
    $ch = unpack( "C", substr( $bin, $ptr, 1 ) );
    $byte = $ch;
    for( $idx = 0 ; $idx < 8 ; $idx++ ){
      if( $byte & 0x80 ){
	$res .= "\x00";
      } else {
	$res .= "\x01";
      }
      $byte <<= 1;
    }
  }

  return $res;
}

########################################################################

sub URLEncode {
    my($url)=@_;
    my(@characters)=split(/(\%[0-9a-fA-F]{2})/,$url);

    foreach(@characters)
    {
        if ( /\%[0-9a-fA-F]{2}/ ) # Escaped character set ...
        {
            # IF it is in the range of 0x00-0x20 or 0x7f-0xff
            #    or it is one of  "<", ">", """, "#", "%",
            #                     ";", "/", "?", ":", "@", "=" or "&"
            # THEN preserve its encoding
            #"
            unless ( /(20|7f|[0189a-fA-F][0-9a-fA-F])/i
                    || /2[2356fF]|3[a-fA-F]|40/i )
            {
                s/\%([2-7][0-9a-fA-F])/sprintf "%c",hex($1)/e;
            }
        }
        else # Other stuff
        {
            # 0x00-0x20, 0x7f-0xff, <, >, and " ... "
            s/([\000-\040\177-\377\074\076\042])
             /sprintf "%%%02x",unpack("C",$1)/egx;
        }
    }
    return join("",@characters);
}

########################################################################

sub LoadCategories {
  open( CLINK, "$clink -ce $folder |" ) || &Error( "cannot exec $clink\n" );

  @categories = ( );
  
  while( <CLINK> ){
    chop;
    push( @categories, $_ );
  }

  close( CLINK );

  return @categories;
}

########################################################################

sub LoadCatalog {
  local( $order ) = @_;

  open( CLINK, "$clink -ce -t31 $folder $categoryid |" ) || &Error( "cannot exec $clink\n" );

  @catalog = ( );

  while( <CLINK> ){
    chop;
    if( 0 == $order ){
      unshift( @catalog, $_ );
    } else {
      push( @catalog, $_ );
    }
  }

  close( CLINK );

  return @catalog;
}

########################################################################

sub LoadObject {
  $file = "${folder}/${directory}/${object}";

  $tag = "AA";
  %contents = ( );

  open( FILE, "$lv $file | cat |" ) || &Error( "cannot open $file" );

  if( '000000' eq $categoryid || '00F000' eq $categoryid || '600000' eq $categoryid ){
    $index = $tag . " " . "����";

    while( <FILE> ){
      chop;
      $contents{ $index } .= "$_<BR>\n";
    }

    close( FILE );
    return %contents;
  }

  while( <FILE> ){
    chop;

    if( $_ =~ /^\t/ ){
      s/^\t//;
      $contents{ $index } .= "<BR>$_\n";
    } else {
      ( $id, $heading, $field ) = split( ' ', $_, 3 );
      if( $heading =~ /:$/ ){
	$heading =~ s/:$//;
      }
      if( '102001' eq $heading || '102000' eq $heading ){
	$index = $tag . " " . '__IMAGE__';
	#$contents{ $index } .= &Bin2Ppm( &DecodeBase64( $field ) );
	$contents{ $index } .= &DecodeBase64( $field );
	if( '102000' eq $heading ){
	  $tag++;
	}
      } else {
	$index = $tag . " " . $heading;
	$contents{ $index } .= $field;
	$tag++;
      }
    }
  }

  close( FILE );

  return %contents;
}

########################################################################

