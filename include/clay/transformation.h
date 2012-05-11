
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
#include <clay/array.h>


#ifndef bool
#define bool short
#endif


#define CLAY_TRANSF_SUCCESS                   0
#define CLAY_TRANSF_BETA_NOT_FOUND            1
#define CLAY_TRANSF_NOT_BETA_LOOP             2
#define CLAY_TRANSF_NOT_BETA_STMT             3 // NOT USED
#define CLAY_TRANSF_REORDER_ARRAY_TOO_SMALL   4
#define CLAY_TRANSF_DEPTH_OVERFLOW            5
#define CLAY_TRANSF_WRONG_COEFF               6
#define CLAY_TRANSF_BETA_EMPTY                7
#define CLAY_TRANSF_BETA_NOT_IN_A_LOOP        8
#define CLAY_TRANSF_WRONG_BLOCK_SIZE          9
#define CLAY_TRANSF_WRONG_FACTOR              10


/*****************************************************************************\
 *                     Loop transformations                                   *
 `****************************************************************************/

int             clay_fission(osl_scop_p, clay_array_p, int);
int             clay_reorder(osl_scop_p, clay_array_p, clay_array_p);
int             clay_interchange(osl_scop_p, clay_array_p, int, int);
int             clay_reversal(osl_scop_p, clay_array_p);
int             clay_skew(osl_scop_p, clay_array_p, int, int);
int             clay_fuse(osl_scop_p, clay_array_p);
int             clay_iss(osl_scop_p, clay_array_p, clay_array_p);
int             clay_stripmine(osl_scop_p, clay_array_p, int, int);
int             clay_unroll(osl_scop_p, clay_array_p, int);


/*****************************************************************************\
 *                     Statement/Beta operations                              *
 `****************************************************************************/

int             clay_statement_get_line(osl_statement_p, int);
bool            clay_statement_is_after(osl_statement_p, clay_array_p);

osl_statement_p clay_beta_find(osl_statement_p, clay_array_p);
osl_statement_p clay_beta_first_statement(osl_statement_p, clay_array_p);
bool            clay_beta_check(osl_statement_p, clay_array_p);
void*           clay_beta_max_value(osl_statement_p, clay_array_p);
int             clay_beta_nb_parts(osl_statement_p,clay_array_p);
void            clay_beta_shift(osl_statement_p, clay_array_p, int);


/*****************************************************************************\
 *                     String operation                                       *
 `****************************************************************************/

char*           clay_string_replace(char*, char*, char*);

#endif
