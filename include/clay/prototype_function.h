
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                           prototype_function.h                     |
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


#ifndef CLAY_PROTOTYPE_FUNCTION_H
#define CLAY_PROTOTYPE_FUNCTION_H

#define ARRAY_T               0
#define INTEGER_T             1
#define ARRAY_OR_INTEGER_T    2

struct clay_prototype_function {
  char *name;
  char *prototype; // prototype of the function
  void **args;
  int *type;
  int argc; // nb args (size of type and args)
  int available; // total allocated memory for the array args and type
  //void (*func)(osl_statement_p,...); // function pointer
};

typedef struct    clay_prototype_function  clay_prototype_function_t;
typedef struct    clay_prototype_function* clay_prototype_function_p;

clay_prototype_function_p  clay_prototype_function_malloc();
void              clay_prototype_function_free(clay_prototype_function_p);
void              clay_prototype_function_args_add(clay_prototype_function_p, 
                                                   void*, int);
void              clay_prototype_function_args_clear(clay_prototype_function_p);

void              clay_prototype_function_conv_int2array(
                                                clay_prototype_function_p,
                                                int,
                                                int);
  
#endif
