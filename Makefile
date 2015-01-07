#
# Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
#
# Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
# Copyright 2015 Nexenta Systems, Inc.  All rights reserved.
#

#
## Permission is hereby granted, free of charge, to any person obtaining
## a copy of this software and associated documentation files (the
## "Software"), to deal in the Software without restriction, including
## without limitation the rights to use, copy, modify, merge, publish,
## distribute, sublicense, and/or sell copies of the Software, and to
## permit persons to whom the Software is furnished to do so, subject to
## the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
## DEALINGS IN THE SOFTWARE.
##


DESTDIR ?= proto
INSTALL = /usr/sbin/install

all:
	cd lib && $(MAKE)
	cd src && $(MAKE)

install: all
	$(INSTALL) -u root -g bin -d $(DESTDIR)/usr/bin
	$(INSTALL) -u root -g bin -d $(DESTDIR)/usr/share/man/man1
	$(INSTALL) -u root -g bin -m 555 -f $(DESTDIR)/usr/bin src/ndmpcopy
	$(INSTALL) -u root -g bin -m 644 -f $(DESTDIR)/usr/share/man/man1 ndmpcopy.1

clean:
	cd src && $(MAKE) clean
	cd lib && $(MAKE) clean

FRC:

