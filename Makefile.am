#
#   /*------------------------------------------------------------------+
#    |                             Clay                                 |
#    |------------------------------------------------------------------|
#    |                          makefile.am                             |
#    |------------------------------------------------------------------|
#    |                   First version: 03/04/2012                      |
#    +------------------------------------------------------------------+
#
# +------------------------------------------------------------------------+
# |  / __)(  )    /__\ ( \/ )                                              |
# | ( (__  )(__  /(__)\ \  /        Chunky Loop Alteration wizardrY        |
# |  \___)(____)(__)(__)(__)                                               |
# +------------------------------------------------------------------------+
# | Copyright (C) 2012 University of Paris-Sud                             |
# |                                                                        |
# | This library is free software; you can redistribute it and/or modify   |
# | it under the terms of the GNU Lesser General Public License as         |
# | published by the Free Software Foundation; either version 2.1 of the   |
# | License, or (at your option) any later version.                        |
# |                                                                        |
# | This library is distributed in the hope that it will be useful but     |
# | WITHOUT ANY WARRANTY; without even the implied warranty of             |
# | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       |
# | Lesser General Public License for more details.                        |
# |                                                                        |
# | You should have received a copy of the GNU Lesser General Public       |
# | License along with this software; if not, write to the Free Software   |
# | Foundation, Inc., 51 Franklin Street, Fifth Floor,                     |
# | Boston, MA  02110-1301  USA                                            |
# |                                                                        |
# | Clay, the Chunky Loop Alteration wizardrY                              |
# | Written by Cedric Bastoul, Cedric.Bastoul@u-psud.fr                    |
# +------------------------------------------------------------------------*/
#
# Makefile.am (or makefile if generated) of Clay.
# Makefile.am is not a makefile, you must run the 'autogen.sh' THEN the
# configure shellscript to generate the Makefile thanks to this file.
#
#############################################################################

if BUNDLED_OSL
    MAYBE_OSL = osl
    OSL_LA = $(top_builddir)/osl/source/libosl.la
endif

SUBDIRS         = $(MAYBE_OSL) doc
DIST_SUBDIRS    = $(MAYBE_OSL) doc
ACLOCAL_AMFLAGS = -I m4

#############################################################################

bin_PROGRAMS    = clay
lib_LTLIBRARIES = libclay.la

#############################################################################

pkginclude_HEADERS = \
	include/clay/clay.h \
	include/clay/array.h \
	include/clay/options.h \
	include/clay/errors.h \
  include/clay/prototype_function.h \
	include/clay/transformation.h

DEFAULT_INCLUDES = -I.
INCLUDES = -I$(top_builddir)/include -I$(top_srcdir)/include
AM_CFLAGS = $(CFLAGS_WARN)
YFLAGS = -y -d

#############################################################################

libclay_la_LIBADD   = @OSL_LIBS@ $(OSL_LA)
libclay_la_CPPFLAGS = @OSL_CPPFLAGS@ -g
libclay_la_LDFLAGS  = @OSL_LDFLAGS@
libclay_la_SOURCES  = \
  source/parser.y \
  source/scanner.l \
  source/array.c \
  source/options.c \
  source/prototype_function.c \
  source/transformation.c

#############################################################################

LDADD             = @OSL_LIBS@ libclay.la
clay_CPPFLAGS     = @OSL_CPPFLAGS@ -g
clay_LDFLAGS      = #@OSL_LDFLAGS@ # TO BE REMOVED
clay_DEPENDENCIES = libclay.la
clay_SOURCES      = source/clay.c

#############################################################################

MAINTAINERCLEANFILES = \
	Makefile.in \
	aclocal.m4 \
	configure \
	$(AUX_DIST)

#############################################################################

dist-hook:
	(cd $(distdir) && mkdir -p $(ac_aux_dir))
	for file in $(AUX_DIST); do \
	  cp $$file $(distdir)/$$file; \
	done

#############################################################################
