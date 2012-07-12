
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             util.c                                 |
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
#include <clay/array.h>
#include <clay/util.h>
#include <osl/statement.h>
#include <osl/body.h>
#include <osl/extensions/scatnames.h>

/* 
 * clay_util_statement_insert_inequation function:
 * Insert a new inequation at the end of the scattering
 * \param[in,out] statement      Satement to split
 * \param[in] inequ              [iter1, iter2, ..., param1, param2, ..., const]
 * \param[in] nb_input_dims      Nb input dims in the array
 * \param[in] nb_params          Nb params in the array
 */
void clay_util_statement_insert_inequation(osl_statement_p statement,
            clay_array_p inequ, int nb_input_dims, int nb_params) {
  
  osl_relation_p scattering = statement->scattering;
  int row = scattering->nb_rows;
  int i, j;
  int precision = scattering->precision;
  
  // insert the inequation spliting (local dims are not in the inequation)
  // (at the end)
  osl_relation_insert_blank_row(scattering, row);
  osl_int_set_si(precision, scattering->m[row], 0, 1); // type inequation
  
  // affects input_dims
  i = scattering->nb_output_dims+1;
  for (j = 0 ; j < nb_input_dims ; j++) {
    osl_int_set_si(precision,
                   scattering->m[row], i,
                   inequ->data[j]);
    i++;
  }
  // affects parameters
  i = 1 + scattering->nb_output_dims + scattering->nb_input_dims + 
      scattering->nb_local_dims;
  for (; j < nb_params + nb_input_dims ; j++) {
    osl_int_set_si(precision,
                   scattering->m[row], i,
                   inequ->data[j]);
    i++;
  }
  // set the constant
  osl_int_set_si(precision,
                 scattering->m[row], scattering->nb_columns-1,
                 inequ->data[inequ->size-1]);
}


/* 
 * clay_util_string_replace function:
 * Search and replace a string with another string , in a string
 * Minor modifications from :
 * http://www.binarytides.com/blog/str_replace-for-c/
 * \param[in] search
 * \param[in] replace
 * \param[in] subject
 */
char* clay_util_string_replace(char *search, char *replace, char *string) {
	char  *ptr = NULL , *old = NULL , *new_string = NULL ;
	int count = 0 , search_size;
	
	search_size = strlen(search);

	// Count how many occurences
	for(ptr = strstr(string , search) ; ptr != NULL ; 
	    ptr = strstr(ptr + search_size , search)) {
		count++;
	}
	
	// Final size
	count = (strlen(replace) - search_size)*count + strlen(string) + 1;
	new_string = calloc(count, 1);

	// The start position
	old = string;

	for(ptr = strstr(string, search) ; ptr != NULL ;
	    ptr = strstr(ptr + search_size, search)) {
		// move ahead and copy some text from original subject , from a
		// certain position
		strncpy(new_string + strlen(new_string), old , ptr - old);

		// move ahead and copy the replacement text
		strcpy(new_string + strlen(new_string) , replace);

		// The new start position after this search match
		old = ptr + search_size;
	}

	// Copy the part after the last search match
	strcpy(new_string + strlen(new_string) , old);

	return new_string;
}


/**
 * clay_util_scatnames_exists_iterator_iterator function:
 * Return true if the iterator name is already in the scattering.
 * \param[in] scattering
 * \return
 */
bool clay_util_scatnames_exists(osl_scatnames_p scatnames, char *iter) {
  osl_strings_p names = scatnames->names;
  if (names == NULL || names->string[0] == NULL)
    return 0;
  
  char **ptr = names->string;
  
  while (*ptr != NULL) {
    if (strcmp(*ptr, iter) == 0)
      return 1;
    ptr++;
  }
  
  return 0;
}


/**
 * clay_util_statement_find_iterator function:
 * Return the index if iter is found in the original iterators list.
 * \param[in] scop
 * \param[in] iter       name of the original iterator we want to search
 * \return
 */
int clay_util_statement_find_iterator(osl_statement_p statement, char *iter) {
  osl_body_p body = (osl_body_p) statement->body->data;
  char **ptr = body->iterators->string;
  int i = 0;
  
  while (*ptr != NULL) {
    if (strcmp(*ptr, iter) == 0)
      return i;
    ptr++;
    i++;
  }
  
  return -1;
}
