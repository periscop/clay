
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                           transformation.c                         |
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
#include <osl/macros.h>
#include <osl/scop.h>
#include <osl/body.h>
#include <osl/strings.h>
#include <osl/util.h>
#include <osl/statement.h>
#include <osl/relation.h>
#include <osl/extensions/scatnames.h>
#include <clay/transformation.h>
#include <clay/array.h>
#include <clay/macros.h>
#include <clay/options.h>
#include <clay/errors.h>
#include <clay/beta.h>
#include <clay/util.h>


/*****************************************************************************\
 *                     Loop transformations                                   *
 `****************************************************************************/


/**
 * clay_reorder function:
 * Reorders the statements in the loop
 * \param[in,out] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] order         Array to reorder the statements
 * \param[in] options
 * \return                  Status
 */
int clay_reorder(osl_scop_p scop, 
                  clay_array_p beta_loop, clay_array_p neworder,
                  clay_options_p options) {

  /* Description:
   * Modify the beta in function of the values in the neworder array.
   */

  osl_relation_p scattering;
  osl_statement_p statement;
  int precision;
  const int column = beta_loop->size * 2; // transition column
  int row;
  int index;
  
  // important to be sure that the scop normalized first
  // because we access to the reorder array in function of the beta value
  clay_beta_normalize(scop);
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_ERROR_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
    
      scattering = statement->scattering;
      row = clay_relation_get_line(scattering, column);
      
      // get the beta value
      index = osl_int_get_si(precision,
                             scattering->m[row],
                             scattering->nb_columns-1);
      
      if (index >= neworder->size)
        return CLAY_ERROR_REORDER_ARRAY_TOO_SMALL;
      
      osl_int_set_si(precision, 
                     scattering->m[row], scattering->nb_columns-1,
                     neworder->data[index]);
    }
    statement = statement->next;
  }
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_reverse function:
 * Reverse the direction of the loop
 * \param[in,out] scop
 * \param[in] beta          Beta vector
 * \param[in] options
 * \return                  Status
 */
int clay_reverse(osl_scop_p scop, clay_array_p beta, int depth,
                 clay_options_p options) {
 
  /* Description:
   * Oppose the output_dims column at the `depth'th level.
   */
  
  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (depth <= 0) 
    return CLAY_ERROR_DEPTH_OVERFLOW;
    
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  int column = depth*2 - 1; // iterator column
  int i;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (beta->size*2-1 >= statement->scattering->nb_output_dims && 
      depth >= beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  // else it's a loop, and the depth must be less or equal than the beta size
  if (depth > beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;

  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      for(i = 0 ; i < scattering->nb_rows ; i++) {
        osl_int_oppose(precision, 
                       scattering->m[i], column+1,
                       scattering->m[i], column+1);
      }
    }
    statement = statement->next;
  }
  
  return CLAY_SUCCESS;
}


/**
 * clay_interchange function:
 * On each statement which belongs to the `beta', the loops that match the
 * `depth_1'th and the `depth_2' are interchanged
 * /!\ If you want to interchange 2 loops, you must give the inner beta loop
 * and not the outer !
 * \param[in,out] scop
 * \param[in] beta          Beta vector (inner loop or statement)
 * \param[in] depth_1       >= 1
 * \param[in] depth_2       >= 1
 * \param[in] options
 * \return                  Status
 */
int clay_interchange(osl_scop_p scop, 
                      clay_array_p beta, int depth_1, int depth_2,
                      clay_options_p options) {
  /* Description:
   * Swap the two output_dims columns.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (depth_1 <= 0 || depth_2 <= 0) 
    return CLAY_ERROR_DEPTH_OVERFLOW;

  osl_statement_p statement = scop->statement;
  osl_relation_p scattering;
  int precision;
  const int column_1 = depth_1*2 - 1; // iterator column
  const int column_2 = depth_2*2 - 1;
  int i;
  void **matrix;
  //int nb_rows;

  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (statement->scattering->nb_output_dims == 1)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  
  // if it's a statement, the depth must be strictly less than the beta size
  if (beta->size*2-1 >= statement->scattering->nb_output_dims && 
      (depth_1 >= beta->size || 
      depth_2 >= beta->size))
    return CLAY_ERROR_DEPTH_OVERFLOW;
  // else it's a loop, and the depth must be less or equal than the beta size
  if (depth_1 > beta->size || depth_2 > beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  // it's not useful to interchange the same line
  if (depth_1 == depth_2)
    return CLAY_SUCCESS;
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
  
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;

      //nb_rows = scattering->nb_rows;
      //if (column_1 >= nb_rows || column_2 >= nb_rows)
      //  return CLAY_DEPTH_OVERFLOW;
      
      // swap the two columns
      matrix = scattering->m;
      for (i = 0 ; i < scattering->nb_rows ; i++) {
        osl_int_swap(precision, 
                     matrix[i], column_1+1,
                     matrix[i], column_2+1);
      }
    }
    
    statement = statement->next;
  }
  return CLAY_SUCCESS;
}


/**
 * clay_split function:
 * Split the loop in two parts at the `depth'th level from the statement
 * \param[in,out] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >= 1
 * \param[in] options
 * \return                  Status
 */
int clay_split(osl_scop_p scop, clay_array_p beta, int depth,
               clay_options_p options) {

  /* Description:
   * Add one on the beta at the `depth'th level.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (beta->size <= 1 || depth <= 0 || depth >= beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  
  osl_statement_p statement = scop->statement;
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  
  clay_beta_shift_after(scop->statement, beta, depth);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_fuse function:
 * Fuse loop with the first loop after
 * \param[in,out] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] options
 * \return                  Status
 */
int clay_fuse(osl_scop_p scop, clay_array_p beta_loop,
              clay_options_p options) {

  /* Description:
   * Set the same beta (only at the level of the beta_loop) on the next loop.
   */

  if (beta_loop->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
 
  osl_relation_p scattering;
  osl_statement_p statement;
  clay_array_p beta_max;
  clay_array_p beta_next;
  int precision;
  const int depth = beta_loop->size;
  const int column = beta_loop->size*2; // transition column
  int row;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_ERROR_NOT_BETA_LOOP;

  precision = statement->scattering->precision;
  
  beta_max = clay_beta_max(statement, beta_loop);
  beta_next = clay_beta_next_part(scop->statement, beta_loop);
  
  if (beta_next != NULL) {
    beta_next->size = depth;
    statement = scop->statement;
    while (statement != NULL) {
      if (clay_beta_check(statement, beta_next)) {
        scattering = statement->scattering;
        if (column < scattering->nb_output_dims) {
        
          // Set the loop level
          row = clay_relation_get_line(scattering, column-2);
          osl_int_set_si(precision, 
                         scattering->m[row], scattering->nb_columns-1, 
                         beta_loop->data[depth-1]);

          // Reorder the statement
          row = clay_relation_get_line(scattering, column);
          osl_int_add_si(precision,
                         scattering->m[row], scattering->nb_columns-1,
                         scattering->m[row], scattering->nb_columns-1,
                         beta_max->data[depth]+1);
        }
      }
      statement = statement->next;
    }
    clay_array_free(beta_next);
  }
  
  clay_array_free(beta_max);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_skew function:
 * Skew the loop (or statement) from the `depth'th loop
 * (i, j) -> (i, j+i*coeff) where `depth' is the loop of i
 * \param[in,out] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >= 1
 * \param[in] coeff         != 0
 * \param[in] options
 * \return                  Status
 */
int clay_skew(osl_scop_p scop, 
              clay_array_p beta, int depth, int coeff,
              clay_options_p options) {

  /* Description:
   * Add a dependance (with the loop at the `depth' level) on an output_dim
   * column.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (depth <= 0)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  if (coeff == 0)
    return CLAY_ERROR_WRONG_COEFF;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column_depth = depth*2 - 1; // iterator column
  int column_beta; // iterator column
  int row;
  int iter_column;
  void **matrix;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  
  if (statement->scattering->nb_output_dims == 1)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  // if it's a statement, the depth must be strictly less than the beta size
  if (beta->size*2-1 >= statement->scattering->nb_output_dims) {
      if (depth >= beta->size)
        return CLAY_ERROR_DEPTH_OVERFLOW;
      if (depth == beta->size-1) // TODO no sense to skew at the same level ?
        return CLAY_SUCCESS;
      column_beta = beta->size*2 - 3;
  }
  // else it's a loop and if the depth is the same as the beta, we change nothing
  else {
    if (depth > beta->size)
      return CLAY_ERROR_DEPTH_OVERFLOW;
    if (depth == beta->size)
      return CLAY_SUCCESS;
    column_beta = beta->size*2 - 1;
  }
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (column_depth >= scattering->nb_output_dims)
        return CLAY_ERROR_DEPTH_OVERFLOW;
      
      matrix = scattering->m;
      iter_column = depth + scattering->nb_output_dims;
      
      row = clay_relation_get_line(scattering, column_beta);
      osl_int_add_si(precision, 
                     matrix[row], iter_column,
                     matrix[row], iter_column,
                     coeff);
    }
    statement = statement->next;
  }
  return CLAY_SUCCESS;
}


/**
 * clay_iss function:
 * Split the loop (or statement) depending of an inequation.
 * \param[in,out] scop
 * \param[in] beta              Beta vector (loop or statement)
 * \param[in] inequation array  [iter1, iter2, ..., param1, param2, ..., const]
 * \param[in] options
 * \return                      Status
 */
int clay_iss(osl_scop_p scop, 
             clay_array_p beta, clay_array_p inequ,
             clay_options_p options) {

  /* Description:
   * Add the inequality for each statements corresponding to the beta.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (inequ->size == 0)
    return CLAY_SUCCESS;
  
  osl_relation_p scattering;
  osl_statement_p statement;
  osl_statement_p newstatement;
  const int column = (beta->size-1)*2;
  int precision;
  int row;
  int j;
  int inequ_nb_input_dims, inequ_nb_parameters;
  void *order; // new loop order for the clones
  
  // search a statement
  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (statement->scattering->nb_input_dims == 0)
    return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
  
  precision = statement->scattering->precision;
  
  // decompose the inequation
  inequ_nb_parameters = scop->context->nb_parameters;
  inequ_nb_input_dims = inequ->size - 1 - inequ_nb_parameters;
  
  if (beta->size*2-1 == statement->scattering->nb_output_dims) {
    if (inequ_nb_input_dims != beta->size-1)
      return CLAY_ERROR_INEQU;
  } else {
    if (inequ_nb_input_dims != beta->size)
      return CLAY_ERROR_INEQU;
  }

  // let the place for the splitted loop
  clay_beta_shift_after(scop->statement, beta, beta->size);

  // init new order value for the new loop
  scattering = statement->scattering;
  row = clay_relation_get_line(scattering, column); // parent loop line
  order = osl_int_malloc(precision);
  osl_int_assign(precision,
                 order, 0,
                 scattering->m[row], scattering->nb_columns-1);
  osl_int_increment(precision,
                    order, 0,
                    order, 0);
  
  // set the inequation
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (inequ->size <= 1 + scattering->nb_input_dims + 
                                                    scattering->nb_parameters) {

        clay_util_statement_insert_inequation(statement, inequ,
                  inequ_nb_input_dims, inequ_nb_parameters);
        
        row = scattering->nb_rows-1; // last inserted line
        
        // create a new statement with the negation of the inequation
        newstatement = osl_statement_nclone(statement, 1);
        scattering = newstatement->scattering;
        for (j = scattering->nb_output_dims ; j < scattering->nb_columns ; j++) {
          osl_int_oppose(precision, 
                         scattering->m[row], j,
                         scattering->m[row], j);
        }
        osl_int_decrement(precision, 
                          scattering->m[row], scattering->nb_columns-1,
                          scattering->m[row], scattering->nb_columns-1);

        // the current statement is after the new statement
        scattering = statement->scattering;
        row = clay_relation_get_line(scattering, column);
        osl_int_assign(precision,
                 scattering->m[row], scattering->nb_columns-1,
                 order, 0);
  
        // the order is not important in the statements list
        newstatement->next = statement->next;
        statement->next = newstatement;
        statement = statement->next;
      }
    }
    statement = statement->next;
  }
  
  osl_int_free(precision, order, 0);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
    
  return CLAY_SUCCESS;
}


/**
 * clay_stripmine function:
 * Decompose a single loop into two nested loop
 * \param[in,out] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] size          Block size of the inner loop
 * \param[in] pretty        If true, clay will keep the variables name
 * \param[in] options
 * \return                  Status
 */
int clay_stripmine(osl_scop_p scop, clay_array_p beta, int depth, int size, 
                   int pretty, clay_options_p options) {
  
  /* Description:
   * Add two inequality for the stripmining.
   * The new dependance with the loop is done on an output_dim.
   * If pretty is true, we add for each statements (not only the beta) two
   * output_dims (one for the iterator and one for the 2*n+1).
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (size <= 0)
    return CLAY_ERROR_WRONG_BLOCK_SIZE;
  if (depth <= 0)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  osl_scatnames_p scat;
  osl_strings_p names;
  int column = (depth-1)*2;
  int precision;
  int row, row_next;
  int i;
  int nb_strings;
  char buffer[OSL_MAX_STRING];
  char *new_var_iter;
  char *new_var_beta;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (statement->scattering->nb_output_dims < 3)
    return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
  
  if (beta->size*2-1 >= statement->scattering->nb_output_dims && 
      depth >= beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  // else it's a loop, and the depth must be less or equal than the beta size
  if (depth > beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  
  precision = statement->scattering->precision;
  if (pretty)
    statement = scop->statement; // TODO optimization...
        
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    scattering = statement->scattering;
    if (clay_beta_check(statement, beta)) {
      
      // set the strip mine
      row = clay_relation_get_line(scattering, column);
      
      osl_relation_insert_blank_column(scattering, column+1);
      osl_relation_insert_blank_column(scattering, column+1);

      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      
      // stripmine
      osl_int_set_si(precision, scattering->m[row+0], column+1, -1);
      osl_int_set_si(precision, scattering->m[row+1], column+2, -size);
      osl_int_set_si(precision, scattering->m[row+2], column+2, size);
      osl_int_set_si(precision, scattering->m[row+2], scattering->nb_columns-1, 
                     size-1);
      
      // inequality
      osl_int_set_si(precision, scattering->m[row+1], 0, 1);
      osl_int_set_si(precision, scattering->m[row+2], 0, 1);
      
      // iterator dependance
      osl_int_set_si(precision, scattering->m[row+1], column+4, 1);
      osl_int_set_si(precision, scattering->m[row+2], column+4, -1);
      
      scattering->nb_output_dims += 2;
      
      // reorder
      row_next = clay_relation_get_line(scattering, column+2);
      osl_int_assign(precision, scattering->m[row], scattering->nb_columns-1,
                     scattering->m[row_next], scattering->nb_columns-1);
      
      osl_int_set_si(precision, scattering->m[row_next],
                     scattering->nb_columns-1, 0);
    
    } else if (pretty && column < scattering->nb_output_dims) {
      // add 2 empty dimensions
      row = clay_relation_get_line(scattering, column);
      
      osl_relation_insert_blank_column(scattering, column+1);
      osl_relation_insert_blank_column(scattering, column+1);

      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      
      // -Identity
      osl_int_set_si(precision, scattering->m[row], column+1, -1);
      osl_int_set_si(precision, scattering->m[row+1], column+2, -1);
      
      scattering->nb_output_dims += 2;
      
      // reorder
      row_next = clay_relation_get_line(scattering, column+2);
      osl_int_assign(precision, scattering->m[row], scattering->nb_columns-1,
                     scattering->m[row_next], scattering->nb_columns-1);
      
      osl_int_set_si(precision, scattering->m[row_next],
                     scattering->nb_columns-1, 0);
    }
    statement = statement->next;
  }
  
  // get the list of scatnames
  scat = osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
  names = scat->names;
  
  // generate iterator variable name
  i = 0;
  do {
    sprintf(buffer, "__%s%s%d", names->string[column+1],
            names->string[column+1], i);
    i++;
  } while (clay_util_scatnames_exists(scat, buffer));
  new_var_iter = strdup(buffer);
  
  // generate beta variable name
  i = 0;
  do {
    sprintf(buffer, "__b%d", i);
    i++;
  } while (clay_util_scatnames_exists(scat, buffer));
  new_var_beta = strdup(buffer);
  
  // insert the two variables
  nb_strings = osl_strings_size(names) + 2;
  osl_strings_p newnames = osl_strings_malloc();
  CLAY_malloc(newnames->string, char**, sizeof(char**) * (nb_strings + 1));
  
  for (i = 0 ; i < column ; i++) {
    newnames->string[i] = names->string[i];
  }
  newnames->string[i] = new_var_beta;
  newnames->string[i+1] = new_var_iter;
  
  for (i = i+2 ; i < nb_strings ; i++) {
    newnames->string[i] = names->string[i-2];
  }
  newnames->string[i] = NULL; // end of the list
  
  // replace the scatnames
  free(names->string);
  free(names);
  scat->names = newnames;
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_unroll function:
 * Unroll a loop 
 * \param[in,out] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] factor        > 0
 * \param[in] setepilog     if true the epilog will be added
 * \param[in] options
 * \return                  Status
 */
int clay_unroll(osl_scop_p scop, clay_array_p beta_loop, int factor,
                int setepilog, clay_options_p options) {

  /* Description:
   * Clone statements in the beta_loop, and recreate the body.
   * Generate an epilog where the lower bound was removed.
   */

  if (beta_loop->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (factor < 1)
    return CLAY_ERROR_WRONG_FACTOR;
  if (factor == 1)
    return CLAY_SUCCESS;
  
  osl_relation_p scattering;
  osl_relation_p domain;
  osl_relation_p epilog_domain = NULL;
  osl_statement_p statement;
  osl_statement_p newstatement;
  osl_statement_p original_stmt;
  osl_statement_p epilog_stmt;
  clay_array_p beta_max;
  int column = beta_loop->size*2;
  int precision;
  int row;
  int nb_stmts;
  int i;
  int max; // last value of beta_max
  int order;
  int order_epilog = 0; // order juste after the beta_loop
  int current_stmt = 0; // counter of statements
  int last_level = -1;
  int current_level;
  
  osl_body_p body;
  osl_body_p newbody;
  char *expression;
  char **iterator;
  char *substitued;
  char *newexpression;
  char *replacement;
  char substitution[] = "@0@";
  
  int iterator_index = beta_loop->size-1;
  int iterator_size;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_ERROR_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  
  // alloc an array of string, wich will contain the current iterator and NULL
  // used for osl_util_identifier_substitution with only one variable
  CLAY_malloc(iterator, char**, sizeof(char*)*2);
  iterator[1] = NULL;
  
  // infos used to reorder cloned statements
  nb_stmts = clay_beta_nb_parts(statement, beta_loop);
  beta_max = clay_beta_max(statement, beta_loop);
  max = beta_max->data[beta_max->size-1];
  clay_array_free(beta_max);
  
  if (setepilog) {
    // shift to let the place for the epilog loop
    clay_beta_shift_after(scop->statement, beta_loop, beta_loop->size);
    order_epilog = beta_loop->data[beta_loop->size-1] + 1;
  }
      
  while (statement != NULL) {
    scattering = statement->scattering;
    
    // TODO NOTE : we can optimize to not check twice this statement
    if (clay_beta_check(statement, beta_loop)) {
      
      original_stmt = statement;
      body = (osl_body_p) statement->body->data;
      expression = body->expression->string[0];
      iterator[0] = (char*) body->iterators->string[iterator_index];
      iterator_size = strlen(iterator[0]);
      substitued = osl_util_identifier_substitution(expression, iterator);
      
      CLAY_malloc(replacement, char*, 1 + iterator_size + 1 + 16 + 1 + 1);
      
      if (setepilog) {
        // set the epilog from the original statement
        epilog_stmt = osl_statement_nclone(original_stmt, 1);
        row = clay_relation_get_line(original_stmt->scattering, column-2);
        scattering = epilog_stmt->scattering;
        osl_int_set_si(precision,
                       scattering->m[row], scattering->nb_columns-1,
                       order_epilog);
        
        // the order is not important in the statements list
        epilog_stmt->next = statement->next;
        statement->next = epilog_stmt;
        statement = epilog_stmt;
      }
      
      // modify the matrix domain
      domain        = original_stmt->domain;
      
      if (setepilog) {
        epilog_domain = epilog_stmt->domain;
      }
      
      while (domain != NULL) {
      
        for (i = domain->nb_rows-1 ; i >= 0  ; i--) {
          if (!osl_int_zero(precision, domain->m[i], 0)) {
          
            // remove the lower bound on the epilog statement
            if(setepilog &&
               osl_int_pos(precision, domain->m[i], iterator_index+1)) {
              osl_relation_remove_row(epilog_domain, i);
            }
            // remove the upper bound on the original statement
            if (osl_int_neg(precision, domain->m[i], iterator_index+1)) {
              osl_int_add_si(precision, 
                             domain->m[i], domain->nb_columns-1,
                             domain->m[i], domain->nb_columns-1,
                             -factor);
            }
          }
        }
        
        // add local dim on the original statement
        osl_relation_insert_blank_column(domain, domain->nb_output_dims+1);
        osl_relation_insert_blank_row(domain, 0);
        (domain->nb_local_dims)++;
        osl_int_set_si(precision, domain->m[0], domain->nb_output_dims+1,
                       -factor);
        osl_int_set_si(precision, domain->m[0], iterator_index+1, 1);
        
        domain        = domain->next;
        
        if (setepilog)
          epilog_domain = epilog_domain->next;
      }
      
      // clone factor-1 times the original statement
     
      row = clay_relation_get_line(original_stmt->scattering, column);
      current_level = osl_int_get_si(scattering->precision, scattering->m[row], 
                                     scattering->nb_columns-1);
      if (last_level != current_level) {
        current_stmt++;
        last_level = current_level;
      }
      
      for (i = 0 ; i < factor-1 ; i++) {
        newstatement = osl_statement_nclone(original_stmt, 1);
        scattering = newstatement->scattering;
        
        // update the body
        sprintf(replacement, "(%s+%d)", iterator[0], i+1);
        newexpression = clay_util_string_replace(substitution, replacement,
                                            substitued);
        newbody = ((osl_body_p) newstatement->body->data);
        free(newbody->expression->string[0]);
        newbody->expression->string[0] = newexpression;
        
        // reorder
        order = current_stmt + max + nb_stmts*i;
        osl_int_set_si(precision,
                       scattering->m[row], scattering->nb_columns-1,
                       order);
        
        // the order is not important in the statements list
        newstatement->next = statement->next;
        statement->next = newstatement;
        statement = newstatement;
      }
      free(substitued);
      free(replacement);
    }
    statement = statement->next;
  }
  free(iterator);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_tile function:
 * Apply a stripmine then an interchange.
 * The stripmine is at the `depth'th level. Next interchange the level `depth'
 * and `depth_outer'
 * \param[in,out] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >=1
 * \param[in] size          >=1
 * \param[in] depth_outer   >=1
 * \param[in] pretty        See stripmine
 * \param[in] options
 * \return                  Status
 */
int clay_tile(osl_scop_p scop, 
              clay_array_p beta, int depth, int depth_outer, int size,
              int pretty, clay_options_p options) {

  /* Description:
   * stripmine + interchange
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (size <= 0)
    return CLAY_ERROR_WRONG_BLOCK_SIZE;
  if (depth <= 0 || depth_outer <= 0)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  if (depth_outer > depth)
    return CLAY_ERROR_DEPTH_OUTER;
  
  int ret;
  ret = clay_stripmine(scop, beta, depth, size, pretty, options);
  
  if (ret == CLAY_SUCCESS) {
    ret = clay_interchange(scop, beta, depth, depth_outer, options);
  }

  return ret;
}


/**
 * clay_shift function:
 * Shift the iteration domain
 * \param[in,out] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] depth         >=1
 * \param[in] vector        [params1, params2, ..., const]
 * \param[in] options
 * \return                  Status
 */
int clay_shift(osl_scop_p scop, 
               clay_array_p beta, int depth, clay_array_p vector,
               clay_options_p options) {

  /* Description:
   * Add a vector on each statements.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (vector->size == 0)
    return CLAY_ERROR_VECTOR_EMPTY;
  if (depth <= 0)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  if (scop->context->nb_parameters != vector->size-1)
    return CLAY_ERROR_VECTOR;
  
  osl_relation_p scattering;
  osl_statement_p statement;
  osl_int_p tmp, tmp_outdim;
  const int column = depth*2 - 1; // iterator column
  int precision;
  int i, j, k;
  
  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (statement->scattering->nb_input_dims == 0)
    return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
  
  // if it's a statement, the depth must be strictly less than the beta size
  if (beta->size*2-1 >= statement->scattering->nb_output_dims) {
      if (depth >= beta->size)
        return CLAY_ERROR_DEPTH_OVERFLOW;
  }
  // else it's a loop and if the depth is the same as the beta, we change nothing
  else {
    if (depth > beta->size)
      return CLAY_ERROR_DEPTH_OVERFLOW;
  }

  precision = statement->scattering->precision;
  
  tmp = osl_int_malloc(precision);
  tmp_outdim = osl_int_malloc(precision);

  // add the vector for each statements
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;

      if (vector->size <= 1 + scattering->nb_parameters) {
        // for each line where there is a number different from zero on the
        // column
        for (k = 0 ; k < scattering->nb_rows ; k++) {
          if (!osl_int_zero(precision, scattering->m[k], 1+column)) {

            // shifted_value += -1 * coeff_outputdim * shifting

            // outdim = -1 * coeff_outputdim
            osl_int_set_si(precision, tmp_outdim, 0, -1);
            osl_int_mul(precision,
                        tmp_outdim, 0,
                        tmp_outdim, 0,
                        scattering->m[k], 1+column);

            // affects parameters
            i = 1 + scattering->nb_output_dims + scattering->nb_input_dims + 
                scattering->nb_local_dims;
            for (j = 0 ; j < vector->size-1 ; j++) {
              osl_int_mul_si(precision,
                             tmp, 0,
                             tmp_outdim, 0,
                             vector->data[j]);
              osl_int_add(precision,
                          scattering->m[k], i,
                          scattering->m[k], i,
                          tmp, 0);
              i++;
            }

            // set the constant
            osl_int_mul_si(precision,
                           tmp, 0,
                           tmp_outdim, 0,
                           vector->data[j]);
            osl_int_add(precision,
                        scattering->m[k], scattering->nb_columns-1,
                        scattering->m[k], scattering->nb_columns-1,
                        tmp, 0);
          }
        }
      }
    }
    statement = statement->next;
  }

  osl_int_free(precision, tmp, 0);
  osl_int_free(precision, tmp_outdim, 0);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_peel function:
 * Removes the first or last iteration of the loop into separate code.
 * \param[in,out] scop
 * \param[in] beta_loop         Loop beta vector
 * \param[in] peeling array     [param1, param2, ..., const]
 * \param[in] peel_first        1 : peel first

                                    // new cloned loop
                                    for (i= 0 ; i <= start + peeling - 1 ; i++)
                                      S(i);
                                    // modifications on the original loop
                                    for (i = start + peeling ; i <= N ; i++) 
                                      S(i);

                                0 : peel last  

                                    // new cloned loop
                                    for (i = 0 ; i <= end - peeling - 1 ; i++)
                                      S(i);
                                    // modifications on the original loop
                                    for (i = end - peeling ; i <= N ; i++) 
                                      S(i);
 * \param[in] options
 * \return                      Status
 */
int clay_peel(osl_scop_p scop, 
              clay_array_p beta_loop, clay_array_p peeling, int peel_first,
              clay_options_p options) {

  /* Description:
   * The peeling function looks like the iss function.
   * We add or substract the vector on the lower or upper bound.
   */

  if (beta_loop->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (peeling->size == 0)
    return CLAY_SUCCESS;
  if (scop->context->nb_parameters != peeling->size-1)
      return CLAY_ERROR_INEQU;
  
  osl_relation_p scattering, domain, newscattering;
  osl_statement_p statement;
  osl_statement_p newstatement;
  const int column = (beta_loop->size-1)*2;
  int precision;
  int row;
  int i, j, k, l;
  void *order; // new loop order for the clones
  
  // search a statement
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_ERROR_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;

  // let the place for the splitted loop
  clay_beta_shift_after(scop->statement, beta_loop, beta_loop->size);

  // init new order value for the new loop
  scattering = statement->scattering;
  
  row = clay_relation_get_line(scattering, column); // parent loop line
  
  order = osl_int_malloc(precision);
  osl_int_assign(precision,
                 order, 0,
                 scattering->m[row], scattering->nb_columns-1);
  osl_int_increment(precision,
                    order, 0,
                    order, 0);
  
  // oppose the vector if we want to peel after the loop
  if (!peel_first) {
    for (i = 0 ; i < peeling->size ; i++) {
      peeling->data[i] = -peeling->data[i];
    }
  }
  
  // peel...
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      scattering = statement->scattering;
      if (peeling->size <= 1 + scattering->nb_input_dims + 
                                                    scattering->nb_parameters) {
        
        newstatement = osl_statement_nclone(statement, 1);
        
        // create the inequation
        
        domain = statement->domain;
        while (domain != NULL) {

          for (i = 0 ; i < domain->nb_rows  ; i++) {

            // an inequation
            if (!osl_int_zero(precision, domain->m[i], 0)) {
            
              // if before : search all the lower bounds
              if((peel_first &&
                  osl_int_pos(precision, domain->m[i], beta_loop->size)) ||
                 (!peel_first &&
                  osl_int_neg(precision, domain->m[i], beta_loop->size))) {
                
                // TODO : optimization...
                
                newscattering = newstatement->scattering;
                
                // add an empty line
                row = scattering->nb_rows;
                osl_relation_insert_blank_row(newscattering, row);
                osl_relation_insert_blank_row(scattering, row);
                
                // copy the current line of the domain into the new line
                
                // copy i/e
                osl_int_assign(precision,
                               scattering->m[row], 0,
                               domain->m[i], 0);
                
                // copy output dims (domain) to input dims (scattering)
                j = scattering->nb_output_dims + 1;
                k = 1;
                while (k < domain->nb_output_dims+1 &&
                       j < 1 + scattering->nb_output_dims +
                               scattering->nb_input_dims) {

                  osl_int_assign(precision,
                                 scattering->m[row], j,
                                 domain->m[i], k);
                  j++;
                  k++;
                }
                
                // column where we insert new local dims
                j = 1 + scattering->nb_output_dims +
                    scattering->nb_input_dims;
                
                // if there are no local dims columns, create new columns 
                // on the current statement and the new statement
                if (scattering->nb_local_dims < domain->nb_local_dims) {
                  for (k = scattering->nb_local_dims ; 
                       k < domain->nb_local_dims ; k++) {
                    
                    osl_relation_insert_blank_column(scattering, j);
                    osl_relation_insert_blank_column(newscattering, j);
                  }
                  
                  scattering->nb_local_dims = domain->nb_local_dims;
                }
                  
                // copy local dims
                k = domain->nb_output_dims+1;
                while (k < 1 + domain->nb_output_dims + domain->nb_local_dims &&
                       j < 1 + scattering->nb_output_dims + 
                               scattering->nb_input_dims + 
                               scattering->nb_local_dims) {
                  
                  osl_int_assign(precision,
                                 scattering->m[row], j,
                                 domain->m[i], k);
                  j++;
                  k++;
                }
                
                // copy parameters
                j = 1 + scattering->nb_output_dims + scattering->nb_input_dims +
                    scattering->nb_local_dims;
                k = 1 + domain->nb_output_dims + domain->nb_local_dims;
                l = 0; // index in the peeling array
                while (k < domain->nb_columns - 1 &&
                       j < scattering->nb_columns - 1 &&
                       l < peeling->size - 1) {
                  
                  osl_int_assign(precision,
                                 scattering->m[row], j,
                                 domain->m[i], k);
                  osl_int_add_si(precision,
                                 scattering->m[row], j,
                                 scattering->m[row], j,
                                 peeling->data[l]);
                  l++;
                  j++;
                  k++;
                }
                
                // copy const
                osl_int_assign(precision,
                               scattering->m[row], scattering->nb_columns-1,
                               domain->m[i], domain->nb_columns-1);
                osl_int_add_si(precision,
                               scattering->m[row], scattering->nb_columns-1,
                               scattering->m[row], scattering->nb_columns-1,
                               peeling->data[peeling->size-1]);
                
                // we have something like this i <= N
                // and we want : i >= N, so we oppose the line
                // TODO : not optimized, we can copy first on the newstatement
                // because we re-oppose after
                if (!peel_first) {
                    j = 1 + scattering->nb_output_dims;
                    while (j < scattering->nb_columns) {
                      osl_int_oppose(precision,
                                     scattering->m[row], j,
                                     scattering->m[row], j);
                      j++;
                    }
                }
                
                // now we copy and oppose the line and substract 1 for the
                // previous loop (this is for the upper bound)
                newscattering = newstatement->scattering;
                for (k = 0 ; k < scattering->nb_columns ; k++) {
                  osl_int_assign(precision,
                                 newscattering->m[row], k,
                                 scattering->m[row], k);
                  if (k >= scattering->nb_output_dims + 1) {
                    osl_int_oppose(precision,
                                   newscattering->m[row], k,
                                   newscattering->m[row], k);
                  }
                }
                
                osl_int_decrement(precision,
                                  newscattering->m[row],
                                  newscattering->nb_columns-1,
                                  newscattering->m[row],
                                  newscattering->nb_columns-1);
              }
            }
          }
          
          domain = domain->next;
        }
        
        // the current statement is after the new statement
        scattering = statement->scattering;
        row = clay_relation_get_line(scattering, column);
        osl_int_assign(precision,
                 scattering->m[row], scattering->nb_columns-1,
                 order, 0);
  
        // the order is not important in the statements list
        newstatement->next = statement->next;
        statement->next = newstatement;
        statement = statement->next;
      }
    }
    statement = statement->next;
  }
  
  osl_int_free(precision, order, 0);
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
    
  return CLAY_SUCCESS;
}


/**
 * clay_context function:
 * Add a line to the context
 * \param[in,out] scop
 * \param[in] vector        [param1, param2, ..., 1]
 * \param[in] options
 * \return                  Status
 */
int clay_context(osl_scop_p scop, clay_array_p vector, 
                 clay_options_p options) {

  /* Description:
   * Add a new line in the context matrix.
   */

  if (vector->size < 2)
    return CLAY_ERROR_VECTOR;
  if (scop->context->nb_parameters != vector->size-2)
    return CLAY_ERROR_VECTOR;
  
  osl_relation_p context;
  int row;
  int i, j;
  int precision;
  
  context = scop->context;
  precision = context->precision;
  row = context->nb_rows;
  osl_relation_insert_blank_row(context, row);
  
  osl_int_set_si(precision,
                 context->m[row], 0,
                 vector->data[0]);
  
  j = 1 + context->nb_output_dims + context->nb_input_dims +
      context->nb_local_dims;
  for (i = 1 ; i < vector->size ; i++) {
    osl_int_set_si(precision,
                   context->m[row], j,
                   vector->data[i]);
    j++;
  }
  
  return CLAY_SUCCESS;
}  
