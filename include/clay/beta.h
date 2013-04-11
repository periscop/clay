
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             beta.h                                 |
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
 

#ifndef CLAY_BETA_H
#define CLAY_BETA_H

#include <osl/scop.h>
#include <osl/statement.h>
#include <osl/relation.h>
#include <clay/array.h>


// NOTE : a beta is just a clay_array_p


#define CLAY_MAX_BETA_SIZE 20


#define CLAY_BETA_IS_LOOP(beta, statement)                          \
  do {                                                              \
    if (beta->size*2-1 >= statement->scattering->nb_output_dims)    \
      return CLAY_ERROR_NOT_BETA_LOOP;                              \
  } while (0)


#define CLAY_BETA_IS_STMT(beta, statement)                          \
  do {                                                              \
    if (beta->size*2-1 != statement->scattering->nb_output_dims)    \
      return CLAY_ERROR_NOT_BETA_STMT;                              \
  } while (0)


#define CLAY_BETA_CHECK_DEPTH(beta, depth, statement)               \
  do {                                                              \
    /* check if it's a statement */                                 \
    /* the depth must be strictly less than the beta size */        \
    if (beta->size*2-1 >= statement->scattering->nb_output_dims &&  \
        depth >= beta->size)                                        \
      return CLAY_ERROR_DEPTH_OVERFLOW;                             \
    /* else it's a loop, and the depth must be less or equal */     \
    /* than the beta size */                                        \
    if (depth > beta->size)                                         \
      return CLAY_ERROR_DEPTH_OVERFLOW;                             \
  } while(0)


int              clay_statement_is_after(osl_statement_p, clay_array_p);
int              clay_statement_is_before(osl_statement_p, clay_array_p);

void             clay_beta_sort(osl_scop_p);
void             clay_beta_normalize(osl_scop_p);
clay_array_p     clay_beta_get(osl_statement_p);
osl_statement_p  clay_beta_find(osl_statement_p, clay_array_p);
int              clay_beta_check(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_min(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_max(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_next(osl_statement_p, clay_array_p beta,
                                osl_statement_p*);
clay_array_p     clay_beta_next_part(osl_statement_p, clay_array_p beta);
int              clay_beta_nb_parts(osl_statement_p,clay_array_p);
void             clay_beta_shift_after(osl_statement_p, clay_array_p, int);
void             clay_beta_shift_before(osl_statement_p, clay_array_p, int);

int              clay_scattering_check_zeros(osl_statement_p, int, int);


#endif
