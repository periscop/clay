dnl
dnl   /*------------------------------------------------------------------+
dnl    |                             Clay                                 |
dnl    |------------------------------------------------------------------|
dnl    |                          autoconf.ac                             |
dnl    |------------------------------------------------------------------|
dnl    |                   First version: 03/04/2012                      |
dnl    +------------------------------------------------------------------+
dnl
dnl +------------------------------------------------------------------------+
dnl |  / __)(  )    /__\ ( \/ )                                              |
dnl | ( (__  )(__  /(__)\ \  /        Chunky Loop Alteration wizardrY        |
dnl |  \___)(____)(__)(__)(__)                                               |
dnl +------------------------------------------------------------------------+
dnl | Copyright (C) 2012 University of Paris-Sud                             |
dnl |                                                                        |
dnl | This library is free software; you can redistribute it and/or modify   |
dnl | it under the terms of the GNU Lesser General Public License as         |
dnl | published by the Free Software Foundation; either version 2.1 of the   |
dnl | License, or (at your option) any later version.                        |
dnl |                                                                        |
dnl | This library is distributed in the hope that it will be useful but     |
dnl | WITHOUT ANY WARRANTY; without even the implied warranty of             |
dnl | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       |
dnl | Lesser General Public License for more details.                        |
dnl |                                                                        |
dnl | You should have received a copy of the GNU Lesser General Public       |
dnl | License along with this software; if not, write to the Free Software   |
dnl | Foundation, Inc., 51 Franklin Street, Fifth Floor,                     |
dnl | Boston, MA  02110-1301  USA                                            |
dnl |                                                                        |
dnl | Clay, the Chunky Loop Alteration wizardrY                              |
dnl | Written by Cedric Bastoul, Cedric.Bastoul@u-psud.fr                    |
dnl +------------------------------------------------------------------------*/
dnl
dnl Input file for autoconf to build a configuration shellscript.

m4_define([version_major], [0])
m4_define([version_minor], [0])
m4_define([version_revision], [1])

AC_PREREQ(2.53)
dnl Fill here the @bug email adress.
AC_INIT([clay], [version_major.version_minor.version_revision],
        [cedric.bastoul@u-psud.fr])
dnl A common file, which serve as a test.
AC_CONFIG_SRCDIR([include/clay/clay.h])
dnl Put as most as possible configuration files to an auxialiry
dnl directory.
AC_CONFIG_AUX_DIR(autoconf)
AC_CONFIG_MACRO_DIR([m4])

dnl Initialize automake.
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl /**************************************************************************
dnl  *                              Checking                                  *
dnl  **************************************************************************/


dnl Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CHECK_PROG(CD, cd)
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_PROG_YACC
AC_CHECK_PROGS(DOXYGEN,doxygen,doxygen)
  
AX_CC_MAXOPT
AC_SUBST(CFLAGS_WARN)
AX_CFLAGS_WARN_ALL(CFLAGS_WARN)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stddef.h stdlib.h string.h strings.h unistd.h])

dnl Checks for library functions.
AC_CHECK_FUNCS(strtol)


dnl /**************************************************************************
dnl  *                             Option setting                             *
dnl  **************************************************************************/

dnl /**************************************************************************
dnl  *                   Where is the OpenScop Library?                       *
dnl  **************************************************************************/

AX_SUBMODULE(osl,system|build|bundled,system)

AC_SUBST(OSL_CPPFLAGS)
AC_SUBST(OSL_LDFLAGS)
AC_SUBST(OSL_LIBS)
case "$with_osl" in
bundled)
	OSL_CPPFLAGS="-I$srcdir/osl/include -Iosl/include"
	OSL_LIBS="$srcdir/osl/libosl.la"
	;;
build)
	OSL_CPPFLAGS="-I$osl_srcdir/include -I$with_osl_builddir/include"
	OSL_LIBS="$with_osl_builddir/libosl.la"
	;;
system)
  OSL_CPPFLAGS="-I$with_osl_prefix/include"
	OSL_LIBS="$with_osl_prefix/lib/libosl.la -losl"
#if test "x$with_osl_exec_prefix" != "x"; then
#		OSL_LDFLAGS="-L$with_osl_exec_prefix/lib"
#	fi
esac
AM_CONDITIONAL(BUNDLED_OSL, test $with_osl = bundled)


dnl /**************************************************************************
dnl  *                   Where is Clan ?                                      *
dnl  **************************************************************************/

AX_SUBMODULE(clan,system|build|bundled|no,no)

AC_SUBST(CLAN_CPPFLAGS)
AC_SUBST(CLAN_LDFLAGS)
AC_SUBST(CLAN_LIBS)
case "$with_clan" in
bundled)
	CLAN_CPPFLAGS="-I$srcdir/clan/include -Iclan/include -DCLAN_LINKED"
	CLAN_LIBS="$srcdir/clan/libclan.la"
	;;
build)
	CLAN_CPPFLAGS="-I$clan_srcdir/include -I$with_clan_builddir/include -DCLAN_LINKED"
	CLAN_LIBS="$with_clan_builddir/libclan.la"
	;;
system)
  CLAN_CPPFLAGS="-I$with_clan_prefix/include -DCLAN_LINKED"
	CLAN_LIBS="$with_clan_prefix/lib/libclan.la -lclan"
#	if test "x$with_clan_exec_prefix" != "x"; then
#		CLAN_LDFLAGS="-L$with_clan_exec_prefix/lib"
#	fi
esac
AM_CONDITIONAL(BUNDLED_CLAN, test $with_clan = bundled)


dnl /**************************************************************************
dnl  *                   Where is Cloog ?                                     *
dnl  **************************************************************************/

AX_SUBMODULE(cloog,system|build|bundled|no,no)

AC_SUBST(CLOOG_CPPFLAGS)
AC_SUBST(CLOOG_LDFLAGS)
AC_SUBST(CLOOG_LIBS)
case "$with_cloog" in
bundled)
	CLOOG_CPPFLAGS="-I$srcdir/cloog/include -Icloog/include -DCLOOG_LINKED"
  CLOOG_LIBS="$srcdir/cloog/libcloog-isl.la"
	;;
build)
	CLOOG_CPPFLAGS="-I$cloog_srcdir/include -I$with_cloog_builddir/include -DCLOOG_LINKED"
	CLOOG_LIBS="$with_cloog_builddir/libcloog-isl.la"
	;;
system)
  CLOOG_CPPFLAGS="-I$with_cloog_prefix/include -DCLOOG_LINKED"
	CLOOG_LIBS="$with_cloog_prefix/lib/libcloog-isl.la -lcloog-isl"
#	if test "x$with_cloog_exec_prefix" != "x"; then
#		CLOOG_LDFLAGS="-L$with_cloog_exec_prefix/lib"
#	fi
esac
AM_CONDITIONAL(BUNDLED_CLOOG, test $with_cloog = bundled)


dnl /**************************************************************************
dnl  *                         Where is Candl ?                               *
dnl  **************************************************************************/

AX_SUBMODULE(candl,system|build|bundled|no,no)

AC_SUBST(CANDL_CPPFLAGS)
AC_SUBST(CANDL_LDFLAGS)
AC_SUBST(CANDL_LIBS)
case "$with_candl" in
bundled)
	CANDL_CPPFLAGS="-I$srcdir/candl/include -Icandl/include -DCANDL_LINKED"
  CANDL_LIBS="$srcdir/candl/libcandl.la"
	;;
build)
	CANDL_CPPFLAGS="-I$candl_srcdir/include -I$with_candl_builddir/include -DCANDL_LINKED"
	CANDL_LIBS="$with_candl_builddir/libcandl.la"
	;;
system)
  CANDL_CPPFLAGS="-I$with_candl_prefix/include -DCANDL_LINKED"
	CANDL_LIBS="$with_candl_prefix/lib/libcandl.la -lcandl"
#	if test "x$with_candl_exec_prefix" != "x"; then
#		CANDL_LDFLAGS="-L$with_candl_exec_prefix/lib"
#	fi
esac
AM_CONDITIONAL(BUNDLED_CANDL, test $with_candl = bundled)


dnl /**************************************************************************
dnl  *                            Substitutions                               *
dnl  **************************************************************************/


dnl Substitutions to do.
AC_SUBST(ac_aux_dir)
AC_SUBST(abs_top_srcdir)

AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_REVISION)

dnl Configure Makefiles.
AC_CONFIG_FILES([
	Makefile
	doc/Makefile
	doc/Doxyfile
	include/clay/macros.h
	tests/Makefile
	],
	[test -z "$CONFIG_HEADERS" || echo timestamp > source/stamp-h.in])

if test $with_osl = bundled; then
	AC_CONFIG_SUBDIRS(osl)
fi

if test $with_clan = bundled; then
	AC_CONFIG_SUBDIRS(clan)
fi

if test $with_cloog = bundled; then
	AC_CONFIG_SUBDIRS(cloog)
fi

if test $with_candl = bundled; then
	AC_CONFIG_SUBDIRS(candl)
fi

AC_OUTPUT

echo "             /*-----------------------------------------------+"
echo "              |           Clay configuration is OK            |"
echo "              +-----------------------------------------------*/"
echo "It appears that your system is OK to start Clay compilation. You need"
echo "now to type \"make\". Lastly type \"make install\" to install Clay on"
echo "your system (log as root if necessary)."
