
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             data.c                                 |
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
#include <string.h>
#include <clay/macros.h>
#include <clay/data.h>
#include <clay/array.h>
#include <clay/list.h>

clay_data_p clay_data_malloc(int type) {
  clay_data_p tmp = malloc(sizeof(clay_data_t));
  tmp->type = type;
  return tmp;
}

void clay_data_free(clay_data_p d) {
  if (!d)
    return;
  clay_data_clear(d);
  free(d);
}


void clay_data_clear(clay_data_p d) {
  switch (d->type) {
    case INTEGER_T:
    case VOID_T:
    case REF_T:
    case UNDEF_T:
      break;

    case ARRAY_T:
      clay_array_free((clay_array_p) d->data.obj);
      break;

    case LIST_T:
      clay_list_free((clay_list_p) d->data.obj);
      break;

    case STRING_T:
      free(d->data.obj);
      break;
  }
}


void clay_data_print(FILE* f, clay_data_p d) {
  if (d == NULL)
    fprintf(f, "NULL\n");

  switch (d->type) {
    case UNDEF_T:
      fprintf(f, "undef\n");
      break;

    case REF_T:
      fprintf(f, "ref ");
      clay_data_print(f, d->data.obj);
      break;

    case INTEGER_T:
      fprintf(f, "int %d\n", d->data.integer);
      break;

    case VOID_T:
      fprintf(f, "void %d\n", d->data.integer);
      break;

    case ARRAY_T:
      fprintf(f, "array ");
      clay_array_print(f, d->data.obj, 1);
      break;

    case LIST_T:
      fprintf(f, "list ");
      clay_list_print(f, d->data.obj);
      break;

    case STRING_T:
      fprintf(f, "string %s\n", (char*) d->data.obj);
      break;
  }
}


clay_data_p clay_data_clone(clay_data_p d) {
  if (d == NULL)
    return NULL;

  clay_data_p newd = clay_data_malloc(d->type);

  switch (d->type) {
    case UNDEF_T:
    case VOID_T:
      break;

    case REF_T:
      newd->data.obj = clay_data_clone(d->data.obj);
      break;

    case INTEGER_T:
      newd->data.integer = d->data.integer;
      break;

    case ARRAY_T:
      newd->data.obj = clay_array_clone(d->data.obj);
      break;

    case LIST_T:
      newd->data.obj = clay_list_clone(d->data.obj);
      break;

    case STRING_T:
      CLAY_strdup(newd->data.obj, (char*) d->data.obj);
      break;
  }

  return newd;
}
