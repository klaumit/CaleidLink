########################################################################
#
# Makefile for caleidlink
#

VERSION = 211

########################################################################
#
# Configurations
#

distdir = caleidlink-$(VERSION)
bindir = /usr/local/bin
x11dir = /usr/X11R6/bin
xpmdir = /usr/X11R6/include/X11/pixmaps

########################################################################
#
# Compiler flags
#

# Flash size and Synchronization mode
#   ACTIVE_SYNC means host-initiated synchronization
#   PASSIVE_SYNC means pda-initiated synchronization with cradle

SYNC_MODE = ACTIVE_SYNC
#SYNC_MODE = PASSIVE_SYNC

# Flash size (MB)
#	4 = XM750, PV450
#	2 = XM500EM, XM500, XM700, PV750, PV250
#	1 = XM400, RX50, EM10, PV170
#FLASH_SIZE = 4
#FLASH_SIZE = 2
FLASH_SIZE = 1

# PDA's internal coding system; see ctable_t.h
ENCODING = -DINTERNAL_ENCODING=SHIFT_JIS
#ENCODING = -DINTERNAL_ENCODING=BIG_FIVE

CALEID = -DFLASH_SIZE=$(FLASH_SIZE) -D$(SYNC_MODE)

# UNIX
CC = gcc
CFLAGS = -O2 -g -I. -I/usr/X11R6/include -funsigned-char -Wall $(CALEID)
LIBS =
MOTIFLIB = -L/usr/X11R6/lib -lXm -lXp -lXpm -lXt -lXext -lX11

# MSDOS
#CC = lcc
#CFLAGS = -I. -DMSDOS -cu $(CALEID)
#DOSLIB = -k-s1000 -m

########################################################################
#
# You don't have to change followings.
#

########################################################################
#
# Targets
#
# default, all, install (as root), dist, clean, adiup.exe

default: caleidlink xmclink
all: caleidlink pvlink cpvlink xmclink

########################################################################
#
# Distribution file
#

dist:
	if [ -f $(distdir).tar.gz ]; then rm $(distdir).tar.gz ; fi
	cd .. ; \
	tar cvf $(distdir)/$(distdir).tar \
		$(distdir)/CVS \
		$(distdir)/README.ja.html \
		$(distdir)/README.en \
		$(distdir)/CHANGES.ja \
		$(distdir)/HTCLINK.ja \
		$(distdir)/Makefile \
		$(distdir)/conf/* \
		$(distdir)/xpm/* \
		$(distdir)/gif/* \
		$(distdir)/XMclink.ad \
		$(distdir)/*.pl \
		$(distdir)/*.h \
		$(distdir)/*.c \
		$(distdir)/contrib/*
	gzip $(distdir).tar

########################################################################
#
# Installation
#

install:
	if [ -f caleidlink ]; then \
		if [ -d $(bindir) ]; then \
			install -c caleidlink $(bindir) ; \
		else \
			echo $(bindir) not exist ; \
		fi ; \
	fi
	if [ -f pvlink ]; then \
		if [ -d $(bindir) ]; then \
			install -c pvlink $(bindir) ; \
		else \
			echo $(bindir) not exist ; \
		fi ; \
	fi
	if [ -f cpvlink ]; then \
		if [ -d $(bindir) ]; then \
			install -c cpvlink $(bindir) ; \
		else \
			echo $(bindir) not exist ; \
		fi ; \
	fi
	if [ -f gif2quickform.pl ]; then \
		if [ -d $(bindir) ]; then \
			install -c gif2quickform.pl $(bindir) ; \
		else \
			echo $(bindir) not exist ; \
		fi ; \
	fi
	if [ -f quickform2gif.pl ]; then \
		if [ -d $(bindir) ]; then \
			install -c quickform2gif.pl $(bindir) ; \
		else \
			echo $(bindir) not exist ; \
		fi ; \
	fi
	if [ -f xmclink ]; then \
		if [ -d $(x11dir) ]; then \
			install -c xmclink $(x11dir) ; \
		else \
			echo $(x11dir) not exist ; \
		fi ; \
		if [ -d xpm ]; then \
			if [ -n $(xpmdir)/clink.xpm ]; then \
				install -c -m 644 xpm/*.xpm $(xpmdir) ; \
			else \
				echo $(xpmdir) not exist ; \
			fi ; \
		fi ; \
	fi

########################################################################
#
# Top level objects
#

OBJS = show.o base64.o sync.o session.o util.o link.o serial.o \
	itable.o ctable.o encode.o decode.o \
	eucjapan.o shiftjis.o iso2jp.o iso2022.o escape.o guess.o

# Caleid link

caleidlink: caleidlink.o folder.o $(OBJS)
	$(CC) -o $@ caleidlink.o folder.o $(OBJS) $(ENCODING_OBJS) $(LIBS)

pvlink: pvlink.o pvfolder.o $(OBJS)
	$(CC) -o $@ pvlink.o pvfolder.o $(OBJS) $(LIBS)

cpvlink: pvlink.o cpvfolder.o $(OBJS)
	$(CC) -o $@ pvlink.o cpvfolder.o $(OBJS) $(LIBS)

caleidlink.o: caleidlink.c version.h protocol.h session.h sync.h folder.h show.h
	$(CC) -c -o $@ $(CFLAGS) $(ENCODING) caleidlink.c

pvlink.o: caleidlink.c version.h protocol.h session.h sync.h folder.h show.h
	$(CC) -c -o $@ $(CFLAGS) -DPVLINK caleidlink.c

# XMclink -- Motif wrapper

xmclink: xmclink.o $(OBJS) folder.o
	$(CC) -o $@ xmclink.o $(OBJS) folder.o $(MOTIFLIB) $(LIBS)

xmclink.o: xmclink.c session.h version.h
	$(CC) -c -o $@ $(CFLAGS) -DPIXMAP_DIRECTORY=\"$(xpmdir)\" xmclink.c

# Caleid snooper

csnoop: csnoop.o folder.o $(OBJS)
	$(CC) -o $@ csnoop.o folder.o $(OBJS) $(ENCODING_OBJS) $(LIBS)

csnoop.o: csnoop.c
	$(CC) -c -o $@ $(CFLAGS) csnoop.c

# ADI uploader

adiup: adiup.o pvfolder.o $(OBJS)
	$(CC) -o $@ adiup.o pvfolder.o $(OBJS) $(LIBS)

adiup.exe: util.obj mcd.obj dserial.obj link.obj session.obj adiup.obj
	$(CC) -o $@ util.obj mcd.obj dserial.obj link.obj session.obj adiup.obj $(DOSLIB) 

mcd.obj: mcd.h mcd.c
	$(CC) -c -o $@ $(CFLAGS) mcd.c

dserial.obj: serial.h dserial.c mcd.h
	$(CC) -c -o $@ $(CFLAGS) dserial.c

link.obj: link.h link.c serial.h
	$(CC) -c -o $@ $(CFLAGS) link.c

session.obj: session.h session.c protocol.h link.h util.h
	$(CC) -c -o $@ $(CFLAGS) session.c

util.obj: util.h util.c
	$(CC) -c -o $@ $(CFLAGS) util.c

adiup.obj: adiup.c protocol.h session.h
	$(CC) -c -o $@ $(CFLAGS) adiup.c

adiup.o: adiup.c protocol.h session.h
	$(CC) -c -o $@ $(CFLAGS) adiup.c

########################################################################
#
# Common objects
#

folder.o: folder.h folder.c protocol.h base64.h util.h
	$(CC) -c -o $@ $(CFLAGS) $(ENCODING) folder.c

pvfolder.o: folder.h folder.c protocol.h base64.h util.h
	$(CC) -c -o $@ $(CFLAGS) folder.c

cpvfolder.o: folder.h folder.c protocol.h base64.h util.h
	$(CC) -c -o $@ $(CFLAGS) -DINTERNAL_ENCODING=BIG_FIVE folder.c

show.o: show.h show.c folder.h protocol.h
	$(CC) -c -o $@ $(CFLAGS) show.c

sync.o: sync.h sync.c protocol.h session.h util.h
	$(CC) -c -o $@ $(CFLAGS) sync.c

session.o: session.h session.c protocol.h link.h util.h
	$(CC) -c -o $@ $(CFLAGS) session.c

link.o: link.h link.c serial.h
	$(CC) -c -o $@ $(CFLAGS) link.c

serial.o: serial.h serial.c
	$(CC) -c -o $@ $(CFLAGS) serial.c

base64.o: base64.h base64.c
	$(CC) -c -o $@ $(CFLAGS) base64.c

itable.o: itable.h itable.c
	$(CC) -c -o $@ $(CFLAGS) itable.c

ctable.o: ctable.h ctable.c
	$(CC) -c -o $@ $(CFLAGS) ctable.c

decode.o: decode.h decode.c iso2022.h shiftjis.h
	$(CC) -c -o $@ $(CFLAGS) decode.c

encode.o: encode.h encode.c iso2jp.h eucjapan.h shiftjis.h
	$(CC) -c -o $@ $(CFLAGS) encode.c

iso2022.o: iso2022.h iso2022.c escape.h
	$(CC) -c -o $@ $(CFLAGS) iso2022.c

escape.o: escape.h escape.c decode.h
	$(CC) -c -o $@ $(CFLAGS) escape.c

iso2jp.o: iso2jp.h iso2jp.c encode.h
	$(CC) -c -o $@ $(CFLAGS) iso2jp.c

eucjapan.o: eucjapan.h eucjapan.c encode.h
	$(CC) -c -o $@ $(CFLAGS) eucjapan.c

shiftjis.o: shiftjis.h shiftjis.c encode.h decode.h
	$(CC) -c -o $@ $(CFLAGS) shiftjis.c

guess.o: guess.h guess.c
	$(CC) -c -o $@ $(CFLAGS) $(ENCODING) guess.c

util.o: util.h util.c
	$(CC) -c -o $@ $(CFLAGS) util.c

########################################################################
#
# Cleaning
#

clean:
	for file in adiup caleidlink pvlink cpvlink xmclink adiup.o caleidlink.o xmclink.o caleidlink.o folder.o pvlink.o pvfolder.o cpvfolder.o $(OBJS) ; \
	do \
		if [ -f $$file ]; then rm $$file ; fi \
	done

########################################################################
