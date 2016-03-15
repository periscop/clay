
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             list.c                                 |
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
#include <clay/list.h>
#include <clay/beta.h>


clay_list_p clay_list_malloc() {
  clay_list_p l;
  CLAY_malloc(l, clay_list_p, sizeof(clay_list_t));
  CLAY_malloc(l->data, clay_array_p*, sizeof(clay_array_p) * CLAY_LIST_INIT_SIZE);
  l->size = 0;
  l->available = CLAY_LIST_INIT_SIZE;
  return l;
}


void clay_list_add(clay_list_p l, clay_array_p a) {
  if (l->size >= l->available) {
    l->available *= 2;
    CLAY_realloc(l->data, clay_array_p*, sizeof(clay_array_p) * l->available);
  }

  l->data[l->size] = a;
  (l->size)++;
}


void clay_list_free(clay_list_p l) {
  if (l) {
    int i;
    for (i = 0 ; i < l->size ; i++)
      clay_array_free(l->data[i]);
    free(l->data);
    free(l);
  }
}


void clay_list_print(FILE *out, clay_list_p l) {
  if (l == NULL) {
    fprintf(out, "NULL\n");
    return;
  }
  int i;
  fprintf(out, "{");
  for (i = 0 ; i < l->size-1 ; i++) {
    clay_array_print(out, l->data[i], 0);
    fprintf(out, ",");
  }
  if(l->size > 0)
    clay_array_print(out, l->data[i], 0);
  fprintf(out, "}\n");
}


void clay_list_clear(clay_list_p l) {
  int i;
  for (i = 0 ; i < l->size ; i++) {
    l->data[i]->size = 0;
  }
  l->size = 0;
}


clay_list_p clay_list_clone(clay_list_p l) {
  clay_list_p newl = clay_list_malloc();
  clay_array_p tmp, orig;
  int i, j;
  for (i = 0 ; i < l->size ; i++) {
    tmp = clay_array_malloc();
    orig = l->data[i];

    for (j = 0 ; j < orig->size ; j++)
      clay_array_add(tmp, orig->data[j]);

    clay_list_add(newl, tmp);
  }
  return newl;
}

/**
 * Check if the list contains the given array.  \c NULL list contains nothing.
 * \c NULL array is not contained in any list.
 * \param[in]   list    Container list
 * \param[in]   array   Element array
 * \returns     \c 1 if the list contains the given array,
 *              \c 0 otherwise
 */
int clay_list_contains(clay_list_p list, clay_array_p array) {
  int i;
  if (list == NULL || array == NULL) {
    return 0;
  }
  for (i = 0; i < list->size; i++) {
    if (clay_beta_equals(list->data[i], array)) {
      return 1;
    }
  }
  return 0;
}

/**
 * Append elements the one list at the end of another list (concatenate).
 * \param[in,out]  list     List to which append elements
 * \param[in]      appendix List to take elements from
 */
void clay_list_cat(clay_list_t *restrict list, clay_list_t *restrict appendix) {
  if (list == NULL || appendix == NULL)
    return;
  int i;
  for (i = 0; i < appendix->size; i++) {
    clay_list_add(list, appendix->data[i]);
  }
}

/**
 * Compare two lists for equality.  Lists are equal if they have identical
 * arrays in the same order.
 * \param[in]   l1  First list
 * \param[in]   l2  Second list
 * \returns         \c 1 if lists are equal, \c 0 otherwise
 */
int clay_list_equal(clay_list_p l1, clay_list_p l2) {
  int i;

  if (l1->size != l2->size)
    return 0;

  for (i = 0; i < l1->size; i++) {
    if (!clay_array_equal(l1->data[i], l2->data[i]))
      return 0;
  }

  return 1;
}

/**
 * Get textual representaiton of a list.
 * \param[in]  list  The list
 * \returns          Textual representation {array1|array2|array3}.  Allocates
 *                   memory for the string and transfers ownership to the caller
 */
char *clay_list_string(clay_list_p list) {
  char **array_strings = (char **) malloc(list->size * sizeof(char *));
  int i;
  int length = 0;
  int watermark;
  char *string;
  char *start;

  for (i = 0; i < list->size; i++) {
    array_strings[i] = clay_array_string(list->data[i]);
    length += strlen(array_strings[i]);
  }

  length += 2 + list->size;
  string = (char *) malloc(length);
  start = string;
  watermark = length;

  snprintf(string, watermark, "{");
  string += 1;
  watermark -= 1;
  for (i = 0; i < list->size - 1; i++) {
    int current_length = strlen(array_strings[i]);
    snprintf(string, watermark, "%s,", array_strings[i]);
    watermark -= current_length;
    string += current_length;
    free(array_strings[i]);
  }
  if (list->size != 0) {
    int current_length = strlen(array_strings[list->size - 1]);
    snprintf(string, watermark, "%s}", array_strings[list->size - 1]);
    watermark -= current_length;
    string += current_length;
    free(array_strings[list->size - 1]);
  }

  return start;
}

