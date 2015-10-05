
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                           transformation.h                         |
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
 

#ifndef CLAY_TRANSFORMATIONS_H
#define CLAY_TRANSFORMATIONS_H

#include <osl/scop.h>
#include <clay/array.h>
#include <clay/list.h>
#include <clay/options.h>

# if defined(__cplusplus)
extern "C"
  {
# endif

int clay_split(osl_scop_p, clay_array_p, unsigned int, clay_options_p);
int clay_reorder(osl_scop_p, clay_array_p, clay_array_p, clay_options_p);
int clay_interchange(osl_scop_p, clay_array_p, unsigned int, unsigned int, 
                     int, clay_options_p);
int clay_reverse(osl_scop_p, clay_array_p, unsigned int, clay_options_p);
int clay_skew(osl_scop_p, clay_array_p, unsigned int, unsigned int,
              clay_options_p);
int clay_fuse(osl_scop_p, clay_array_p, clay_options_p);
int clay_iss(osl_scop_p, clay_array_p, clay_list_p, clay_array_p*,
             clay_options_p);
int clay_stripmine(osl_scop_p, clay_array_p, unsigned int, unsigned int, int, 
                   clay_options_p);
int clay_unroll(osl_scop_p, clay_array_p, unsigned int, int, clay_options_p);
int clay_tile(osl_scop_p, clay_array_p, unsigned int, unsigned int, 
              unsigned int, int, clay_options_p);
int clay_shift(osl_scop_p, clay_array_p, unsigned int, clay_list_p, 
               clay_options_p);
int clay_peel(osl_scop_p, clay_array_p, clay_list_p, clay_options_p);
int clay_context(osl_scop_p, clay_array_p, clay_options_p);
int clay_dimreorder(osl_scop_p, clay_array_p, unsigned int, clay_array_p, 
                    clay_options_p);
int clay_dimprivatize(osl_scop_p, clay_array_p, unsigned int, unsigned int, 
                      clay_options_p);
int clay_dimcontract(osl_scop_p, clay_array_p, unsigned int, unsigned int, 
                     clay_options_p);
int clay_add_array(osl_scop_p, char*, int*, clay_options_p);
int clay_replace_array(osl_scop_p, unsigned int, unsigned int, clay_options_p);
int clay_get_array_id(osl_scop_p, char*, int*, clay_options_p);
int clay_datacopy(osl_scop_p, unsigned int, unsigned int, clay_array_p, int, 
                  clay_array_p, clay_options_p);
int clay_block(osl_scop_p, clay_array_p, clay_array_p, clay_options_p);
int clay_grain(osl_scop_p, clay_array_p, int, int, clay_options_p);
int clay_densify(osl_scop_p, clay_array_p, int, clay_options_p);
int clay_collapse(osl_scop_p, clay_array_p, clay_options_p);

# if defined(__cplusplus)
  }
# endif

#endif
