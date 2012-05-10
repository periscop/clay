
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
#include <osl/extensions/scatnames.h>
#include <osl/statement.h>
#include <osl/relation.h>
#include <clay/transformation.h>
#include <clay/array.h>
#include <clay/macros.h>


/**
 * clay_reorder function:
 * Reorders the statements in the loop
 * \param[in] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] order         Array to reorder the statements
 * \return                  Status
 */
int clay_reorder(osl_scop_p scop, 
                  clay_array_p beta_loop, clay_array_p neworder) {
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column = beta_loop->size * 2; // transition column
  int row;
  int i = 0;
  
  statement = clay_beta_find(statement, beta_loop);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_TRANSF_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      if (i >= neworder->size)
        return CLAY_TRANSF_REORDER_ARRAY_TOO_SMALL;
      row = clay_statement_get_line(statement, column);
      scattering = statement->scattering;
      osl_int_set_si(precision, 
                     scattering->m[row], scattering->nb_columns-1,
                     neworder->data[i]);
      i++;
    }
    statement = statement->next;
  }
  
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_reversal function:
 * Reverse the direction of the loop
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \return                  Status
 */
int clay_reversal(osl_scop_p scop, clay_array_p beta) {
  if (beta->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
    
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column = beta->size*2 - 1; // iterator column
  int row;
  int i, begin, end;
  void *matrix_row;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  
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
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_interchange function:
 * On each statement which belongs to the `beta', the loops that match the
 * `depth_1'th and the `depth_2' are interchanged
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] depth_1       >= 1
 * \param[in] depth_2       >= 1
 * \return                  Status
 */
int clay_interchange(osl_scop_p scop, 
                      clay_array_p beta, int depth_1, int depth_2) {
  if (beta->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if (depth_1 <= 0 || depth_2 <= 0 || 
      depth_1 > beta->size || depth_2 > beta->size)
    return CLAY_TRANSF_DEPTH_OVERFLOW;
  if (depth_1 == depth_2)
    return CLAY_TRANSF_SUCCESS;
    
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
    return CLAY_TRANSF_BETA_NOT_FOUND;
    
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta)) {
      scattering = statement->scattering;

      //nb_rows = scattering->nb_rows;
      //if (column_1 >= nb_rows || column_2 >= nb_rows)
      //  return CLAY_TRANSF_DEPTH_OVERFLOW;
      
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
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_fission function:
 * Split the loop in two parts at the `depth'th level from the statement
 * \param[in] scop
 * \param[in] beta          Beta vector
 * \param[in] depth         >= 1
 * \return                  Status
 */
int clay_fission(osl_scop_p scop, clay_array_p beta, int depth) {
  if (beta->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if (beta->size <= 1 || depth <= 0 || depth >= beta->size)
    return CLAY_TRANSF_DEPTH_OVERFLOW;
  
  osl_statement_p statement = scop->statement;
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  
  clay_beta_shift(scop->statement, beta, depth);
  
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_fuse function:
 * Fuse loop with the first loop after
 * \param[in] scop
 * \param[in] beta_vector   Loop beta vector
 * \return                  Status
 */
int clay_fuse(osl_scop_p scop, clay_array_p beta_loop) {
  if (beta_loop->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int loop_level = beta_loop->data[beta_loop->size-1];
  const int column = beta_loop->size*2; // transition column
  int row;
  void *vbetamax;
  
  statement = clay_beta_find(statement, beta_loop);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_TRANSF_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  vbetamax = clay_beta_max_value(statement, beta_loop);
  
  statement = scop->statement;
  beta_loop->data[beta_loop->size-1]++; // TODO not ++
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      scattering = statement->scattering;
      if (column < scattering->nb_output_dims) {
        row = clay_statement_get_line(statement, column-2);
        osl_int_set_si(precision, 
                       scattering->m[row], scattering->nb_columns-1, 
                       loop_level);

        row = clay_statement_get_line(statement, column);
        osl_int_add(precision,
                    scattering->m[row], scattering->nb_columns-1,
                    scattering->m[row], scattering->nb_columns-1,
                    vbetamax, 0);
      }
    }
    statement = statement->next;
  }
  beta_loop->data[beta_loop->size-1]--;
  osl_int_free(precision, vbetamax, 0);
  
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_skew function:
 * Skew the loop from the `depth'th loop
 * (i, j) -> (i, j+i*coeff) where `depth' is the loop of i
 * \param[in] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] depth         >= 1
 * \param[in] coeff         != 0
 * \return                  Status
 */
int clay_skew(osl_scop_p scop, 
               clay_array_p beta_loop, int depth, int coeff) {
  if (beta_loop->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if (depth <= 0)
    return CLAY_TRANSF_DEPTH_OVERFLOW;
  if (coeff == 0)
    return CLAY_TRANSF_WRONG_COEFF;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  int precision;
  const int column_depth = depth*2 - 1; // iterator column
  const int column_beta = beta_loop->size*2 - 1; // iterator column
  int row_depth, row_beta;
  int i;
  void **matrix;
  
  statement = clay_beta_find(statement, beta_loop);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_TRANSF_NOT_BETA_LOOP;
  
  precision = statement->scattering->precision;
  // TODO NOTE : we can optimize to not check twice this statement
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      scattering = statement->scattering;
      if (column_depth >= scattering->nb_output_dims)
        return CLAY_TRANSF_DEPTH_OVERFLOW;
      
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
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_iss function:
 * Split the loop (or statement) depending of an inequation
 * (i, j) -> (i, j+i*coeff) where `depth' is the loop of i
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] equation array
 * \return                  Status
 */
int clay_iss(osl_scop_p scop, 
             clay_array_p beta, clay_array_p equ) {
  if (beta->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if(equ->size <= 1)
    return CLAY_TRANSF_SUCCESS;
  
  osl_relation_p scattering;
  osl_statement_p statement;
  osl_statement_p newstatement;
  const int column = (beta->size-1)*2;
  int precision;
  int i, j;
  int row;
  int equ_nb_input_dims, equ_nb_parameters;
  void *order;
  
  // we need the first this because we need to get the nb_input_dims and 
  // nb_parameters for the equation
  statement = clay_beta_first_statement(scop->statement, beta);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (statement->scattering->nb_input_dims == 0)
    return CLAY_TRANSF_BETA_NOT_IN_A_LOOP;
  
  precision = statement->scattering->precision;
  
  clay_beta_shift(scop->statement, beta, beta->size);
  
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
        statement = newstatement;
      }
    }
    statement = statement->next;
  }
  osl_int_free(precision, order, 0);
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_stripmine function:
 * Decompose a single loop into two nested loop
 * \param[in] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] block         Size of the inner loop
 * \param[in] pretty        If true, clay will keep the variables name
 *                          /!\ It takes much more computing 
 * \return                  Status
 */
int clay_stripmine(osl_scop_p scop, clay_array_p beta, int block, int pretty) {
  if (beta->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if (block <= 0)
    return CLAY_TRANSF_WRONG_BLOCK_SIZE;
  
  osl_relation_p scattering;
  osl_statement_p statement = scop->statement;
  osl_scatnames_p scat;
  osl_strings_p names;
  int column = (beta->size-1)*2;
  int precision;
  int row, row_next;
  int iter_column;
  int alea; // for generate a new variable name
  int i;
  int nb_strings;
  char buffer[OSL_MAX_STRING];
  char *new_var_iter;
  char *new_var_beta;
  
  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (statement->scattering->nb_output_dims < 3)
    return CLAY_TRANSF_BETA_NOT_IN_A_LOOP;
  
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
  
  // insert the new variable string
  scat = osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
  names = scat->names;
  
  alea = rand();
  nb_strings = osl_strings_size(names) + 2;
  
  sprintf(buffer, "__%s%s%d", names->string[column+1], names->string[column+1], 
          alea);
  new_var_iter = strdup(buffer);
  sprintf(buffer, "__b%d", alea);
  new_var_beta = strdup(buffer);
  
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
  newnames->string[i] = NULL;
  free(names->string);
  free(names);
  scat->names = newnames;
  
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_unroll function:
 * Unroll a loop 
 * \param[in] scop
 * \param[in] beta_loop     Loop beta vector
 * \param[in] factor        > 0
 * \return                  Status
 */
int clay_unroll(osl_scop_p scop, clay_array_p beta_loop, int factor) {
  
  /*
   *  WIP
   */  
  
  if (beta_loop->size == 0)
    return CLAY_TRANSF_BETA_EMPTY;
  if (factor < 1)
    return CLAY_TRANSF_WRONG_FACTOR;
  
  osl_relation_p scattering;
  osl_statement_p statement;
  osl_statement_p newstatement;
  int column = beta_loop->size*2;
  int precision;
  int row;
  int nb_stmts;
  int i;
  int max;
  int order;
  int current_stmt = 1;
  void *vbetamax;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_TRANSF_BETA_NOT_FOUND;
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims)
    return CLAY_TRANSF_NOT_BETA_LOOP;

  nb_stmts = clay_beta_nb_statements(statement, beta_loop);
  precision = statement->scattering->precision;  
  vbetamax = clay_beta_max_value(statement, beta_loop);
  max = osl_int_get_si(precision, vbetamax, 0);
  osl_int_free(precision, vbetamax, 0);
  
  statement = scop->statement;
  while (statement != NULL) {
    scattering = statement->scattering;
    if (clay_beta_check(statement, beta_loop)) {
      row = clay_statement_get_line(statement, column);
      for (i = 0 ; i < factor-1 ; i++) {
        order = current_stmt + max + nb_stmts*i;
        newstatement = osl_statement_nclone(statement, 1);
        scattering = newstatement->scattering;
        osl_int_set_si(precision,
                       scattering->m[row], scattering->nb_columns-1,
                       order);
        newstatement->next = statement->next;
        statement->next = newstatement;
        statement = newstatement;
      }
      current_stmt++;
    }
    statement = statement->next;
  }
  
  return CLAY_TRANSF_SUCCESS;
}


/**
 * clay_beta_max_value function:
 * Return the last value of the beta max
 * \param[in] statement     List of statements
 * \param[in] beta_loop     Loop beta vector
 * \return
 */
void* clay_beta_max_value(osl_statement_p statement, clay_array_p beta_loop) {
  if (!statement)
    return NULL;
  if (!clay_beta_check(statement, beta_loop))
    statement = clay_beta_find(statement, beta_loop);
  if (!statement)
    return NULL;
  
  osl_relation_p scattering = statement->scattering;
  const int column = beta_loop->size*2; // transition column after the beta
  int precision = scattering->precision;
  int row;
  void *tmp;
  void *vbetamax = osl_int_malloc(precision);
  
  row = clay_statement_get_line(statement, column);
  osl_int_assign(scattering->precision, 
                 vbetamax, 0,
                 scattering->m[row], scattering->nb_columns-1);
  
  if (beta_loop->size*2-1 >= statement->scattering->nb_output_dims) {
    // it's a statement, return the last value
    return vbetamax;
  }
  
  tmp = osl_int_malloc(precision);
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_loop)) {
      scattering = statement->scattering;
      // tmp = current_order - vbetamax
      row = clay_statement_get_line(statement, column);
      osl_int_sub(precision,
                  tmp, 0,
                  scattering->m[row], scattering->nb_columns-1,
                  vbetamax, 0);
      // current_order > vbetamax   (-> tmp > 0)
      if (osl_int_pos(precision, tmp, 0)) {
        osl_int_assign(precision, 
                       vbetamax, 0,
                       scattering->m[row], scattering->nb_columns-1);
      }
    }
    statement = statement->next;
  }
  osl_int_free(precision, tmp, 0);
  return vbetamax;
}

/**
 * clay_statement_get_line function:
 * Because the lines in the scattering matrix may have not ordered, we have to
 * search the corresponding line. It returns the first line where the value is
 * different from zero in the `column'
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
 * clay_beta_find function:
 * Return the first statement which corresponds to the beta vector
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
 * Return the first statement which is at the same level of the beta
 * (not in a sub loop)
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
      if (scattering->nb_output_dims == beta->size*2-1 || 
          scattering->nb_output_dims == beta->size*2+1) {
        break;
      }
    }
    statement = statement->next;
  }
  return statement;
}


/**
 * clay_beta_nb_statements function:
 * \param[in] statement     Statements list
 * \param[in] beta          Vector to search
 * \return
 */
int clay_beta_nb_statements(osl_statement_p statement, 
                            clay_array_p beta) {
  int n = 0;
  while (statement != NULL) {
    if (clay_beta_check(statement, beta))
      n++;
    statement = statement->next;
  }
  return n;
}


/**
 * clay_beta_shift function:
 * Shift all the statements which are after or on the beta vector
 * \param[in] statement     Statements list
 * \param[in] beta          Beta vector 
 * \param[in] depth         Depth level to shift
 */
void clay_beta_shift(osl_statement_p statement, clay_array_p beta, int depth) {
  if (beta->size == 0)
    return;
  osl_relation_p scattering;
  const int column = (depth-1)*2;  // transition column
  int row;
  int precision = statement->scattering->precision;
  int restore_size = beta->size;
  
  clay_array_p beta_parent = clay_array_clone(beta);
  (beta_parent)->size = depth-1; // beta before the loop at the `depth'th level
  
  while (statement != NULL) {
    if (clay_beta_check(statement, beta_parent) && 
        !clay_beta_check(statement, beta)) {
      if (clay_statement_is_after(statement, beta)) {
        scattering = statement->scattering;
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
 * clay_statement_is_after function:
 * Return true if statement is after the beta
 * \param[in] statement
 * \param[in] beta
 * \return
 */
bool clay_statement_is_after(osl_statement_p statement, clay_array_p beta) {
  osl_relation_p scat = statement->scattering;
  int precision = scat->precision;
  int row;
  int i;
  int end = (scat->nb_output_dims > beta->size*2+1 ? 
             beta->size :
             (scat->nb_output_dims+1)/2);
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
      return 1;
    } else if (osl_int_neg(precision, tmp, 0)) {
      osl_int_free(precision, tmp, 0);
      return 0;
    }
  }
  osl_int_free(precision, tmp, 0);
  return 0;
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
  if (beta->size*2-1 > statement->scattering->nb_output_dims)
    return 0;
  
  osl_relation_p scattering = statement->scattering;
  int last_column = scattering->nb_columns-1;
  int i, j = 1, k, t;
  int ok;
  int tmp;
  int precision = scattering->precision;
  
  for (k = 0 ; k < beta->size ; k++) {
    ok = 0;
    for (i = 0 ; i < scattering->nb_rows ; i++) {
      // test if the line is an equality
      if (osl_int_zero(precision, scattering->m[i], 0)) {
        if (!osl_int_zero(precision, scattering->m[i], j)) {
          tmp = osl_int_get_si(precision, scattering->m[i], last_column);
          if (tmp != beta->data[k]) {
            return 0;
          }
          
          // check if the column and the line are correct
          // errors should not happen
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
          for (t = j+1 ; t < last_column ; t++) {
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
