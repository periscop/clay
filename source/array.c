
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             array.c                                |
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clay/macros.h>
#include <clay/array.h>


/**
 * clay_array_malloc function:
 * \return     new struct array
 */
clay_array_p clay_array_malloc() {
  clay_array_p arr;
  CLAY_malloc(arr, clay_array_p, sizeof(clay_array_t));
  CLAY_malloc(arr->data, int*, sizeof(int)*CLAY_ARRAY_INIT_SIZE);
  arr->size = 0;
  arr->available = CLAY_ARRAY_INIT_SIZE;
  return arr;
}


/**
 * clay_array_add function:
 * Push i at the end of arr
 * \param[in,out] arr
 * \param[in] i
 */
void clay_array_add(clay_array_p arr, int i) {
  if (arr->size >= arr->available) {
    arr->available *= 2;
    CLAY_realloc(arr->data, int*, sizeof(int) * arr->available);
  }
  arr->data[arr->size] = i;
  (arr->size)++;
}


/**
 * clay_array_remove_last function:
 * Remove last element of the array if exists.
 * \param[in,out] array A Clay array
 */
void clay_array_remove_last(clay_array_p array) {
  if (array != NULL && array->size > 0) { --(array->size); }
}

/**
 * Remove all the elements from the array without deallocating the space.
 * \param[in,out] array   A Clay array
 */
void clay_array_clear(clay_array_p array) {
  if (array == NULL)
    return;

  array->size = 0;
}


/**
 * clay_array_free function: 
 * \param[in] arr
 */
void clay_array_free(clay_array_p arr) {
  if (arr) {
    if (arr->data) {
      free(arr->data);
    }
    free(arr);
  }
}


/**
 * clay_array_print function: 
 * Print the array like this : [0,1,2,3,4,5,6]
 * \param[in] out   file where to print
 * \param[in] arr
 * \param[in] cr    if 1, it will print a \n at the end
 */
void clay_array_print(FILE *out, clay_array_p arr, int cr) {
  if (arr == NULL) {
    fprintf(out, "NULL\n");
    return;
  }
  int i;
  fprintf(out, "[");
  for (i = 0 ; i < arr->size-1 ; i++) {
    fprintf(out, "%d,", arr->data[i]);
  }
  if(arr->size > 0)
    fprintf(out, "%d", arr->data[i]);
  fprintf(out, "]");

  if (cr)
    fprintf(out, "\n");
}


/**
 * clay_array_clone function: 
 * \param[in] arr
 * \return    cloned array
 */
clay_array_p clay_array_clone(clay_array_p arr) {
  clay_array_p newarr = clay_array_malloc();
  int i;
  for (i = 0 ; i < arr->size ; i++) {
    clay_array_add(newarr, arr->data[i]);
  }
  return newarr;
}


/**
 * clay_array_concat function: 
 * a1 = a1 + a2
 * \param[in,out] a1
 * \param[in] a2
 */
void clay_array_concat(clay_array_p a1, clay_array_p a2) {
  int i;
  for (i = 0 ; i < a2->size ; i++) {
    clay_array_add(a1, a2->data[i]);
  }
}


/**
 * clay_array_equal function: 
 * a1 == a2
 * \param[in] a1
 * \param[in] a2
 * \return    0 or 1
 */
int clay_array_equal(clay_array_p a1, clay_array_p a2) {
  if (a1->size != a2->size)
    return 0;

  int i;
  for (i = 0 ; i < a1->size ; i++)
    if (a1->data[i] != a2->data[i])
      return 0;

  return 1;
}

/**
 * Checks if an array contains the given value.
 * \param [in] a1     the array.
 * \parma [in] value  the value to look for.
 * \returns     1 if a1 contains value, 0 otherwise.
 */
int clay_array_contains(clay_array_p array, int value) {
  int i;
  for (i = 0; i < array->size; i++) {
    if (array->data[i] == value) {
      return 1;
    }
  }
  return 0;
}

/**
 * Get textual representaiton of the array.
 * \param [in] array  The array.
 * \returns           Textual representation [1,2,3,4].  Allocates memory for
 *                    the string and transfers ownership to the caller.
 */
char *clay_array_string(clay_array_p array) {
  size_t length = 3 + array->size * sizeof(int) * 4;
  char *string = (char *) malloc(length);
  char *start = string;
  int i;
  char buffer[sizeof(int) * 3 + 1];
  int watermark = length;

  snprintf(string, watermark, "[");
  string += 1;
  watermark -= 1;

  for (i = 0; i < array->size - 1; i++) {
    int current_length;
    snprintf(buffer, sizeof(int) * 3 + 1, "%d", array->data[i]);
    snprintf(string, watermark, "%s,", buffer);
    current_length = strlen(buffer);
    string += current_length + 1;
    watermark -= current_length + 1;
  }
  if (array->size != 0) {
    int current_length;
    snprintf(buffer, sizeof(int) * 3 + 1, "%d", array->data[array->size - 1]);
    snprintf(string, watermark, "%s", buffer);
    current_length = strlen(buffer);
    string += current_length;
    watermark -= current_length;
  }
  snprintf(string, watermark, "]");

  return start;
}

