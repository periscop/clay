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
SUBDIRS =

#############################################################################

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	htmldoc.tar.gz \
	clay.pdf

info_TEXINFOS = clay.texi

DOCDIR = htmldoc
DOXYFILE = Doxyfile
DOXYGEN = @DOXYGEN@

# The following requires a fixed version of the Emacs 19.30 etags.
ETAGS_ARGS = --lang=none \
 --regex='/^@node[ \t]+\([^,]+\)/\1/' $(srcdir)/doc/clay.texi
TAGS_DEPENDENCIES = clay.texi $(DOXYFILE).in

htmldoc.tar.gz:
	mkdir -p $(top_srcdir)/doc/htmldoc
	$(DOXYGEN) $(DOXYFILE)
	tar czf htmldoc.tar.gz htmldoc
