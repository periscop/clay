
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
#include <clay/macros.h>
#include <clay/beta.h>
#include <clay/util.h>
#include <osl/statement.h>
#include <osl/body.h>
#include <osl/extensions/scatnames.h>

/** 
 * clay_util_statement_insert_inequation function:
 * Insert a new inequation at the end of the scattering
 * \param[in,out] statement
 * \param[in] inequ              [iter1, iter2, ..., param1, param2, ..., const]
 * \param[in] nb_input_dims      Nb input dims in the array
 * \param[in] nb_params          Nb params in the array
 */
/*void clay_util_statement_insert_inequation(osl_statement_p statement,
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
*/



/** 
 * clay_util_array_output_dims_pad_zero function:
 * Pad zeros for alpha columns
 * For example if we have [i, j], the result will be [0, i, 0, j, 0]
 * \param[in,out] array
 */
void clay_util_array_output_dims_pad_zero(clay_array_p a) {
  int i;
  int end = a->size*2+1;

  for (i = a->size ; i < end ; i++)
    clay_array_add(a, 0);

  for (i = end-1 ; i >= 0 ; i--) {
    a->data[i*2+1] = a->data[i];
    a->data[i] = 0;
  }
}


/** 
 * clay_util_statement_insert_inequation function:
 * Insert a new inequation at the end of the scattering
 * The list must have less or equal than 3 arrays
 * Warning: here the output dims are complete
 *          example: if the output dims are : 0 i 0 j 0, you have
 *                   to give all these numbers
 * \param[in,out] statement
 * \param[in] inequ              {(([output, ...],) [param, ..],) [const]}
 */
void clay_util_statement_set_inequation(
                        osl_statement_p statement,
                        clay_list_p inequ) {
  
  osl_relation_p scattering = statement->scattering;
  clay_array_p arr_dims = NULL, arr_params = NULL, arr_const = NULL;
  int row = scattering->nb_rows;
  int i, j;
  int precision = scattering->precision;
  
  // insert the inequation spliting (local dims are not in the inequation)
  // (at the end)
  osl_relation_insert_blank_row(scattering, row);
  osl_int_set_si(precision, scattering->m[row], 0, 1); // type inequation

  if (inequ->size > 3) {
    CLAY_error("list with more than 3 arrays not supported");
  } else if (inequ->size == 3) {
    arr_dims   = inequ->data[0];
    arr_params = inequ->data[1];
    arr_const  = inequ->data[2];
  } else if (inequ->size == 2) {
    arr_params = inequ->data[0];
    arr_const  = inequ->data[1];
  } else {
    arr_const  = inequ->data[0];
  }

  // affects output dims
  if (inequ->size == 3) {
    i = 1;
    for (j = 0 ; j < arr_dims->size ; j++) {
      osl_int_set_si(precision,
                     scattering->m[row], i,
                     arr_dims->data[j]);
      i++;
    }
  }

  // affects parameters
  if (inequ->size >= 2) {
    i = 1 + scattering->nb_output_dims + scattering->nb_input_dims + 
        scattering->nb_local_dims;
    for (j = 0; j < arr_params->size ; j++) {
      osl_int_set_si(precision,
                     scattering->m[row], i,
                     arr_params->data[j]);
      i++;
    }
  }

  // set the constant
  if (inequ->size >= 1 && arr_const->size == 1) {
    osl_int_set_si(precision,
                   scattering->m[row], scattering->nb_columns-1,
                   arr_const->data[0]);
  }
}


/** 
 * clay_util_statement_set_vector function:
 * Set the equation on each line where the column of the output dim is
 * different of zero
 * \param[in,out] statement
 * \param[in] vector           {(([output, ...],) [param, ..],) [const]}
 * \param[in] column           column on the output dim
 */
void clay_util_statement_set_vector(
                        osl_statement_p statement,
                        clay_list_p vector, int column) {
  
  osl_relation_p scattering = statement->scattering;
  clay_array_p arr_dims = NULL, arr_params = NULL, arr_const = NULL;
  int i, j, k;
  int precision = scattering->precision;
  osl_int_p tmp;

  tmp = osl_int_malloc(precision);

  if (vector->size > 3) {
    CLAY_error("list with more than 3 arrays not supported");
  } else if (vector->size == 3) {
    arr_dims   = vector->data[0];
    arr_params = vector->data[1];
    arr_const  = vector->data[2];
  } else if (vector->size == 2) {
    arr_params = vector->data[0];
    arr_const  = vector->data[1];
  } else {
    arr_const  = vector->data[0];
  }

  // for each line where there is a number different from zero on the
  // column
  for (k = 0 ; k < scattering->nb_rows ; k++) {
    if (!osl_int_zero(precision, scattering->m[k], 1+column)) {

      // scattering = coeff_outputdim * shifting

      // affect output dims
      if (vector->size >= 3) {
        i = 1;
        for (j = 0 ; j < arr_dims->size ; j++) {
          osl_int_mul_si(precision,
                         scattering->m[k], i,
                         scattering->m[k], 1+column,
                         arr_dims->data[j]);
          i++;
        }
      }

      // here we add we the last value
      // scattering += coeff_outputdim * shifting

      // affects parameters
      if (vector->size >= 2) {
        i = 1 + scattering->nb_output_dims + scattering->nb_input_dims + 
            scattering->nb_local_dims;
        for (j = 0 ; j < arr_params->size ; j++) {
          osl_int_mul_si(precision,
                         tmp, 0,
                         scattering->m[k], 1+column,
                         arr_params->data[j]);
          osl_int_add(precision,
                      scattering->m[k], i,
                      scattering->m[k], i,
                      tmp, 0);
          i++;
        }
      }

      // set the constant
      if (vector->size >= 1 && arr_const->size == 1) {
        osl_int_mul_si(precision,
                       tmp, 0,
                       scattering->m[k], 1+column,
                       arr_const->data[0]);
        osl_int_add(precision,
                    scattering->m[k], scattering->nb_columns-1,
                    scattering->m[k], scattering->nb_columns-1,
                    tmp, 0);
      }
    }
  }
  
  osl_int_free(precision, tmp, 0);
}


/**
 * clay_util_relation_negate_row function:
 * Negate the line at `row' (doesn't affect the e/i column)
 * \param[in,out] statement
 * \param[in] row                row to negate
 */
void clay_util_relation_negate_row(osl_relation_p r, int row) {
  int i;
  int precision = r->precision;
  for (i = 1 ; i < r->nb_columns ; i++) {
    osl_int_oppose(precision, 
                   r->m[row], i,
                   r->m[row], i);
  }
  osl_int_decrement(precision, 
                    r->m[row], r->nb_columns-1,
                    r->m[row], r->nb_columns-1);
}


/**
 * clay_util_statement_insert function:
 * Insert `newstatement' before `statement', and set his beta value
 * \param[in,out] statement
 * \param[in,out] newstatement
 * \param[in] column             column on the output dim (where we want to split)
 *                               this is a `alpha column' of the 2*d+1
 * \param[in] order              new beta value
 * \return                       return statement->next (so newtstatement)
 */
osl_statement_p clay_util_statement_insert(osl_statement_p statement,
                                           osl_statement_p newstatement,
                                           int column,
                                           int order) {
  osl_relation_p scattering = statement->scattering;

  // the current statement is after the new statement
  int row = clay_relation_get_line(scattering, column);
  osl_int_set_si(scattering->precision,
                 scattering->m[row], scattering->nb_columns-1,
                 order);

  // the order is not important in the statements list
  newstatement->next = statement->next;
  statement->next = newstatement;
  statement = statement->next;

  return statement;
}


/**
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
