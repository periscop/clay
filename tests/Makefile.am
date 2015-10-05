
#   /*--------------------------------------------------------------------+
#    |                              Clay                                  |
#    |--------------------------------------------------------------------|
#    |                             Clay.c                                 |
#    |--------------------------------------------------------------------|
#    |                    First version: 03/04/2012                       |
#    +--------------------------------------------------------------------+

# +--------------------------------------------------------------------------+
# |  / __)(  )    /__\ ( \/ )                                                |
# | ( (__  )(__  /(__)\ \  /         Chunky Loop Alteration wizardrY         |
# |  \___)(____)(__)(__)(__)                                                 |
# +--------------------------------------------------------------------------+
# | Copyright (C) 2012 University of Paris-Sud                               |
# |                                                                          |
# | This library is free software; you can redistribute it and/or modify it  |
# | under the terms of the GNU Lesser General Public License as published by |
# | the Free Software Foundation; either version 2.1 of the License, or      |
# | (at your option) any later version.                                      |
# |                                                                          |
# | This library is distributed in the hope that it will be useful but       |
# | WITHOUT ANY WARRANTY; without even the implied warranty of               |
# | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser  |
# | General Public License for more details.                                 |
# |                                                                          |
# | You should have received a copy of the GNU Lesser General Public License |
# | along with this software; if not, write to the Free Software Foundation, |
# | Inc., 51 Franklin Street, Fifth Floor,                                   |
# | Boston, MA  02110-1301  USA                                              |
# |                                                                          |
# | Clay, the Chunky Loop Alteration wizardrY                                |
# | Written by Joel Poudroux, joel.poudroux@u-psud.fr                        |
# |            Cedric Bastoul <Cedric.Bastoul@u-psud.fr>                     |
# +--------------------------------------------------------------------------*/

#############################################################################
SUBDIRS 		=

#############################################################################

MAINTAINERCLEANFILES    = Makefile.in

TESTS_ENVIRONMENT       = top_builddir=$(top_builddir) SRCDIR=$(srcdir)

#############################################################################

check_SCRIPTS		= check_unitary.sh check_fail.sh check_tests.sh

EXTRA_DIST              = checker.sh

## Replace by program names when needed
# check_PROGRAMS = ...
TESTS			= $(check_SCRIPTS)

TEST_FILES = \
	$(top_srcdir)/tests/tests/ax-do1.c \
	$(top_srcdir)/tests/tests/ax-do_double_tile.c \
	$(top_srcdir)/tests/tests/ax-do3.c \
	$(top_srcdir)/tests/tests/iss_collapse_invariant.c
	
UNITARY_TEST_FILES = \
	$(top_srcdir)/tests/unitary/empty_code.c \
	$(top_srcdir)/tests/unitary/empty_script.c \
	$(top_srcdir)/tests/unitary/split_loop1.c \
	$(top_srcdir)/tests/unitary/split_nothing_before.c \
	$(top_srcdir)/tests/unitary/split_statement1.c \
	$(top_srcdir)/tests/unitary/split_statement2.c \
	$(top_srcdir)/tests/unitary/split_statement3.c \
	$(top_srcdir)/tests/unitary/fuse.c \
	$(top_srcdir)/tests/unitary/fuse_diff_domain.c \
	$(top_srcdir)/tests/unitary/fuse_nothing_after.c \
	$(top_srcdir)/tests/unitary/fuse_statement_between.c \
	$(top_srcdir)/tests/unitary/interchange_loop1.c \
	$(top_srcdir)/tests/unitary/interchange_loop2.c \
	$(top_srcdir)/tests/unitary/interchange_loop3.c \
	$(top_srcdir)/tests/unitary/interchange_loop4.c \
	$(top_srcdir)/tests/unitary/interchange_nochanges.c \
	$(top_srcdir)/tests/unitary/interchange_statement.c \
	$(top_srcdir)/tests/unitary/iss_loop1.c \
	$(top_srcdir)/tests/unitary/iss_loop2.c \
	$(top_srcdir)/tests/unitary/iss_loop3.c \
	$(top_srcdir)/tests/unitary/iss_loop4.c \
	$(top_srcdir)/tests/unitary/reorder_all.c \
	$(top_srcdir)/tests/unitary/reorder_nochanges.c \
	$(top_srcdir)/tests/unitary/reorder_simple.c \
	$(top_srcdir)/tests/unitary/reorder_swap_loops.c \
	$(top_srcdir)/tests/unitary/reverse_loop1.c \
	$(top_srcdir)/tests/unitary/reverse_loop2.c \
	$(top_srcdir)/tests/unitary/reverse_statement1.c \
	$(top_srcdir)/tests/unitary/reverse_statement2.c \
	$(top_srcdir)/tests/unitary/skew_loop1.c \
	$(top_srcdir)/tests/unitary/skew_loop2.c \
	$(top_srcdir)/tests/unitary/skew_loop3.c \
	$(top_srcdir)/tests/unitary/stripmine_loop1.c \
	$(top_srcdir)/tests/unitary/stripmine_non_pretty.c \
	$(top_srcdir)/tests/unitary/stripmine_pretty.c \
	$(top_srcdir)/tests/unitary/stripmine_statement.c \
	$(top_srcdir)/tests/unitary/unroll1.c \
	$(top_srcdir)/tests/unitary/unroll2.c \
	$(top_srcdir)/tests/unitary/tile_loop1.c \
	$(top_srcdir)/tests/unitary/shift_loop1.c \
	$(top_srcdir)/tests/unitary/shift_loop2.c \
	$(top_srcdir)/tests/unitary/shift_statement.c \
	$(top_srcdir)/tests/unitary/peel_first.c \
	$(top_srcdir)/tests/unitary/peel_first_nochanges.c \
	$(top_srcdir)/tests/unitary/peel_last.c \
	$(top_srcdir)/tests/unitary/peel_last_nochanges.c \
	$(top_srcdir)/tests/unitary/peel_remove_condition.c \
	$(top_srcdir)/tests/unitary/context.c \
	$(top_srcdir)/tests/unitary/dimreorder.c \
	$(top_srcdir)/tests/unitary/dimprivatize_simple.c \
	$(top_srcdir)/tests/unitary/dimcontract.c \
	$(top_srcdir)/tests/unitary/datacopy.c \
	$(top_srcdir)/tests/unitary/block.c \
	$(top_srcdir)/tests/unitary/block_inside.c \
	$(top_srcdir)/tests/unitary/grain_stmt.c \
	$(top_srcdir)/tests/unitary/grain_loop.c \
	$(top_srcdir)/tests/unitary/densify_stmt.c \
	$(top_srcdir)/tests/unitary/densify_loop.c \
	$(top_srcdir)/tests/unitary/collapse_loop.c \
	$(top_srcdir)/tests/unitary/collapse_multi.c \
	$(top_srcdir)/tests/unitary/collapse_nested.c

FAIL_TEST_FILES = \
	$(top_srcdir)/tests/must_fail/beta_not_found.c \
	$(top_srcdir)/tests/must_fail/split_overflow_loop.c \
	$(top_srcdir)/tests/must_fail/split_overflow_statement.c \
	$(top_srcdir)/tests/must_fail/interchange_overflow_loop.c \
	$(top_srcdir)/tests/must_fail/interchange_overflow_statement2.c \
	$(top_srcdir)/tests/must_fail/interchange_overflow_statement.c \
	$(top_srcdir)/tests/must_fail/iss_inequation.c \
	$(top_srcdir)/tests/must_fail/peel_not_a_loop.c \
	$(top_srcdir)/tests/must_fail/reorder_array_too_small.c \
	$(top_srcdir)/tests/must_fail/reverse_overflow_loop.c \
	$(top_srcdir)/tests/must_fail/reverse_overflow_statement.c \
	$(top_srcdir)/tests/must_fail/skew_overflow_loop.c \
	$(top_srcdir)/tests/must_fail/skew_overflow_statement.c \
	$(top_srcdir)/tests/must_fail/skew_overflow_statement2.c \
	$(top_srcdir)/tests/must_fail/skew_statement.c \
	$(top_srcdir)/tests/must_fail/skew_statement1.c \
	$(top_srcdir)/tests/must_fail/stripmine_not_in_a_loop.c \
	$(top_srcdir)/tests/must_fail/context.c \
	$(top_srcdir)/tests/must_fail/dimprivatize_cant_privatize.c \
	$(top_srcdir)/tests/must_fail/grain_overflow.c \
	$(top_srcdir)/tests/must_fail/grain_negative.c \
	$(top_srcdir)/tests/must_fail/collapse_multi.c

TEST_FILES_REF = $(TEST_FILES:.c=.c.scop)
UNITARY_TEST_FILES_REF = $(UNITARY_TEST_FILES:.c=.c.scop)
FAIL_TEST_FILES_REF = $(FAIL_TEST_FILES:.c=.c.scop)

EXTRA_DIST += \
	$(UNITARY_TEST_FILES) \
	$(TEST_FILES) \
	$(FAIL_TEST_FILES) \
	$(check_SCRIPTS)

CLEANFILES = ""

TESTS_ENVIRONMENT += \
	CHECKER="$(top_srcdir)/tests/checker.sh" \
	TEST_FILES="$(TEST_FILES)" \
	UNITARY_TEST_FILES="$(UNITARY_TEST_FILES)" \
	FAIL_TEST_FILES="$(FAIL_TEST_FILES)"
