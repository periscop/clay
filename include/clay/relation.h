
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                           relation.h                               |
    |--------------------------------------------------------------------|
    |                    First version: 01/10/2015                       |
    +--------------------------------------------------------------------+

 +--------------------------------------------------------------------------+
 |  / __)(  )    /__\ ( \/ )                                                |
 | ( (__  )(__  /(__)\ \  /         Chunky Loop Alteration wizardrY         |
 |  \___)(____)(__)(__)(__)                                                 |
 +--------------------------------------------------------------------------+
 | Copyright (C) 2012 University of Paris-Sud                               |
 | Copyright (c) 2015 Inria                                                 |
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
 | Written by Oleksandr Zinenko, oleksandr.zinenko@inria.fr                 |
 +--------------------------------------------------------------------------*/

#ifndef CLAY_RELATION_H
#define CLAY_RELATION_H

#include <clay/array.h>
#include <clay/util.h>

#include <osl/osl.h>

#if defined(__cplusplus)
extern "C"
  {
#endif

osl_int_t clay_relation_line_gcd(osl_relation_p, int, int, int);
osl_int_t clay_relation_gcd(osl_relation_p, int);
void      clay_relation_normalize_alpha(osl_relation_p);
int       clay_relation_output_form(osl_relation_p);
void      clay_relation_zero_coefficient(osl_relation_p, int, int, int);
int       clay_relation_output_form(osl_relation_p);
int       clay_relation_rank(osl_relation_p);
int       clay_relation_rank_intrusive(osl_relation_p);
void      clay_relation_alpha_equation_rows(clay_array_p, osl_relation_p);
int       clay_relation_nb_explicit_dim(osl_relation_p);
int       clay_relation_nb_explicit_dim_intrusive(osl_relation_p);
void      clay_relation_sort_rows(osl_relation_p);
void      clay_relation_substitute(osl_relation_p, int, int, int);

#if defined(__cplusplus)
  }
#endif

#endif //CLAY_RELATION_H
