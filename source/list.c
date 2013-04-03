
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
#include <clay/macros.h>
#include <clay/list.h>


clay_list_p clay_list_malloc() {
  clay_list_p l;
  CLAY_malloc(l, clay_list_p, sizeof(clay_list_t));
  CLAY_malloc(l->data, clay_array_p*, sizeof(clay_array_p) * CLAY_LIST_INIT_SIZE);
  l->size = 0;
  l->available = CLAY_ARRAY_INIT_SIZE;
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
