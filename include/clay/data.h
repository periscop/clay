
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             data.h                                 |
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


#ifndef CLAY_DATA_H
#define CLAY_DATA_H

#include <stdio.h>

#define INTEGER_T             0
#define VOID_T                1
#define ARRAY_T               2
#define STRING_T              3
#define LIST_T                4
#define REF_T                 5
#define UNDEF_T               6
#define MULTI_T               7

# if defined(__cplusplus)
extern "C"
  {
# endif

struct clay_data {
  int type;
  union {
    int integer;
    void *obj;
  } data;
};

typedef struct    clay_data  clay_data_t;
typedef struct    clay_data* clay_data_p;

clay_data_p  clay_data_malloc(int);
void         clay_data_free(clay_data_p);
void         clay_data_clear(clay_data_p);
void         clay_data_print(FILE*, clay_data_p);
clay_data_p  clay_data_clone(clay_data_p);

# if defined(__cplusplus)
  }
# endif

#endif

