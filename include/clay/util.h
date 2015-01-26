
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             util.h                                 |
    |--------------------------------------------------------------------|
    |                    First version: 03/04/2012                       |
    +--------------------------------------------------------------------+

 +--------------------------------------------------------------------------+
 |  / __)(  )    /__\ ( \/ )                                                |
 | ( (__  )(__  /(__)\ \  /         Chunky Loop Alteration wizardrY         |
 |  \___)(____)(__)(__)(__)                                                 |
 +--------------------------------------------------------------------------+
 | Copyright (C) 2012 University of Paris-Sud                               |
 |                                                                          |
 | This library is free software; you can redistribute it and/or modify it  |
 | under the terms of the GNU Lesser General Public License as published by |
 | the Free Software Foundation; either version 2.1 of the License, or      |
 | (at your option) any later version.                                      |
 |                                                                          |
 | This library is distributed in the hope that it will be useful but       |
 | WITHOUT ANY WARRANTY; without even the implied warranty of               |
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser  |
 | General Public License for more details.                                 |
 |                                                                          |
 | You should have received a copy of the GNU Lesser General Public License |
 | along with this software; if not, write to the Free Software Foundation, |
 | Inc., 51 Franklin Street, Fifth Floor,                                   |
 | Boston, MA  02110-1301  USA                                              |
 |                                                                          |
 | Clay, the Chunky Loop Alteration wizardrY                                |
 | Written by Joel Poudroux, joel.poudroux@u-psud.fr                        |
 +--------------------------------------------------------------------------*/


#ifndef CLAY_UTIL_H
#define CLAY_UTIL_H

#ifndef bool
#define bool short
#endif

#include <clay/array.h>
#include <clay/list.h>
#include <osl/scop.h>
#include <osl/statement.h>
#include <osl/relation.h>
#include <osl/extensions/extbody.h>
#include <osl/extensions/scatnames.h>
#include <osl/extensions/arrays.h>

# if defined(__cplusplus)
extern "C"
  {
# endif

/*void
  clay_util_statement_insert_inequation
  (osl_statement_p, clay_array_p, int, int);*/

void  clay_util_scop_export_body(osl_scop_p);

void  clay_util_scattering_update_beta(osl_relation_p, clay_array_p);

int   clay_util_relation_get_line(osl_relation_p, int);
void  clay_util_relation_negate_row(osl_relation_p, int);
void  clay_util_relation_insert_inequation(osl_relation_p, clay_list_p);
void  clay_util_relation_set_vector(osl_relation_p, clay_list_p, int);

int   clay_util_statement_find_iterator(osl_statement_p, char*);
void  clay_util_statement_set_inequation(osl_statement_p, clay_list_p);
void  clay_util_statement_set_vector(osl_statement_p, clay_list_p, int);
osl_statement_p clay_util_statement_insert(osl_statement_p,
                                           osl_statement_p,
                                           int,
                                           int);
void  clay_util_statement_replace_beta(osl_statement_p,
                                       clay_array_p,
                                       clay_array_p);

void  clay_util_array_output_dims_pad_zero(clay_array_p);
bool  clay_util_scatnames_exists(osl_scatnames_p, char*);
char* clay_util_string_replace(char*, char*, char*);

void  clay_util_body_regenerate_access(osl_extbody_p, 
                                       osl_relation_p, 
                                       int,
                                       osl_arrays_p,
                                       osl_scatnames_p,
                                       osl_strings_p);

int   clay_util_arrays_search(osl_arrays_p, unsigned int);

int clay_util_foreach_access(osl_scop_p, clay_array_p, unsigned int, 
                             int (*func)(osl_relation_list_p, void*), void*,
                             int);
int clay_util_is_row_beta_definition(osl_relation_p relation, int row);

# if defined(__cplusplus)
  }
# endif

#endif
