
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
#include <clay/beta.h>
#include <clay/macros.h>
#include <clay/util.h>
#include <clay/errors.h>
#include <clay/relation.h>

#include <osl/statement.h>
#include <osl/body.h>
#include <osl/extensions/extbody.h>
#include <osl/extensions/scatnames.h>
#include <osl/scop.h>
#include <osl/generic.h>
#include <osl/util.h>
#include <osl/relation.h>
#include <osl/relation_list.h>
#include <osl/macros.h>

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
  int old_size = a->size;

  for (i = old_size ; i < end ; i++)
    clay_array_add(a, 0);

  for (i = old_size - 1 ; i >= 0 ; i--) {
    a->data[i*2+1] = a->data[i];
    a->data[i] = 0;
  }
}

// update the provided part of the beta and leave everything else untouched
void clay_util_scattering_update_beta(osl_relation_p scattering,
                                      clay_array_p beta) {
  int i, row;
  if (!scattering || !beta || beta->size == 0) {
    return;
  }

  for (i = 0; i < OSL_min(beta->size, scattering->nb_output_dims / 2 + 1); i++) {
    row = clay_util_relation_get_line(scattering, i * 2);
    osl_int_set_si(scattering->precision,
                   &scattering->m[row][scattering->nb_columns - 1],
                   beta->data[i]);
  }
}

// update the provided part of the beta for every part of the scattering relation union
// that matches the given old beta
void clay_util_statement_replace_beta(osl_statement_p statement,
                                      clay_array_p old_beta,
                                      clay_array_p beta) {
  osl_relation_p scattering = statement->scattering;
  while (scattering != NULL) {
    if (clay_beta_check_relation(scattering, old_beta)) {
      clay_util_scattering_update_beta(scattering, beta);
    }
    scattering = scattering->next;
  }
}

/**
 * Appends an inequality row to the relation with values coming from the list
 * grouped by their type (output dimensions, input dimensions, parameters,
 * constant; in this order).  If first elements in the list are missing, they
 * are assumed to be zero, for example a list with one array should contains
 * only a constant.  If any of the arrays in the list is shorter than the
 * corresponding dimensionality, missing last values are assumed to be zero,
 * for example an array [1,2,3] will be interpreted as [1,2,3,0,0] if five
 * dimensions are required.
 * \param[in,out] relation pointer to the relation that will include the row
 * \param[in]     inequ    coefficients in the row
 */
void clay_util_relation_insert_inequation(osl_relation_p relation,
                                          clay_list_p inequ) {
  clay_array_p dims = NULL,
               input_dims = NULL,
               params = NULL,
               consts = NULL;
  int row = relation->nb_rows;
  int precision = relation->precision;
  int i, j;

  // Insert a blank row at the end and fill it in.
  osl_relation_insert_blank_row(relation, row);
  osl_int_set_si(precision, &relation->m[row][0], 1); // inequality

  if (inequ->size > 4) {
    CLAY_error("wrong coefficient list inserted (size > 4)");
  } else if (inequ->size == 0) {
    CLAY_error("empty coefficient list inserted");
  } else if (inequ->size == 4) {
    dims       = inequ->data[0];
    input_dims = inequ->data[1];
    params     = inequ->data[2];
    consts     = inequ->data[3];
  } else if (inequ->size == 3) {
    input_dims = inequ->data[0];
    params     = inequ->data[1];
    consts     = inequ->data[2];
  } else if (inequ->size == 2) {
    params     = inequ->data[0];
    consts     = inequ->data[1];
  } else {
    consts     = inequ->data[0];
  }

  if (consts->size != 1) {
    CLAY_error("row insertion: constant must be a single value");
  }

  // Decomposition switch, fallthoroughs are _intentional_.
  switch (inequ->size) {
  case 4:
    for (j = 0, i = 1; j < dims->size; j++, i++) {
      osl_int_set_si(precision,
                     &relation->m[row][i],
                     dims->data[j]);
    }
    // intentional fall through
  case 3:
    for (j = 0, i = 1 + relation->nb_output_dims;
         j < input_dims->size;
         j++, i++) {
      osl_int_set_si(precision,
                     &relation->m[row][i],
                     input_dims->data[j]);
    }
    // intentional fall through
  case 2:
    for (j = 0, i = 1 + relation->nb_output_dims +
                    relation->nb_input_dims + relation->nb_local_dims;
        j < params->size; 
        j++, i++) {
      osl_int_set_si(precision,
                     &relation->m[row][i],
                     params->data[j]);
    }
    // intentional fall through
  case 1:
    osl_int_set_si(precision,
                   &relation->m[row][relation->nb_columns-1],
                   consts->data[0]);
    break;
  default:
    CLAY_error("list with more than 3 arrays not supported");
    break;
  }
}

/** 
 * clay_util_statement_set_inequation function:
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
  osl_int_set_si(precision, &scattering->m[row][0], 1); // type inequation

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
                     &scattering->m[row][i],
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
                     &scattering->m[row][i],
                     arr_params->data[j]);
      i++;
    }
  }

  // set the constant
  if (inequ->size >= 1 && arr_const->size == 1) {
    osl_int_set_si(precision,
                   &scattering->m[row][scattering->nb_columns-1],
                   arr_const->data[0]);
  }
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
                   &r->m[row][i],
                   r->m[row][i]);
  }
  osl_int_decrement(precision, 
                    &r->m[row][r->nb_columns-1],
                    r->m[row][r->nb_columns-1]);
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
  int row = clay_util_relation_get_line(scattering, column);
  osl_int_set_si(scattering->precision,
                 &scattering->m[row][scattering->nb_columns-1],
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
int clay_util_scatnames_exists(osl_scatnames_p scatnames, char *iter) {
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
  osl_body_p body;
  osl_extbody_p extbody = NULL;
  
  extbody = osl_generic_lookup(statement->extension, OSL_URI_EXTBODY);
  if (extbody)
    body = extbody->body;
  else
    body = osl_generic_lookup(statement->extension, OSL_URI_BODY);

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


/**
 * clay_util_scop_export_body function:
 * Convert each extbody to a body structure
 * \param[in] scop
 */
void clay_util_scop_export_body(osl_scop_p scop) {
  if (scop == NULL)
    return;

  osl_statement_p stmt = scop->statement;
  osl_extbody_p ebody = NULL;
  osl_body_p body = NULL;
  osl_generic_p gen = NULL;

  while (stmt) {
    ebody = osl_generic_lookup(stmt->extension, OSL_URI_EXTBODY);
    if (ebody!=NULL) {

      body = osl_generic_lookup(stmt->extension, OSL_URI_BODY);
      if (body) {
        osl_generic_remove(&stmt->extension, OSL_URI_BODY);
      }
      body = osl_body_clone(ebody->body);
      gen = osl_generic_shell(body, osl_body_interface());
      osl_generic_add(&stmt->extension, gen);
      osl_generic_remove(&stmt->extension, OSL_URI_EXTBODY);
      ebody=NULL;
      body=NULL;
    }
    stmt = stmt->next;
  }
}


void static clay_util_name_sprint(char **dst, size_t *hwm,
                                  int *print_plus, int val, char *name) {
  if (*print_plus)
    osl_util_safe_strcat(dst, " + ", hwm);
  else
    *print_plus = 1;

  char buffer[32];

  if (name == NULL) {
    snprintf(buffer, 32, "%d", val);
    osl_util_safe_strcat(dst, buffer, hwm);
  } else {
    if (val == 1) {
      osl_util_safe_strcat(dst, name, hwm);
    } else if (val == -1) {
      osl_util_safe_strcat(dst, "-", hwm);
      osl_util_safe_strcat(dst, name, hwm);
    } else {
      snprintf(buffer, 32, "%d*", val);
      osl_util_safe_strcat(dst, buffer, hwm);
      osl_util_safe_strcat(dst, name, hwm);
    }
  }
}


/**
 * clay_util_body_regenerate_access function:
 * Read the access array and re-generate the code in the body
 * \param[in] ebody     An extbody structure
 * \param[in] access    The relation to regenerate the code
 * \param[in] index     nth access (needed to access to the array start and 
 *                      length of the extbody structure)
 */
void clay_util_body_regenerate_access(osl_extbody_p ebody,
                                      osl_relation_p access,
                                      int index,
                                      osl_arrays_p arrays,
                                      osl_scatnames_p scatnames,
                                      osl_strings_p params) {

  if (!arrays || !scatnames || !params || access->nb_output_dims == 0 ||
      index >= ebody->nb_access)
    return;

  const int precision = access->precision;
  int i, j, k, row, val, print_plus;

  // check if there are no inequ
  for (i = 0 ; i < access->nb_rows ; i++) {
    if (!osl_int_zero(precision, access->m[i][0]))
      CLAY_error("I don't know how to regenerate access with inequalities");
  }

  // check identity matrix in output dims
  int n;
  for (j = 0 ; j < access->nb_output_dims ; j++) {
    n = 0;
    for (i = 0 ; i < access->nb_rows ; i++)
      if (!osl_int_zero(precision, access->m[i][j+1])) {
        if (n >= 1)
          CLAY_error("I don't know how to regenerate access with "
                     "dependences in output dims");
        n++;
      }
  }

  char *body = ebody->body->expression->string[0];
  int body_len = strlen(body);
  int start = ebody->start[index];
  int len = ebody->length[index];
  int is_zero; // if the line contains only zeros

  if (start >= body_len || start + len >= body_len || (start == -1 && len == -1))
    return;

  char *new_body;
  char end_body[OSL_MAX_STRING];
  size_t hwm = OSL_MAX_STRING;

  CLAY_malloc(new_body, char *, OSL_MAX_STRING * sizeof(char));

  // copy the beginning of the body
  if (start+1 >= OSL_MAX_STRING)
    CLAY_error("memcpy: please recompile osl with a higher OSL_MAX_STRING");
  memcpy(new_body, body, start);
  new_body[start] = '\0';

  // save the end in a buffer
  int sz = body_len - start - len;
  if (sz + 1 >= OSL_MAX_STRING)
    CLAY_error("memcpy: please recompile osl with a higher OSL_MAX_STRING");
  memcpy(end_body, body + start + len, sz);
  end_body[sz] = '\0';


  // copy access name string
  val = osl_relation_get_array_id(access);
  val = clay_util_arrays_search(arrays, val); // get the index in th array
  osl_util_safe_strcat(&new_body, arrays->names[val], &hwm);


  // generate each dims
  for (i = 1 ; i < access->nb_output_dims ; i++) {
    row = clay_util_relation_get_line(access, i);
    if (row == -1)
      continue;

    osl_util_safe_strcat(&new_body, "[", &hwm);

    is_zero = 1;
    print_plus = 0;
    k = 1 + access->nb_output_dims;

    // iterators
    for (j = 0 ; j < access->nb_input_dims ; j++, k++) {
      val = osl_int_get_si(precision, access->m[row][k]);
      if (val != 0) {
        clay_util_name_sprint(&new_body,
                              &hwm, 
                              &print_plus,
                              val,
                              scatnames->names->string[j*2+1]);
        is_zero = 0;
      }
    }

    // params
    for (j = 0 ; j < access->nb_parameters ; j++, k++) {
      val = osl_int_get_si(precision, access->m[row][k]);
      if (val != 0) {
        clay_util_name_sprint(&new_body,
                              &hwm, 
                              &print_plus,
                              val,
                              params->string[j]);
        is_zero = 0;
      }
    }

    // const
    val = osl_int_get_si(precision, access->m[row][k]);
    if (val != 0 || is_zero)
      clay_util_name_sprint(&new_body,
                            &hwm, 
                            &print_plus,
                            val,
                            NULL);

    osl_util_safe_strcat(&new_body, "]", &hwm);
  }

  // length of the generated access
  ebody->length[index] = strlen(new_body) - start;

  // concat the end
  osl_util_safe_strcat(&new_body, end_body, &hwm);

  // update ebody
  free(ebody->body->expression->string[0]);
  ebody->body->expression->string[0] = new_body;

  // shift the start
  int diff = ebody->length[index] - len;
  for (i = index+1 ; i < ebody->nb_access ; i++)
    if (ebody->start[i] != -1)
      ebody->start[i] += diff;
}


/**
 * clay_util_arrays_search function:
 * Search the string which corresponds to id
 * arrays is an extension of osl
 * \param[in] arrays    An arrays osl structure
 * \param[in] id        The id to search
 * \return              Return the index in the arrays 
 */
int clay_util_arrays_search(osl_arrays_p arrays, unsigned int id) {
  int i;
  for (i = 0 ; i < arrays->nb_names ; i++) {
    if (arrays->id[i] == id)
      return i;
  }
  return -1;
}


/**
 * clay_util_foreach_access function:
 * Execute func on each access which corresponds to access_name
 * \param[in,out] scop
 * \param[in] beta
 * \param[in] access_name      The id to search
 * \param[in] func             The function to execute for each access
 *                             The function takes an osl_relation_list_p in 
 *                             parameter (the elt can be modified) and must 
 *                             return a define error or CLAY_SUCCESS
 * \param[in] args             args of `func'
 * \param[in] regenerate_body  If 1: after each call to func, 
 *                             clay_util_body_regenerate_access is also called
 * \return                     Return a define error or CLAY_SUCCESS
 */
int clay_util_foreach_access(osl_scop_p scop,
                             clay_array_p beta,
                             unsigned int access_name,
                             int (*func)(osl_relation_list_p, void*),
                             void *args,
                             int regenerate_body) {

  osl_statement_p stmt = scop->statement;
  osl_relation_list_p access;
  osl_relation_p a;
  osl_extbody_p ebody = NULL;
  osl_body_p body = NULL;
  osl_generic_p gen= NULL;
  int count_access;
  int found = 0;
  int ret;

  // TODO : global vars ?
  osl_arrays_p arrays;
  osl_scatnames_p scatnames;
  osl_strings_p params;
  arrays = osl_generic_lookup(scop->extension, OSL_URI_ARRAYS);
  scatnames = osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
  params = osl_generic_lookup(scop->parameters, OSL_URI_STRINGS);

  if (!arrays || !scatnames || !params)
    CLAY_warning("no arrays or scatnames extension");

  stmt = clay_beta_find(scop->statement, beta);
  if (!stmt)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // for each access in the beta, we search the access_name
  while (stmt != NULL) {
    if (clay_beta_check(stmt, beta)) {
      access = stmt->access;
      count_access = 0;

      while (access) {
        a = access->elt;

        if (osl_relation_get_array_id(a) == access_name) {
          found = 1;

          ebody = osl_generic_lookup(stmt->extension, OSL_URI_EXTBODY);
          if (ebody==NULL) {
            CLAY_error("extbody uri not found on this statement");
            fprintf(stderr, "%s\n",
              ebody->body->expression->string[0]);
          }

          // call the function
          ret = (*func)(access, args);
          if (ret != CLAY_SUCCESS) {
            fprintf(stderr, "%s\n",
              ebody->body->expression->string[0]);
            return ret;
          }

          // re-generate the body
          if (regenerate_body) {
            clay_util_body_regenerate_access(
                ebody,
                access->elt,
                count_access,
                arrays,
                scatnames,
                params);


            //synchronize extbody with body
            body = osl_generic_lookup(stmt->extension, OSL_URI_BODY);
            if (body) {
              osl_generic_remove(&stmt->extension, OSL_URI_BODY);
              body = osl_body_clone(ebody->body);
              gen = osl_generic_shell(body, osl_body_interface());
              osl_generic_add(&stmt->extension, gen);
            }
          }
        }

        ebody = NULL;
        body  = NULL;
        access = access->next;
        count_access++;
      }
    }
    stmt = stmt->next;
  }

  if (!found)
   fprintf(stderr,"[Clay] Warning: access number %d not found\n", access_name);

  return CLAY_SUCCESS;
}


/**
 * clay_util_relation_get_line function:
 * Because the lines in the scattering matrix may have not ordered, we have to
 * search the corresponding line. It returns the first line where the value is
 * different from zero in the `column'. `column' is between 0 and 
 * nb_output_dims-1
 * \param[in] relation
 * \param[in] column        Line to search
 * \return                  Return the real line
 */
int clay_util_relation_get_line(osl_relation_p relation, int column) {
  if (column < 0 || column > relation->nb_output_dims)
    return -1;
  int i;
  int precision = relation->precision;
  for (i = 0 ; i < relation->nb_rows ; i++) {
    if (!osl_int_zero(precision, relation->m[i][column+1])) {
      break;
    }
  }
  return (i == relation->nb_rows ? -1 : i );
}

int clay_util_is_row_beta_definition(osl_relation_p relation, int row) {
  int precision = relation->precision;
  if (relation->nb_columns < 1)
    return 0;
  if (row >= relation->nb_rows || row < 0)
    return 0;
  // Lines that define beta dimensions have a form of -1*beta_i + constant = 0.
  // 1. Is an equality
  if (!osl_int_zero(precision, relation->m[row][0]))
    return 0;
  // 2. Has only one non-zero coefficient for beta dims, equal to -1.
  int idx = -1;
  for (int i = 0; i < relation->nb_output_dims; i += 2) {
    if (idx == -1) {
      if (osl_int_mone(precision, relation->m[row][1 + i])) {
        idx = i;
      } else if (!osl_int_zero(precision, relation->m[row][1 + i])) {
        return 0;
      }
    } else {
      if (!osl_int_zero(precision, relation->m[row][1 + i])) {
        return 0;
      }
    }
  }
  // 3. Has all 0 coefficients for alpha dims.
  for (int i = 2; i < relation->nb_output_dims; i += 2) {
    if (!osl_int_zero(precision, relation->m[row][i]))
      return 0;
  }
  // 4. Input dims and parameters are 0, constant may be anything.
  for (int i = 1 + relation->nb_output_dims; i < relation->nb_columns - 1; i++) {
    if (!osl_int_zero(precision, relation->m[row][i]))
      return 0;
  }
  return 1;
}

void clay_alpha_normalize(osl_scop_p scop) {
  osl_statement_p statement;
  osl_relation_p scattering;

  if (!scop || !scop->statement)
    return;

  for (statement = scop->statement; statement != NULL;
       statement = statement->next) {
    for (scattering = statement->scattering; scattering != NULL;
         scattering = scattering->next) {
      clay_relation_normalize_alpha(scattering);
    }
  }
}

void clay_scop_normalize(osl_scop_p scop) {
  clay_alpha_normalize(scop);
  clay_beta_normalize(scop);
}
