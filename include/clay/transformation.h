
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
 

#ifndef CLAY_TRANSFORMATIONS_H
#define CLAY_TRANSFORMATIONS_H


#include <osl/statement.h>
#include <osl/scop.h>
#include <osl/extensions/scatnames.h>
#include <clay/array.h>
#include <clay/options.h>


#ifndef bool
#define bool short
#endif



// used by the normalize function
#define CLAY_TRANSFORMATIONS_MAX_BETA_SIZE 20




/*****************************************************************************\
 *                     Loop transformations                                   *
 `****************************************************************************/

int        clay_fission(osl_scop_p, clay_array_p, int, clay_options_p);
int        clay_reorder(osl_scop_p, clay_array_p, clay_array_p, clay_options_p);
int        clay_interchange(osl_scop_p, clay_array_p, int, int, clay_options_p);
int        clay_reversal(osl_scop_p, clay_array_p, int, clay_options_p);
int        clay_skew(osl_scop_p, clay_array_p, int, int, clay_options_p);
int        clay_fuse(osl_scop_p, clay_array_p, clay_options_p);
int        clay_iss(osl_scop_p, clay_array_p, clay_array_p, clay_options_p);
int        clay_stripmine(osl_scop_p, clay_array_p, int, int, int,
                          clay_options_p);
int        clay_unroll(osl_scop_p, clay_array_p, int, clay_options_p);


/*****************************************************************************\
 *                     Other operations                                       *
 `****************************************************************************/

void            clay_scop_normalize_beta(osl_scop_p);

int             clay_statement_get_line(osl_statement_p, int);
clay_array_p    clay_statement_get_beta(osl_statement_p);
bool            clay_statement_is_after(osl_statement_p, clay_array_p);
bool            clay_statement_is_before(osl_statement_p, clay_array_p);
bool            clay_scattering_check_zero(osl_statement_p, int, int);

bool            clay_scatnames_exists(osl_scatnames_p, char*);

osl_statement_p clay_beta_find(osl_statement_p, clay_array_p);
osl_statement_p clay_beta_first_statement(osl_statement_p, clay_array_p);
bool            clay_beta_check(osl_statement_p, clay_array_p);
clay_array_p    clay_beta_min(osl_statement_p, clay_array_p);
clay_array_p    clay_beta_max(osl_statement_p, clay_array_p);
clay_array_p    clay_beta_next(osl_statement_p, clay_array_p beta,
                               osl_statement_p*);
clay_array_p    clay_beta_next_part(osl_statement_p, clay_array_p beta);
int             clay_beta_nb_parts(osl_statement_p,clay_array_p);
void            clay_beta_shift_after(osl_statement_p, clay_array_p, int);
void            clay_beta_shift_before(osl_statement_p, clay_array_p, int);

char*           clay_string_replace(char*, char*, char*);

#endif
