
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
 
#ifndef CLAY_ERRORS_H
#define CLAY_ERRORS_H

#define CLAY_SUCCESS                         0
#define CLAY_ERROR_BETA_NOT_FOUND            1
#define CLAY_ERROR_NOT_BETA_LOOP             2
#define CLAY_ERROR_NOT_BETA_STMT             3 // NOT USED
#define CLAY_ERROR_REORDER_ARRAY_TOO_SMALL   4
#define CLAY_ERROR_DEPTH_OVERFLOW            5
#define CLAY_ERROR_WRONG_COEFF               6
#define CLAY_ERROR_BETA_EMPTY                7
#define CLAY_ERROR_BETA_NOT_IN_A_LOOP        8
#define CLAY_ERROR_WRONG_BLOCK_SIZE          9
#define CLAY_ERROR_WRONG_FACTOR              10
#define CLAY_ERROR_UNKNOWN_FUNCTION          11
#define CLAY_ERROR_NB_ARGS                   12
#define CLAY_ERROR_INVALID_TYPE              13
#define CLAY_ERROR_DEPTH_OUTER               14
#define CLAY_ERROR_VECTOR_EMPTY              15
#define CLAY_ERROR_IDENT_STMT_NOT_FOUND      16
#define CLAY_ERROR_IDENT_NAME_NOT_FOUND      17

#endif
