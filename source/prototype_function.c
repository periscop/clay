
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


#include <stdlib.h>
#include <stdio.h>
#include <clay/prototype_function.h>
#include <clay/macros.h>
#include <clay/array.h>


/**
 * clay_prototype_function_malloc function:
 * \return     Return a new function structure
 */
clay_prototype_function_p clay_prototype_function_malloc() {
  clay_prototype_function_p f;
  CLAY_malloc(f, clay_prototype_function_p, sizeof(clay_prototype_function_t));
  CLAY_malloc(f->args, void**, sizeof(void*) * 10);
  CLAY_malloc(f->type, int*, sizeof(int) * 10);
  f->argc = 0;
  f->available = 10;
  return f;
}


/**
 * clay_prototype_function_args_add function
 * \param[in] function structure
 * \param[in] param       Parameter to add in the args
 * \param[in] type        Type of the parameter
 */
void clay_prototype_function_args_add(clay_prototype_function_p f, void *param, 
                                   int type) {
  if (f->argc >= f->available) {
    f->available += 10;
    CLAY_realloc(f->args, void**, sizeof(void*) * f->available);
    CLAY_realloc(f->type, int*, sizeof(int) * f->available);
  }
  f->args[f->argc] = param;
  f->type[f->argc] = type;
  (f->argc)++;
}


/**
 * clay_prototype_function_args_clear function: 
 * Clear all the parameters
 * \param[in] function structure
 */
void clay_prototype_function_args_clear(clay_prototype_function_p f) {
  int i;
  if (f->args) {
    for (i = 0 ; i < f->argc ; i++) {
      if (f->args[i]) {
        switch (f->type[i]) {
          case ARRAY_T:
            clay_array_free(f->args[i]);
            break;
          case INTEGER_T:
            free(f->args[i]);
            break;
        }
      }
    }
  }
  f->argc = 0;
}

/**
 * clay_prototype_function_free function: 
 * Delete everything
 * \param[in] function structure
 */
void clay_prototype_function_free(clay_prototype_function_p f) {
  clay_prototype_function_args_clear(f);
  free(f->args);
  free(f->type);
  free(f);
}



/**
 * clay_prototype_function_conv_int2array function: 
 * Convert the `index'th arg into a clay_array_p
 * If size == 3 and f->arg[index] == 8:
 * result = [0,0,0,8]
 * \param[in] function structure
 */
void  clay_prototype_function_conv_int2array(clay_prototype_function_p f,
                                             int index,
                                             int size) {
  clay_array_p tmp;
  int i, val;
  
  if (f->type[index] == INTEGER_T) {
    
    tmp = clay_array_malloc();
    for (i = 0 ; i < size ; i++) {
      clay_array_add(tmp, 0);
    }
    
    val = *((int*)f->args[index]);
    clay_array_add(tmp, val);
    
    free(f->args[index]);
    f->args[index] = tmp;
    
    f->type[index] = ARRAY_T;
    
    
    clay_array_print(stderr, f->args[index]);
  }
}
