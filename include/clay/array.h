
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             array.h                                |
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
 

#ifndef CLAY_ARRAY_H
#define CLAY_ARRAY_H

#define CLAY_ARRAY_INIT_SIZE 10
#define CLAY_LIST_INIT_SIZE 3

#include <stdio.h>

struct clay_array {
  int *data;
  int size; // memory used
  int available;  // total allocated memory
};

typedef struct clay_array clay_array_t;
typedef struct clay_array* clay_array_p;


struct clay_list {
  clay_array_p *data;
  int size;
  int available;
};

typedef struct clay_list clay_list_t;
typedef struct clay_list* clay_list_p;


clay_array_p      clay_array_malloc();
void              clay_array_add(clay_array_p, int);
void              clay_array_free(clay_array_p);
void              clay_array_print(FILE*, clay_array_p, int);
clay_array_p      clay_array_clone(clay_array_p);
void              clay_array_concat(clay_array_p, clay_array_p);
int               clay_array_equal(clay_array_p, clay_array_p);

clay_list_p       clay_list_malloc();
void              clay_list_add(clay_list_p, clay_array_p);
void              clay_list_free(clay_list_p);
void              clay_list_print(FILE*, clay_list_p);
void              clay_list_clear(clay_list_p);


#endif
