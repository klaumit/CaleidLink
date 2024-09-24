#
# Makefile for pvuploader (Linux version)
#

CC	= gcc
LD	= gcc

# Optimization: -O<n> or debugging: -g
OPT	= -O2

# uncomment this if you want a debug log file (debug.log)
#DBG	= -DWITH_DEBUG_FILE

LDLIBS	=

CFLAGS	= -Wall -pedantic $(OPT) $(DBG)
LDFLAGS	=

OBJS	=\
	pvuploader.o serialwork.o

PROGS =\
	pvuploader

INSTALL_DIR = $(HOME)/bin

all:		$(PROGS)

install:	all
		[ -d $(INSTALL_DIR) ] || mkdir $(INSTALL_DIR)
		install -m 755 ./pvuploader $(INSTALL_DIR)/pvuploader

clean:
		rm -f $(OBJS)
		rm -f $(PROGS)

pvuploader:	$(OBJS)
		@rm -f $@
		$(LD) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
		@chmod 750 $@

.c.o:
		$(CC) $(CFLAGS) -c $<

pvuploader.o:	receiver_hex.h serialwork.h pvuploader.h
serialwork.o:	pvuploader.h serialwork.h
