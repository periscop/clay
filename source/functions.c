
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             functions.c                            |
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


#include <stdlib.h>
#include <clay/functions.h>
#include <clay/prototype_function.h>


// Authorized functions in Clay


int clay_function_split_type[]       = {ARRAY_T, INTEGER_T};
int clay_function_reorder_type[]     = {ARRAY_T, ARRAY_T};
int clay_function_interchange_type[] = {ARRAY_T, INTEGER_T, INTEGER_T, INTEGER_T};
int clay_function_reverse_type[]     = {ARRAY_T, INTEGER_T};
int clay_function_fuse_type[]        = {ARRAY_T};
int clay_function_skew_type[]        = {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_iss_type[]         = {ARRAY_T, LIST_T};
int clay_function_stripmine_type[]   = {ARRAY_T, INTEGER_T, INTEGER_T, INTEGER_T};
int clay_function_unroll_type[]      = {ARRAY_T, INTEGER_T};
int clay_function_tile_type[]        = {ARRAY_T, INTEGER_T, INTEGER_T, INTEGER_T, INTEGER_T};
int clay_function_shift_type[]       = {ARRAY_T, INTEGER_T, LIST_T};
int clay_function_peel_type[]        = {ARRAY_T, LIST_T};
int clay_function_context_type[]     = {ARRAY_T};
int clay_function_dimreorder_type[]  = {ARRAY_T, INTEGER_T, ARRAY_T};
int clay_function_dimprivatize_type[] = {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_dimcontract_type[] = {ARRAY_T, INTEGER_T, INTEGER_T};


// That is just the prototype of each functions, so there are no data for args
const clay_prototype_function_t functions[CLAY_FUNCTIONS_TOTAL] = 
{
  {
    "split",      
    "split(ident, uint depth)",
    NULL, clay_function_split_type, 2, 0
  },
  {
    "reorder",
    "reorder(ident:loop, array neworder)",
    NULL, clay_function_reorder_type, 2, 0
  },
  {
    "interchange", 
    "interchange(ident:inner, uint depth_1, uint depth_2, bool pretty)",
    NULL, clay_function_interchange_type, 4, 0
  },
  {
    "reverse",
    "reverse(ident, uint depth)",
    NULL, clay_function_reverse_type, 2, 0
  },
  {
    "fuse",
    "fuse(ident:loop)",
    NULL, clay_function_fuse_type, 1, 0
  },
  {
    "skew",      
    "skew(ident, uint depth, int coeff)",
    NULL, clay_function_skew_type, 3, 0
  },
  {
    "iss",    
    "iss(ident:loop, list inequation { ((output,) params,)) const }",
    NULL, clay_function_iss_type, 2, 0
  },
  {
    "stripmine",
    "stripmine(ident, uint depth, uint size, bool pretty)",
    NULL, clay_function_stripmine_type, 4, 0
  },
  {
    "unroll",  
    "unroll(ident:loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 0
  },
  {
    "unroll_noepilog",
    "unroll_noepilog(ident:loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 0
  },
  {
    "tile",    
    "tile(ident, uint depth, uint depth_outer, uint size, bool pretty)",
    NULL, clay_function_tile_type, 5, 0
  },
  {
    "shift",   
    "shift(ident, uint depth, list vector { ((output,) (params,)) const })",
    NULL, clay_function_shift_type, 3, 0
  },
  {
    "peel",   
    "peel(ident:loop,  list inequation { ((output,) params,)) const })",
    NULL, clay_function_peel_type, 2, 0
  },
  {
    "context", 
    "context(array vector)",
    NULL, clay_function_context_type, 1, 0
  },
  {
    "dimreorder",
    "dimreorder(ident, uint #access, array neworder)",
    NULL, clay_function_dimreorder_type, 3, 0
  },
  {
    "dimprivatize",
    "dimprivatize(ident, uint depth, uint #access)",
    NULL, clay_function_dimprivatize_type, 3, 0
  },
  {
    "dimcontract",
    "dimcontract(ident, uint depth, uint #access)",
    NULL, clay_function_dimcontract_type, 3, 0
  }

};
