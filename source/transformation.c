
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <osl/macros.h>
#include <osl/scop.h>
#include <osl/body.h>
#include <osl/strings.h>
#include <osl/util.h>
#include <osl/extensions/scatnames.h>
#include <osl/extensions/scatnames.h>
#include <osl/statement.h>
#include <osl/relation.h>
#include <clay/transformation.h>
#include <clay/array.h>
#include <clay/macros.h>
#include <clay/options.h>
#include <clay/errors.h>


/*****************************************************************************\
 *                     Loop transformations                                   *
 `****************************************************************************/


/**
 * clay_reorder function:
 * Reorders the statements in the loop
 * \param[in] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] order         Array to reorder the statements
 * \param[in] options
 * \return                  Status
 */
int clay_reorder(osl_scop_p scop, 
                  clay_array_p beta_loop, clay_array_p neworder,
                  clay_options_p options) {
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column = beta_loop->size * 2; // transition column
  int row;
  int i = 0;
  
  statement = clay_beta_find(statement, beta_loop);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      if (i >= neworder->size)
        return CLAY_REORDER_ARRAY_TOO_SMALL;
      row = clay_statement_get_line(statement, column);
      scattering = statement->scattering;
      osl_int_set_si(precision, 
                     scattering->m[row], scattering->nb_columns-1,
                     neworder->data[i]);
      i++;
    }
    statement = statement->next;
  }
  
  if (options && options->normalize)
    clay_scop_normalize_beta(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_reversal function:
 * Reverse the direction of the loop
 * \param[in] scop
 * \param[in] beta          Beta vector
 * \param[in] options
 * \return                  Status
 */
int clay_reversal(osl_scop_p scop, clay_array_p beta, int depth,
                  clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (depth <= 0) 
    return CLAY_DEPTH_OVERFLOW;
    
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  int column = depth*2 - 1; // iterator column
  int row;
  int i, begin, end;
  void *matrix_row;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (beta->size*2-1 >= statement->scattering->nb_output_dims && 
      depth >= beta->size)
    return CLAY_DEPTH_OVERFLOW;
  // else it's a loop, and the depth must be less or equal than the beta size
  if (depth > beta->size)
    return CLAY_DEPTH_OVERFLOW;

  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      begin = scattering->nb_output_dims;
      end = begin + scattering->nb_input_dims;
      for(i = begin ; i <= end ; i++) {
        row = clay_statement_get_line(statement, column);
        matrix_row = scattering->m[row];
        osl_int_oppose(precision, 
                       matrix_row, i,
                       matrix_row, i);
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
 * \param[in] scop
 * \param[in] beta          Beta vector (inner loop or statement)
 * \param[in] depth_1       >= 1
 * \param[in] depth_2       >= 1
 * \param[in] options
 * \return                  Status
 */
int clay_interchange(osl_scop_p scop, 
                      clay_array_p beta, int depth_1, int depth_2,
                      clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (depth_1 <= 0 || depth_2 <= 0) 
    return CLAY_DEPTH_OVERFLOW;

  osl_statement_p statement = scop->statement;
  osl_relation_p scattering;
  int precision;
  const int column_1 = depth_1*2 - 1; // iterator column
  const int column_2 = depth_2*2 - 1;
  int row_1, row_2;
  void **matrix;
  void *tmp;
  //int nb_rows;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (statement->scattering->nb_output_dims == 1)
    return CLAY_DEPTH_OVERFLOW;
  // if it's a statement, the depth must be strictly less than the beta size
  if (beta->size*2-1 >= statement->scattering->nb_output_dims && 
      (depth_1 >= beta->size || 
      depth_2 >= beta->size))
    return CLAY_DEPTH_OVERFLOW;
  // else it's a loop, and the depth must be less or equal than the beta size
  if (depth_1 > beta->size || depth_2 > beta->size)
    return CLAY_DEPTH_OVERFLOW;
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
      
      matrix = scattering->m;
      
      row_1 = clay_statement_get_line(statement, column_1);
      row_2 = clay_statement_get_line(statement, column_2);
      
      // recreate the -Identity matrix
      osl_int_set_si(precision, matrix[row_1], column_1+1, 0);
      osl_int_set_si(precision, matrix[row_1], column_2+1, -1);
      osl_int_set_si(precision, matrix[row_2], column_2+1, 0);
      osl_int_set_si(precision, matrix[row_2], column_1+1, -1);
      
      tmp = matrix[row_1];
      matrix[row_1] = matrix[row_2];
      matrix[row_2] = tmp;
    }
    
    statement = statement->next;
  }
  return CLAY_SUCCESS;
}


/**
 * clay_fission function:
 * Split the loop in two parts at the `depth'th level from the statement
 * \param[in] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >= 1
 * \param[in] options
 * \return                  Status
 */
int clay_fission(osl_scop_p scop, clay_array_p beta, int depth,
                 clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (beta->size <= 1 || depth <= 0 || depth >= beta->size)
    return CLAY_DEPTH_OVERFLOW;
  
  osl_statement_p statement = scop->statement;
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  
  clay_beta_shift_before(scop->statement, beta, depth);
  
  if (options && options->normalize)
    clay_scop_normalize_beta(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_fuse function:
 * Fuse loop with the first loop after
 * \param[in] scop
 * \param[in] beta_vector   Loop beta vector
 * \param[in] options
 * \return                  Status
 */
int clay_fuse(osl_scop_p scop, clay_array_p beta_loop,
              clay_options_p options) {
  if (beta_loop->size == 0)
    return CLAY_BETA_EMPTY;
 
  osl_relation_p scattering;
  osl_statement_p statement;
  clay_array_p beta_max;
  clay_array_p beta_next;
  int precision;
  const int depth = beta_loop->size;
  const int column = beta_loop->size*2; // transition column
  int row;
  int restore_size;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_NOT_BETA_LOOP;

  precision = statement->scattering->precision;
  
  beta_max = clay_beta_max(statement, beta_loop);
  beta_next = clay_beta_next_part(scop->statement, beta_loop);
  
  if (beta_next != NULL) {
    restore_size = beta_next->size;
    beta_next->size = depth;
    statement = scop->statement;
    while (statement != NULL) {
      if (clay_beta_check(statement, beta_next)) {
        scattering = statement->scattering;
        if (column < scattering->nb_output_dims) {
        
          // Set the loop level
          row = clay_statement_get_line(statement, column-2);
          osl_int_set_si(precision, 
                         scattering->m[row], scattering->nb_columns-1, 
                         beta_loop->data[depth-1]);

          // Reorder the statement
          row = clay_statement_get_line(statement, column);
          osl_int_add_si(precision,
                         scattering->m[row], scattering->nb_columns-1,
                         scattering->m[row], scattering->nb_columns-1,
                         beta_max->data[depth]+1);
        }
      }
      statement = statement->next;
    }
    beta_next->size = restore_size;
    clay_array_free(beta_next);
  }
  
  clay_array_free(beta_max);
  
  if (options && options->normalize)
    clay_scop_normalize_beta(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_skew function:
 * Skew the loop (or statement) from the `depth'th loop
 * (i, j) -> (i, j+i*coeff) where `depth' is the loop of i
 * \param[in] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >= 1
 * \param[in] coeff         != 0
 * \param[in] options
 * \return                  Status
 */
int clay_skew(osl_scop_p scop, 
              clay_array_p beta, int depth, int coeff,
              clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (depth <= 0)
    return CLAY_DEPTH_OVERFLOW;
  if (coeff == 0)
    return CLAY_WRONG_COEFF;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column_depth = depth*2 - 1; // iterator column
  int column_beta; // iterator column
  int row_depth, row_beta;
  int i;
  void **matrix;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  
  if (statement->scattering->nb_output_dims == 1)
    return CLAY_DEPTH_OVERFLOW;
  // if it's a statement, the depth must be strictly less than the beta size
  if (beta->size*2-1 >= statement->scattering->nb_output_dims) {
      if (depth >= beta->size)
        return CLAY_DEPTH_OVERFLOW;
      if (depth == beta->size-1) // TODO no sense ?
        return CLAY_SUCCESS;
      column_beta = beta->size*2 - 3;
  }
  // else it's a loop and if the depth is the same as the beta, we change nothing
  else {
    if (depth == beta->size)
      return CLAY_SUCCESS;
    column_beta = beta->size*2 - 1;
  }
  if (depth > beta->size)
    return CLAY_DEPTH_OVERFLOW;
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (column_depth >= scattering->nb_output_dims)
        return CLAY_DEPTH_OVERFLOW;
      matrix = scattering->m;
      i = depth + scattering->nb_output_dims; // TODO : iterotor column ??
      row_depth = clay_statement_get_line(statement, column_depth);
      row_beta = clay_statement_get_line(statement, column_beta);
      osl_int_add(precision, 
                  matrix[row_beta], i,
                  matrix[row_beta], i,
                  matrix[row_depth], i);
      osl_int_mul_si(precision, 
                     matrix[row_beta], i,
                     matrix[row_beta], i,
                     coeff);
    }
    statement = statement->next;
  }
  return CLAY_SUCCESS;
}


/**
 * clay_iss function:
 * Split the loop (or statement) depending of an inequation
 * (i, j) -> (i, j+i*coeff) where `depth' is the loop of i
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] equation array
 * \param[in] options
 * \return                  Status
 */
int clay_iss(osl_scop_p scop, 
             clay_array_p beta, clay_array_p equ,
             clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (equ->size == 0)
    return CLAY_SUCCESS;
  
  osl_relation_p scattering;
  osl_statement_p statement;
  osl_statement_p newstatement;
  const int column = (beta->size-1)*2;
  int precision;
  int i, j;
  int row;
  int equ_nb_input_dims, equ_nb_parameters;
  void *order; // new loop order for the clones
  
  // we need the first because we need to get the nb_input_dims and the
  // nb_parameters for the equation
  statement = clay_beta_first_statement(scop->statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (statement->scattering->nb_input_dims == 0)
    return CLAY_BETA_NOT_IN_A_LOOP;
  
  precision = statement->scattering->precision;
  
  clay_beta_shift_after(scop->statement, beta, beta->size);

  // init new order
  scattering = statement->scattering;
  row = clay_statement_get_line(statement, column); // parent loop line
  order = osl_int_malloc(precision);
  osl_int_assign(precision,
                 order, 0,
                 scattering->m[row], scattering->nb_columns-1);
  osl_int_increment(precision,
                    order, 0,
                    order, 0);
  
  equ_nb_input_dims = scattering->nb_input_dims;
  equ_nb_parameters = scattering->nb_parameters;
  
  // set the inequation
  statement = scop->statement;
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (equ->size <= 1 + scattering->nb_input_dims + 
                                                    scattering->nb_parameters) {

        // insert the inequation spliting (local dims are not in the equation)
        // (at the end)
        row = scattering->nb_rows;
        osl_relation_insert_blank_row(scattering, row);
        osl_int_set_si(precision, scattering->m[row], 0, 1);
        
        // affects input_dims
        i = scattering->nb_output_dims+1;
        for (j = 0 ; j < equ_nb_input_dims ; j++) {
          osl_int_set_si(precision,
                         scattering->m[row], i,
                         equ->data[j]);
          i++;
        }
        // affects parameters
        i = 1 + scattering->nb_output_dims + scattering->nb_input_dims + 
            scattering->nb_local_dims;
        for (j = equ_nb_input_dims ; j < equ_nb_parameters ; j++) {
          osl_int_set_si(precision,
                         scattering->m[row], i,
                         equ->data[j]);
          i++;
        }
        // affects the constant
        osl_int_set_si(precision,
                       scattering->m[row], scattering->nb_columns-1,
                       equ->data[equ->size-1]);
        
        // insert a new statement with the negation of the inequation
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
      
        // the first statement is after the new statement
        scattering = statement->scattering;
        row = clay_statement_get_line(statement, column);
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
    clay_scop_normalize_beta(scop);
    
  return CLAY_SUCCESS;
}


/**
 * clay_stripmine function:
 * Decompose a single loop into two nested loop
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] block         Size of the inner loop
 * \param[in] pretty        If true, clay will keep the variables name
 *                          /!\ It takes much more computing 
 * \param[in] options
 * \return                  Status
 */
int clay_stripmine(osl_scop_p scop, clay_array_p beta, int block, int pretty,
                   clay_options_p options) {
  if (beta->size == 0)
    return CLAY_BETA_EMPTY;
  if (block <= 0)
    return CLAY_WRONG_BLOCK_SIZE;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  osl_scatnames_p scat;
  osl_strings_p names;
  int column = (beta->size-1)*2;
  int precision;
  int row, row_next;
  int iter_column;
  int i;
  int nb_strings;
  char buffer[OSL_MAX_STRING];
  char *new_var_iter;
  char *new_var_beta;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_BETA_NOT_FOUND;
  if (statement->scattering->nb_output_dims < 3)
    return CLAY_BETA_NOT_IN_A_LOOP;
  
  if (beta->size*2-1 == statement->scattering->nb_output_dims)
    column -= 2; // loop level
  
  precision = statement->scattering->precision;
  if (pretty)
    statement = scop->statement; // TODO optimization...
        
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    scattering = statement->scattering;
    if (clay_beta_check(statement, beta)) {
      
      // set the strip mine
      row = clay_statement_get_line(statement, column);
      
      osl_relation_insert_blank_column(scattering, column+1);
      osl_relation_insert_blank_column(scattering, column+1);

      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      
      osl_int_set_si(precision, scattering->m[row+0], column+1, -1);
      osl_int_set_si(precision, scattering->m[row+1], column+2, -block);
      osl_int_set_si(precision, scattering->m[row+2], column+2, block);
      osl_int_set_si(precision, scattering->m[row+2], scattering->nb_columns-1, 
                     block-1);
                     
      osl_int_set_si(precision, scattering->m[row+1], 0, 1);
      osl_int_set_si(precision, scattering->m[row+2], 0, 1);
      
      iter_column = CLAY_min(beta->size, scattering->nb_input_dims) + 
                    scattering->nb_output_dims + 2;
      osl_int_set_si(precision, scattering->m[row+1], iter_column, 1);
      osl_int_set_si(precision, scattering->m[row+2], iter_column, -1);
      
      scattering->nb_output_dims += 2;
      
      // reorder
      row_next = clay_statement_get_line(statement, column+2);
      osl_int_assign(precision, scattering->m[row], scattering->nb_columns-1,
                     scattering->m[row_next], scattering->nb_columns-1);
      
      osl_int_set_si(precision, scattering->m[row_next],
                     scattering->nb_columns-1, 0);
    
    } else if (pretty && column < scattering->nb_output_dims) {
      
      // add 2 empty dimensions
      row = clay_statement_get_line(statement, column);
      
      osl_relation_insert_blank_column(scattering, column+1);
      osl_relation_insert_blank_column(scattering, column+1);

      osl_relation_insert_blank_row(scattering, column);
      osl_relation_insert_blank_row(scattering, column);
      
      osl_int_set_si(precision, scattering->m[row], column+1, -1);
      osl_int_set_si(precision, scattering->m[row+1], column+2, -1);
      
      scattering->nb_output_dims += 2;
      
      // reorder
      row_next = clay_statement_get_line(statement, column+2);
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
  
  // generate new variable names
  i = 0;
  do {
    sprintf(buffer, "__%s%s%d", names->string[column+1],
            names->string[column+1], i);
    i++;
  } while (clay_scatnames_exists(scat, buffer));
  new_var_iter = strdup(buffer);
  
  i = 0;
  do {
    sprintf(buffer, "__b%d", i);
    i++;
  } while (clay_scatnames_exists(scat, buffer));
  new_var_beta = strdup(buffer);
  
  // insert the new variables
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
    clay_scop_normalize_beta(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_unroll function:
 * Unroll a loop 
 * \param[in] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] factor        > 0
 * \param[in] options
 * \return                  Status
 */
int clay_unroll(osl_scop_p scop, clay_array_p beta_loop, int factor,
               clay_options_p options) {
  if (beta_loop->size == 0)
    return CLAY_BETA_EMPTY;
  if (factor < 1)
    return CLAY_WRONG_FACTOR;
  if (factor == 1)
    return CLAY_SUCCESS;
  
  osl_relation_p scattering;
  osl_relation_p domain;
  osl_relation_p epilog_domain;
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
  int order_epilog; // order juste after the beta_loop
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
    return CLAY_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_NOT_BETA_LOOP;
  
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
  
  // shift to let the place for the epilog loop
  clay_beta_shift_after(scop->statement, beta_loop, beta_loop->size);
  order_epilog = beta_loop->data[beta_loop->size-1] + 1;
  
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
      
      
      // set the epilog from the original statement
      epilog_stmt = osl_statement_nclone(original_stmt, 1);
      row = clay_statement_get_line(original_stmt, column-2);
      scattering = epilog_stmt->scattering;
      osl_int_set_si(precision,
                     scattering->m[row], scattering->nb_columns-1,
                     order_epilog);
      
      epilog_stmt->next = statement->next;
      statement->next = epilog_stmt;
      statement = epilog_stmt;
      
      // modify the matrix domain
      domain        = original_stmt->domain;
      epilog_domain = epilog_stmt->domain;
      while (domain != NULL) {
      
        for (i = domain->nb_rows-1 ; i >= 0  ; i--) {
          if (!osl_int_zero(precision, domain->m[i], 0)) {
          
            // remove the lower bound on the epilog statement
            if(osl_int_pos(precision, domain->m[i], iterator_index+1)) {
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
        osl_int_set_si(precision, domain->m[0], domain->nb_output_dims+1, -factor);
        osl_int_set_si(precision, domain->m[0], iterator_index+1, 1);
        
        domain        = domain->next;
        epilog_domain = epilog_domain->next;
      }
      
      // clone factor-1 times the original statement
     
      row = clay_statement_get_line(original_stmt, column);
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
        newexpression = clay_string_replace(substitution, replacement,
                                            substitued);
        newbody = ((osl_body_p) newstatement->body->data);
        free(newbody->expression->string[0]);
        newbody->expression->string[0] = newexpression;
        
        // reorder
        order = current_stmt + max + nb_stmts*i;
        osl_int_set_si(precision,
                       scattering->m[row], scattering->nb_columns-1,
                       order);
        
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
    clay_scop_normalize_beta(scop);
  
  return CLAY_SUCCESS;
}


/*****************************************************************************\
 *                     Other operations                                       *
 `****************************************************************************/

/* 
 * clay_scop_normalize_beta function:
 * Normalize all the beta
 * \param[in] scop
 */
void clay_scop_normalize_beta(osl_scop_p scop) {
  
  osl_statement_p sout;
  osl_relation_p scattering;
  clay_array_p beta;
  clay_array_p beta_last;
  
  int counter_loops[CLAY_TRANSFORMATIONS_MAX_BETA_SIZE];
  int i;
  int row;
  
  for (i = 0 ; i < CLAY_TRANSFORMATIONS_MAX_BETA_SIZE ; i++) {
    counter_loops[i] = 0;
  }
  
  // first statement
  beta_last = clay_array_malloc();
  beta = clay_beta_next(scop->statement, beta_last, &sout);
  
  if (beta == NULL) {
    clay_array_free(beta_last);
    return;
  }
  
  // for each statement, we set the smallest beta
  while (1) {
    // set the smallest beta
    for (i = 0 ; i < beta->size ; i++) {
      beta->data[i] = counter_loops[i];
      row = clay_statement_get_line(sout, i*2);
      scattering = sout->scattering;
      osl_int_set_si(scattering->precision, scattering->m[row],
                     scattering->nb_columns-1, counter_loops[i]);
    }
    
    clay_array_free(beta_last);
    beta_last = beta;
    beta = clay_beta_next(scop->statement, beta_last, &sout);
    
    if (beta == NULL) {
      clay_array_free(beta_last);
      return;
    }
    
    // update the counter array for the next step
    
    // search the first value wich is greater than the beta_last
    i = 0;
    while (i < beta->size && i < beta_last->size) {
      if (beta->data[i] > beta_last->data[i])
        break;
      i++;
    }
    
    if (i != beta->size)
      counter_loops[i]++;
    
    // the rest is set to zero
    i++;
    while (i < beta_last->size) {
      counter_loops[i] = 0;
      i++;
    }
    
  }
}


/* 
 * clay_string_replace function:
 * Search and replace a string with another string , in a string
 * Minor modifications from :
 * http://www.binarytides.com/blog/str_replace-for-c/
 * \param[in] search
 * \param[in] replace
 * \param[in] subject
 */
char* clay_string_replace(char *search, char *replace, char *string) {
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
 * clay_beta_max function:
 * Return beta min
 * \param[in] statement     List of statements
 * \param[in] beta          Beta vector parent
 * \return
 */
clay_array_p clay_beta_min(osl_statement_p statement, clay_array_p beta) {
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return NULL;
  
  clay_array_p beta_min = clay_statement_get_beta(statement);
  
  while (statement != NULL) {
    if (clay_beta_check(statement, beta) &&
        clay_statement_is_before(statement, beta_min)) {
      clay_array_free(beta_min);
      beta_min = clay_statement_get_beta(statement);
    }
    statement = statement->next;
  }
  
  return beta_min;
}


/**
 * clay_beta_max function:
 * Return the beta max
 * \param[in] statement     List of statements
 * \param[in] beta          Beta vector parent
 * \return
 */
clay_array_p clay_beta_max(osl_statement_p statement, clay_array_p beta) {
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return NULL;
  
  clay_array_p beta_max = clay_statement_get_beta(statement);
  
  while (statement != NULL) {
    if (clay_beta_check(statement, beta) &&
        clay_statement_is_after(statement, beta_max)) {
      clay_array_free(beta_max);
      beta_max = clay_statement_get_beta(statement);
    }
    statement = statement->next;
  }
  
  return beta_max;
}


/**
 * clay_beta_next function:
 * Return the beta after the given beta.
 * \param[in] statement     List of statements
 * \param[in] beta          Beta vector
 * \param[in] sout          If not NULL, this is the statement corresponding to 
 *                          the returned beta
 * \return
 */
clay_array_p clay_beta_next(osl_statement_p statement, clay_array_p beta,
                            osl_statement_p *sout) {
  if(!statement)
    return NULL;
  clay_array_p beta_next = NULL;
  const int nb_dims = beta->size*2-1;
  int is_statement;
  
  // Search the first statement after the beta
  while (statement != NULL) {
    is_statement = statement->scattering->nb_output_dims <= nb_dims;
    if ((!is_statement && !clay_statement_is_before(statement, beta)) ||
        (is_statement && clay_statement_is_after(statement, beta))) {
      beta_next = clay_statement_get_beta(statement);
      if (sout) *sout = statement;
      break;
    }
    statement = statement->next;
  }
  
  if (beta_next == NULL) {
    if (sout) *sout = NULL;
    return NULL;
  }
  if (is_statement && 
      statement->scattering->nb_output_dims < beta_next->size*2-1) {
    clay_array_free(beta_next);
    if (sout) *sout = NULL;
    return NULL;
  }

  // Search if there is an another beta before the beta we have found
  while (statement != NULL) {
    is_statement = statement->scattering->nb_output_dims <= nb_dims;
    if (((!is_statement && !clay_statement_is_before(statement, beta)) ||
         (is_statement && clay_statement_is_after(statement, beta))) &&
        clay_statement_is_before(statement, beta_next)) {
      clay_array_free(beta_next);
      beta_next = clay_statement_get_beta(statement);
      if (sout) *sout = statement;
    }
    statement = statement->next;
  }
  
  return beta_next;
}


/**
 * clay_beta_next_part function:
 * Return the beta after (strictly) the given beta.
 * If the beta is a loop, the next beta is the beta which is after the loop (and
 * not the statements inside the loop)
 * \param[in] statement     List of statements
 * \param[in] beta          Beta vector
 * \return
 */
clay_array_p clay_beta_next_part(osl_statement_p statement, clay_array_p beta) {
  if(!statement)
    return NULL;
  clay_array_p beta_next = NULL;
  
  // Search the first statement after the beta
  while (statement != NULL) {
    if (clay_statement_is_after(statement, beta)) {
      beta_next = clay_statement_get_beta(statement);
      break;
    }
    statement = statement->next;
  }
  
  if (beta_next == NULL)
    return NULL;
  
  // Search if there is an another beta before the beta we have found
  while (statement != NULL) {
    if (clay_statement_is_after(statement, beta) && 
        clay_statement_is_before(statement, beta_next)) {
      clay_array_free(beta_next);
      beta_next = clay_statement_get_beta(statement);
    }
    statement = statement->next;
  }
  
  return beta_next;
}


/**
 * clay_scatnames_exists_iterator_iterator function:
 * Return true if the original iterator name is already in the scattering.
 * \param[in] scattering
 * \return
 */
bool clay_scatnames_exists(osl_scatnames_p scatnames, char *iter) {
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
 * clay_statement_get_line function:
 * Because the lines in the scattering matrix may have not ordered, we have to
 * search the corresponding line. It returns the first line where the value is
 * different from zero in the `column'. `column' is between 0 and 
 * nb_output_dims-1
 * \param[in] statement     Current statement
 * \param[in] column        Line to search
 * \return                  Return the real line
 */
int clay_statement_get_line(osl_statement_p statement, int column) {
  osl_relation_p scattering = statement->scattering;
  if (column < 0 || column >= scattering->nb_output_dims)
    return -1;
  int i;
  int precision = scattering->precision;
  for (i = 0 ; i < scattering->nb_rows ; i++) {
    if (osl_int_zero(precision, scattering->m[i], 0)) {
      if (!osl_int_zero(precision, scattering->m[i], column+1)) {
        break;
      }
    }
  }
  return (i == scattering->nb_rows ? -1 : i );
}


/**
 * clay_statement_get_beta function:
 * \param[in] statement     Current statement
 * \return                  Return the real line
 */
clay_array_p clay_statement_get_beta(osl_statement_p statement) {
  if (statement == NULL)
    return NULL;
  osl_relation_p scattering = statement->scattering;
  clay_array_p beta = clay_array_malloc();
  int i, j,  row, tmp;
  int last_column = scattering->nb_columns-1;
  int precision = scattering->precision;
  
  for (j = 1 ; j < scattering->nb_output_dims+1 ; j += 2) {
    
    // search the first non zero
    for (i = 0 ; i < scattering->nb_rows ; i++) {
      // test if the line is an equality
      if (osl_int_zero(precision, scattering->m[i], 0)) {
        // non zero on the column
        if (!osl_int_zero(precision, scattering->m[i], j)) {
          // line and column are corrects
          if (clay_scattering_check_zero(statement, i, j)) {
            row = clay_statement_get_line(statement, i);
            tmp = osl_int_get_si(scattering->precision, 
                                 scattering->m[row], last_column);
            clay_array_add(beta, tmp);
            break;
          }
        }
      }
    }
  }

  return beta;
}


/**
 * clay_beta_find function:
 * Return the first statement (the first checked by the beta and not the first 
 * in the list, use clay_beta_min for this purpose) which corresponds to the 
 * beta vector
 * \param[in] statement     Start statement list
 * \param[in] beta          Vector to search
 * \return                  Return the first corresponding statement
 */
osl_statement_p clay_beta_find(osl_statement_p statement, 
                               clay_array_p beta) {
  while (statement != NULL) {
    if (clay_beta_check(statement, beta))
      break;
    statement = statement->next;
  }
  return statement;
}


/**
 * clay_beta_first_statement function:
 * Return the first statement (the first checked by the beta and not the first 
 * in the list, use clay_beta_min for this purpose) which is at the same level 
 * of the beta (not in a sub loop)
 * \param[in] statement     Start statement list
 * \param[in] beta          Vector to search
 * \return                  Return the first corresponding statement
 */
osl_statement_p clay_beta_first_statement(osl_statement_p statement, 
                                          clay_array_p beta) {
  osl_relation_p scattering;
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (scattering->nb_output_dims <= beta->size*2-1 || 
          scattering->nb_output_dims <= beta->size*2+1) {
        break;
      }
    }
    statement = statement->next;
  }
  return statement;
}


/**
 * clay_beta_nb_parts function:
 * It doesn't count statements inside sub loops
 * Example:
 * for(i) {
 *   S1(i)
 *   for(j) {
 *     S2(i,j)
 *     S3(i,j)
 *   }
 * }
 * nb_parts in for(i) = 2 (S1 and for(j))
 * \param[in] statement     Statements list
 * \param[in] beta          Vector to search
 * \return
 */
int clay_beta_nb_parts(osl_statement_p statement, clay_array_p beta) {
  int n = 0;
  const int column = beta->size*2;
  int last_level = -1;
  int current_level;
  int row;
  osl_relation_p scattering;
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;
      if (column < scattering->nb_output_dims) {
        row = clay_statement_get_line(statement, column);
        current_level = osl_int_get_si(scattering->precision, scattering->m[row],
                                       scattering->nb_columns-1);
        if (current_level != last_level) {
          n++;
          last_level = current_level;
        }
      } else if (column-2 >= scattering->nb_output_dims-1) {
        // it's a statement
        return 1;
      }
    }
    statement = statement->next;
  }
  return n;
}


/**
 * clay_beta_shift_before function:
 * Shift all the statements that are before or on the beta vector
 * \param[in] statement     Statements list
 * \param[in] beta          Beta vector 
 * \param[in] depth         Depth level to shift, >= 1
 */
void clay_beta_shift_before(osl_statement_p statement, clay_array_p beta,
                           int depth) {
  if (beta->size == 0)
    return;
  osl_relation_p scattering;
  const int column = (depth-1)*2;  // transition column
  int row;
  int precision = statement->scattering->precision;
  int restore_size = beta->size;
  
  clay_array_p beta_parent = clay_array_clone(beta);
  (beta_parent)->size = depth-1;
  
  while (statement != NULL) {
    if (!clay_statement_is_before(statement, beta)) {
      if (clay_beta_check(statement, beta_parent)) {
        scattering = statement->scattering;
        if (column < scattering->nb_output_dims) {
          row = clay_statement_get_line(statement, column);
          osl_int_increment(precision, 
                            scattering->m[row], scattering->nb_columns-1, 
                            scattering->m[row], scattering->nb_columns-1);
        }
      }
    }
    statement = statement->next;
  }
  beta_parent->size = restore_size;
  clay_array_free(beta_parent);
}


/**
 * clay_beta_shift_after function:
 * Shift all the statements that are after or on the beta vector
 * \param[in] statement     Statements list
 * \param[in] beta          Beta vector 
 * \param[in] depth         Depth level to shift, >= 1
 */
void clay_beta_shift_after(osl_statement_p statement, clay_array_p beta,
                           int depth) {
  if (beta->size == 0)
    return;
  osl_relation_p scattering;
  const int column = (depth-1)*2;  // transition column
  int row;
  int precision = statement->scattering->precision;
  int restore_size = beta->size;
  
  clay_array_p beta_parent = clay_array_clone(beta);
  beta_parent->size = depth-1;
  
  while (statement != NULL) {
    if (clay_statement_is_after(statement, beta)) {
      scattering = statement->scattering;
      if (column < scattering->nb_output_dims) {
        row = clay_statement_get_line(statement, column);
        osl_int_increment(precision, 
                          scattering->m[row], scattering->nb_columns-1, 
                          scattering->m[row], scattering->nb_columns-1);
      }
    }
    statement = statement->next;
  }
  beta_parent->size = restore_size;
  clay_array_free(beta_parent);
}


/**
 * clay_statement_is_before function:
 * Return true if the statement is before (strictly) the beta
 * \param[in] statement
 * \param[in] beta
 * \return
 */
bool clay_statement_is_before(osl_statement_p statement, clay_array_p beta) {
  osl_relation_p scat = statement->scattering;
  int precision = scat->precision;
  int row;
  int i;
  int end;
  
  if (scat->nb_output_dims >= beta->size*2+1)
    end = beta->size;
  else
    end = (scat->nb_output_dims+1)/2;

  void *tmp;
  tmp = osl_int_malloc(precision);
  
  for (i = 0 ; i < end ; i++) {
    row = clay_statement_get_line(statement, i*2);
    osl_int_add_si(precision,
                   tmp, 0,
                   scat->m[row], scat->nb_columns-1,
                   -beta->data[i]); // -> substraction
    if (osl_int_pos(precision, tmp, 0)) {
      osl_int_free(precision, tmp, 0);
      return 0;
    } else if (osl_int_neg(precision, tmp, 0)) {
      osl_int_free(precision, tmp, 0);
      return 1;
    }
  }
  osl_int_free(precision, tmp, 0);
  return 0;
}


/**
 * clay_statement_is_after function:
 * Return true if statement is after (strictly) the beta
 * \param[in] statement
 * \param[in] beta
 * \return
 */
bool clay_statement_is_after(osl_statement_p statement, clay_array_p beta) {
  return !clay_statement_is_before(statement, beta) && 
         !clay_beta_check(statement, beta);
}


/**
 * clay_beta_check function:
 * check if the current statement corresponds to the beta vector
 * \param[in] statement     Statement to test
 * \param[in] beta          Vector to search
 * \return                  true if correct
 */
bool clay_beta_check(osl_statement_p statement, clay_array_p beta) {
  if (beta->size == 0)
    return 1;
  
  int end;
  if (statement->scattering->nb_output_dims >= beta->size*2+1)
    end = beta->size;
  else
    end = (statement->scattering->nb_output_dims+1)/2;
  
  //if (beta->size*2-1 > statement->scattering->nb_output_dims)
  //  return 0;
  
  osl_relation_p scattering = statement->scattering;
  int last_column = scattering->nb_columns-1;
  int i, j = 1, k;
  int ok;
  int tmp;
  int precision = scattering->precision;
  
  for (k = 0 ; k < end ; k++) {
    ok = 0;
    
    // search the first non zero
    for (i = 0 ; i < scattering->nb_rows ; i++) {
      
      // test if the line is an equality
      if (osl_int_zero(precision, scattering->m[i], 0)) {
        
        // non zero on the column
        if (!osl_int_zero(precision, scattering->m[i], j)) {
          tmp = osl_int_get_si(precision, scattering->m[i], last_column);
          if (tmp != beta->data[k]) {
            return 0;
          }
          
          // line and column are corrects ?
          if (!clay_scattering_check_zero(statement, i, j)) {
            // errors should not happen
            return 0;
          }
          
          ok = 1;
          break;
        }
      }
    }
    
    if (!ok)
      return 0;
    
    j += 2;
  }
  return 1;
}


/**
 * clay_scattering_check_zero function:
 * check if the scattering on the first statement contains only zero at line i 
 * and column j, but not on (i,j), on the first column (equality column), and on
 * the last column.
 * Used to check if the line is a line of a beta vector
 * \param[in] statement
 * \param[in] i
 * \param[in] j
 * \return                  true if correct
 */
bool clay_scattering_check_zero(osl_statement_p statement, int i, int j) {
  osl_relation_p scattering = statement->scattering;
  int precision = scattering->precision;
  int t;
  
  for (t = i+1 ; t < scattering->nb_rows ; t++) {
    if (!osl_int_zero(precision, scattering->m[t], j)) {
      fprintf(stderr, "[Clay] Warning: the scattering is incorrect (column %d)\n", j);
      fprintf(stderr, "[Clay] a non-zero value appear\n");
      osl_statement_p tmp = statement->next;
      statement->next = NULL;
      osl_statement_dump(stderr, statement);
      statement->next = tmp;
      return 0;
    }
  }
  for (t = j+1 ; t < scattering->nb_columns-1 ; t++) {
    if (!osl_int_zero(precision, scattering->m[i], t)) {
      fprintf(stderr, "[Clay] Warning: the scattering is incorrect (line %d)\n", i+1);
      fprintf(stderr, "[Clay] a non-zero value appear\n");
      osl_statement_p tmp = statement->next;
      statement->next = NULL;
      osl_statement_dump(stderr, statement);
      statement->next = tmp;
      return 0;
    }
  }
  
  return 1;
}
