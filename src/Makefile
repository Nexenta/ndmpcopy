###############################################################################
#
# Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright 2015 Nexenta Systems, Inc.  All rights reserved.
#

# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
###############################################################################

 
SHELL = /bin/sh
 
CC = gcc
CP = cp
CHMOD = chmod
CFLAGS = -g -w 
LIBDIR = ../lib
INCLUDES = -I$(LIBDIR)
 
LD = $(CC)
LDFLAGS = 
# These should work for solaris 2.5
LIBS = -L$(LIBDIR) -lndmp -lsocket -lnsl
# These should work for linux
#LIBS = -L$(LIBDIR) -lndmp

 
TARGET = ndmpcopy
TARGETS_H = dump.h main.h ndmpc.h notify.h queue.h
SOURCES = main.c dump.c queue.c file_history.c handler_tbl.c log.c notify.c
OBJS    = $(SOURCES:.c=.o)
 
all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o $(TARGET)
 

