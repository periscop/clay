
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             Clay.c                                 |
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

#ifndef bool
#define bool short
#endif

#define CLAY_MAX_BETA_SIZE 20


// NOTE : a beta is just a clay_array_p


int              clay_relation_get_line(osl_relation_p, int);
bool             clay_statement_is_after(osl_statement_p, clay_array_p);
bool             clay_statement_is_before(osl_statement_p, clay_array_p);

void             clay_beta_normalize(osl_scop_p);
clay_array_p     clay_beta_get(osl_statement_p);
osl_statement_p  clay_beta_find(osl_statement_p, clay_array_p);
bool             clay_beta_check(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_min(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_max(osl_statement_p, clay_array_p);
clay_array_p     clay_beta_next(osl_statement_p, clay_array_p beta,
                                osl_statement_p*);
clay_array_p     clay_beta_next_part(osl_statement_p, clay_array_p beta);
int              clay_beta_nb_parts(osl_statement_p,clay_array_p);
void             clay_beta_shift_after(osl_statement_p, clay_array_p, int);
void             clay_beta_shift_before(osl_statement_p, clay_array_p, int);

bool             clay_scattering_check_zeros(osl_statement_p, int, int);


#endif
