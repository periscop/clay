
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
#include <osl/generic.h>
#include <osl/extensions/scatnames.h>
#include <osl/extensions/arrays.h>
#include <osl/extensions/extbody.h>

#include <clay/transformation.h>
#include <clay/array.h>
#include <clay/list.h>
#include <clay/macros.h>
#include <clay/options.h>
#include <clay/errors.h>
#include <clay/beta.h>
#include <clay/util.h>
#include <clay/relation.h>


/*****************************************************************************\
 *                     Loop transformations                                   *
 `****************************************************************************/

/*
   Many functions here have the common structure that may be extracted into
   a separate higher-order function.

  if (!scop || !scop->statement || !scop->statement->scattering)
     return CLAY_SUCCESS;

  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  precision = statement->scattering->precision;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (!clay_beta_check_relation(scattering, beta)) {
        scattering = scattering->next;
        continue;
      }
      CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
      // Do staff.
      scattering = scattering->next;
    }
    statement = statement->next;
  }

 */

// asssumes betas are normalized
int clay_collapse(osl_scop_p scop, clay_array_p beta, clay_options_p options) {
  int i, row, col, row_1, row_2;
  int candidate_row_1, candidate_row_2;
  int nb_pairs;
  clay_array_p first_beta, second_beta, max_beta;
  clay_array_p matched_rows_2;
  clay_list_p matching_betas = clay_list_malloc();
  osl_statement_p statement, first_statement, second_statement;
  osl_relation_p scattering, s1, s2;

  (void) options; // prevent `unused' warning

  if (!scop || !scop->statement || !scop->statement->scattering)
     return CLAY_SUCCESS;

  clay_scop_normalize(scop);

  // Find all betas-vectors matching the given prefix.
  statement = scop->statement;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (!clay_beta_check_relation(scattering, beta)) {
        scattering = scattering->next;
        continue;
      }
      clay_list_add(matching_betas, clay_beta_extract(scattering));
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  // Every relation should have a pair to collapse it with.
  if ((matching_betas->size % 2) != 0 || matching_betas->size == 0) {
    clay_list_free(matching_betas);
    return CLAY_ERROR_BETA_NOT_FOUND;
  }


  clay_beta_list_sort(matching_betas);
  max_beta = clay_beta_max(scop->statement, beta);
  if (max_beta->size <= beta->size) {
    clay_array_free(max_beta);
    clay_list_free(matching_betas);
    return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
  }
  nb_pairs = (max_beta->data[beta->size] + 1) / 2;// assume betas are normalized
  clay_array_free(max_beta);

  // For each matching beta, find its counterpart by going through half of the
  // sorted beta list (counterpats are also matching) and check if both parts
  // can collapse to one.
  for (i = 0; i < matching_betas->size / 2; i++) {
    first_beta = matching_betas->data[i];
    if (first_beta->size == beta->size) { // Collpase works on loop level.
      clay_list_free(matching_betas);
      return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
    }
    second_beta = clay_array_clone(first_beta);
    second_beta->data[beta->size] += nb_pairs;


    // Both betas should belong to the same statement.
    first_statement = clay_beta_find(scop->statement, first_beta);
    second_statement = clay_beta_find(scop->statement, second_beta);
    if (!first_statement || !second_statement ||
        first_statement != second_statement) {
      clay_array_free(second_beta);
      clay_list_free(matching_betas);
      return CLAY_ERROR_BETA_NOT_FOUND;
    }


    s1 = NULL;
    s2 = NULL;
    for (scattering = first_statement->scattering; scattering != NULL;
         scattering = scattering->next) {
      if (clay_beta_check_relation(scattering, first_beta)) {
        s1 = scattering;
      }
      if (clay_beta_check_relation(scattering, second_beta)) {
        s2 = scattering;
      }
    }
    clay_array_free(second_beta);

    // Check scattering parameters are compatible.
    if (!s1 || !s2 ||
        s1->nb_rows != s2->nb_rows ||
        s1->nb_input_dims != s2->nb_input_dims ||
        s1->nb_output_dims != s2->nb_output_dims ||
        s1->nb_local_dims != s2->nb_local_dims ||
        s1->nb_parameters != s2->nb_parameters) {
      clay_list_free(matching_betas);
      return CLAY_ERROR_WRONG_COEFF; // FIXME: local dimensions can be merged
    }

    // Check all equalities are the same (in normalized form, equations come
    // before inequalities, sorted lexicographically so that line by line
    // comparison is possible.
    for (row = 0; row < s1->nb_rows; row++) {
      if (clay_util_is_row_beta_definition(s1, row)) {
        continue; // ignore beta rows that are already compare by beta-matching
      }
      if (osl_int_one(s1->precision, s1->m[row][0])) {
        break; // inequality part started
      }
      for (col = 0; col < s1->nb_columns; col++) {
        if (osl_int_ne(s1->precision, s1->m[row][col], s2->m[row][col])) {
          clay_list_free(matching_betas);
          return CLAY_ERROR_WRONG_COEFF;
        }
      }
    }

    matched_rows_2 = clay_array_malloc();
    candidate_row_1 = -1;
    for (row_1 = row; row_1 < s1->nb_rows; row_1++) {
      int matched_row = -1;
      for (row_2 = row; row_2 < s2->nb_rows; row_2++) {
        int mismatch = 0;

        // Skip if row is already matched.
        if (clay_array_contains(matched_rows_2, row_2)) {
          continue;
        }

        for (col = 0; col < s1->nb_columns; col++) {
          if (osl_int_ne(s1->precision,
                         s1->m[row_1][col], s2->m[row_2][col])) {
            mismatch = 1;
            break;
          }
        }
        if (!mismatch) {
          matched_row = row_2;
          clay_array_add(matched_rows_2, row_2);
          break;
        }
      }

      // If no row matched,
      if (matched_row == -1) {
        if (candidate_row_1 != -1) { // Cannot have two unmatching rows.
          clay_array_free(matched_rows_2);
          clay_list_free(matching_betas);
          return CLAY_ERROR_WRONG_COEFF;
        }
        candidate_row_1 = row_1;
      }
    }

    // Find the single unmatched row in s2.
    candidate_row_2 = -1;
    for (row_2 = row; row_2 < s2->nb_rows; row_2++) {
      if (!clay_array_contains(matched_rows_2, row_2)) {
        candidate_row_2 = row_2;
        break;
      }
    }
    clay_array_free(matched_rows_2);
    if (candidate_row_2 == -1) {
      // XXX: Something went wrong...
      clay_list_free(matching_betas);
      return CLAY_ERROR_WRONG_COEFF;
    }

    // Check if unmatched rows differ up to negation.
    clay_util_relation_negate_row(s2, candidate_row_2);
    for (col = 0; col < s1->nb_columns; col++) {
      if (osl_int_ne(s1->precision, s1->m[candidate_row_1][col],
                                    s2->m[candidate_row_2][col])) {
        clay_list_free(matching_betas);
        return CLAY_ERROR_WRONG_COEFF;
      }
    }
    clay_util_relation_negate_row(s2, candidate_row_2);

    // All preconditions are met, now we may remove the inequality and the
    // second relation.
    osl_relation_remove_row(s1, candidate_row_1);
    osl_relation_remove_part(&second_statement->scattering, s2);
  }

  clay_list_free(matching_betas);
  return CLAY_SUCCESS;
}

// depth, iterator -- 1-based
int clay_reshape(osl_scop_p scop, clay_array_p beta, int depth, int iterator, int amount, clay_options_p options) {
  osl_statement_p statement;
  osl_relation_p scattering, copy;
  int row, precision, output, input;
  osl_int_t summand;
  int nb_explicit_before, nb_explicit_after;

  output = 2*depth;

  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (amount == 0)
    return CLAY_ERROR_WRONG_FACTOR;

  precision = statement->scattering->precision;
  osl_int_init(precision, &summand);
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (!clay_beta_check_relation(scattering, beta)) {
        scattering = scattering->next;
        continue;
      }
      // CLAY_BETA_CHECK_DEPTH(beta, depth, scattering); copied as it does not
      // allow to free resources
      if (beta->size*2-1 >= scattering->nb_output_dims && depth >= beta->size) {
        osl_int_clear(precision, &summand);
        return CLAY_ERROR_DEPTH_OVERFLOW;
      }
      if (depth > beta->size) {
        osl_int_clear(precision, &summand);
        return CLAY_ERROR_DEPTH_OVERFLOW;
      }
      if (iterator <= 0 || iterator > scattering->nb_input_dims) {
        osl_int_clear(precision, &summand);
        return CLAY_ERROR_DEPTH_OVERFLOW;
      }

      copy = osl_relation_clone(scattering);
      nb_explicit_before = clay_relation_nb_explicit_dim_intrusive(copy);

      input = 1 + scattering->nb_output_dims + (iterator - 1);

      for (row = 0; row < copy->nb_rows; row++) {
        osl_int_mul_si(precision, &summand, copy->m[row][output], amount);
        osl_int_add(precision, &copy->m[row][input], copy->m[row][input], summand);
      }

      nb_explicit_after = clay_relation_nb_explicit_dim_intrusive(copy);
      if (nb_explicit_before != nb_explicit_after) {
        osl_int_clear(precision, &summand);
        osl_relation_free(copy);
        return CLAY_ERROR_WRONG_COEFF;
      }

      for (row = 0; row < scattering->nb_rows; row++) {
        osl_int_mul_si(precision, &summand, scattering->m[row][output], amount);
        osl_int_add(precision, &scattering->m[row][input], scattering->m[row][input], summand);
      }

      osl_relation_free(copy);
      scattering = scattering->next;
    }
    statement = statement->next;
  }
  osl_int_clear(precision, &summand);
  return CLAY_SUCCESS;
}

// depth 1-based
int clay_densify(osl_scop_p scop, clay_array_p beta, int depth, clay_options_p options) {
  osl_statement_p statement;
  osl_relation_p scattering;
  int precision;
  osl_int_t factor;
  int row, col;

  if (!scop || !scop->statement || !scop->statement->scattering)
    return CLAY_SUCCESS;

  osl_int_init(scop->statement->scattering->precision, &factor);

  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  precision = statement->scattering->precision;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (!clay_beta_check_relation(scattering, beta)) {
        scattering = scattering->next;
        continue;
      }
      CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);

      factor = clay_relation_gcd(scattering, depth);
      if (osl_int_zero(precision, factor))
        continue;

      for (row = 0; row < scattering->nb_rows; row++) {
        if (osl_int_zero(precision, scattering->m[row][depth * 2])) {
          continue;
        }
        for (col = 2; col < scattering->nb_columns; col++) {
          // if beta, ignore
          if (col >= 1 && col < scattering->nb_output_dims + 1 && (col % 2) == 1) {
            continue;
          }
          // if target dimension, ignore
          if (col == depth * 2) {
            continue;
          }
          osl_int_div_exact(precision, &scattering->m[row][col],
                            scattering->m[row][col], factor);
        }
      }
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  osl_int_clear(precision, &factor);
  return CLAY_SUCCESS;
}

int clay_grain(osl_scop_p scop, clay_array_p beta, int depth, int grain, clay_options_p options) {
  osl_statement_p statement;
  osl_relation_p scattering;
  int precision;

  if (grain <= 0) {
    return CLAY_ERROR_WRONG_FACTOR;
  }

  if (depth <= 0) {
    return CLAY_ERROR_DEPTH_OVERFLOW;
  }

  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  precision = statement->scattering->precision;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (!clay_beta_check_relation(scattering, beta)) {
        scattering = scattering->next;
        continue;
      }
      CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
      // Multiply all lines by the factor except for those defining beta dimensions.
      // Beta-extract from statements expects the coefficient before beta to be -1.
      for (int i = 0; i < scattering->nb_rows; i++) {
        if (clay_util_is_row_beta_definition(scattering, i))
          continue;
        for (int j = 1; j < scattering->nb_columns; j++) {
          // Columns for the given depth should not be changed.
          if (j == 2 * depth)
            continue;
          osl_int_mul_si(precision, &scattering->m[i][j], scattering->m[i][j], grain);
        }
      }

      scattering = scattering->next;
    }
    statement = statement->next;
  }

  return CLAY_SUCCESS;
}

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
  // Replace element in the nested beta-vectors with respect to the values in
  // the neworder array.

  osl_relation_p scattering;
  osl_statement_p statement;
  int precision;
  const int column = beta_loop->size * 2; // relation_get_line adds 1 -> beta
  int row;
  int index;

  // Ensure that beta-vectors are normalized since we use their elements as
  // indices to neworder array.
  clay_beta_normalize(scop);

  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  precision = statement->scattering->precision;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      // Select all the parts of relation union that match current beta.
      if (!clay_beta_check_relation(scattering, beta_loop)) {
        scattering = scattering->next;
        continue;
      }
      CLAY_BETA_IS_LOOP(beta_loop, scattering);

      row = clay_util_relation_get_line(scattering, column);

      // Get the beta value.
      index = osl_int_get_si(precision,
                             scattering->m[row][scattering->nb_columns - 1]);

      if (index >= neworder->size)
        return CLAY_ERROR_REORDER_ARRAY_TOO_SMALL;

      osl_int_set_si(precision,
                     &scattering->m[row][scattering->nb_columns - 1],
                     neworder->data[index]);
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  // Normalize again if asked (useful if the neworder was not normalized).
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
int clay_reverse(osl_scop_p scop, clay_array_p beta, unsigned int depth,
                 clay_options_p options) {
  // Oppose the output_dims column at the `depth'th level.

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

  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      precision = scattering->precision;
      CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
      if (clay_beta_check_relation(scattering, beta)) {
        for (i = 0; i < scattering->nb_rows; i++) {
          osl_int_oppose(precision,
                         &scattering->m[i][column + 1],
                         scattering->m[i][column + 1]);
        }
      }
      scattering = scattering->next;
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
 * \param[in] pretty        1 or 0 : update the scatnames
 * \param[in] options
 * \return                  Status
 */
int clay_interchange(osl_scop_p scop,
                     clay_array_p beta,
                     unsigned int depth_1, unsigned int depth_2,
                     int pretty,
                     clay_options_p options) {
  // Swap the two output_dims columns.

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
  osl_int_t **matrix;

  statement = clay_beta_find(statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  scattering = statement->scattering;
  while (scattering != NULL) {
    if (scattering->nb_output_dims <= 1)
      return CLAY_ERROR_DEPTH_OVERFLOW;
    CLAY_BETA_CHECK_DEPTH(beta, depth_1, scattering);
    CLAY_BETA_CHECK_DEPTH(beta, depth_2, scattering);
    scattering = scattering->next;
  }

  // Swapping with itself doesn't affect the scattering.
  if (depth_1 == depth_2)
    return CLAY_SUCCESS;

  precision = statement->scattering->precision;
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (clay_beta_check_relation(scattering, beta)) {
        // Swap the two columns.
        matrix = scattering->m;
        for (i = 0 ; i < scattering->nb_rows ; i++)
          osl_int_swap(precision, 
                       &matrix[i][column_1+1],
                       &matrix[i][column_2+1]);
      }
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  // swap the two variables
  if (pretty) {
    osl_scatnames_p scat;
    osl_strings_p names;
    scat = osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
    names = scat->names;

    int ii = 0;
    for (ii = 0 ; names->string[ii] ; ii++)
      ;

    int c1 = depth_1 * 2 - 1;
    int c2 = depth_2 * 2 - 1;

    if (c1 < ii && c2 < ii) {
      char *tmp = names->string[c1];
      names->string[c1] = names->string[c2];
      names->string[c2] = tmp;
    }
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
int clay_split(osl_scop_p scop, clay_array_p beta, unsigned int depth,
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
  const int column = beta_loop->size*2; // alpha column
  int row;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  scattering = statement->scattering;
  while (scattering != NULL) {
    if (clay_beta_check_relation(scattering, beta_loop)) {
      CLAY_BETA_IS_LOOP(beta_loop, scattering);
    }
    scattering = scattering->next;
  }

  precision = statement->scattering->precision;
  
  beta_max = clay_beta_max(statement, beta_loop);
  beta_next = clay_beta_next_part(scop->statement, beta_loop);
  
  if (beta_next != NULL) {
    beta_next->size = depth;
    statement = scop->statement;
    while (statement != NULL) {
      scattering = statement->scattering;
      while (scattering != NULL) {
        if (clay_beta_check_relation(scattering, beta_next)) {
          if (column < scattering->nb_output_dims) {
            // Set the loop level
            row = clay_util_relation_get_line(scattering, column-2);
            osl_int_set_si(precision,
                           &scattering->m[row][scattering->nb_columns-1],
                           beta_loop->data[depth-1]);

            // Reorder the statement
            row = clay_util_relation_get_line(scattering, column);
            osl_int_add_si(precision,
                           &scattering->m[row][scattering->nb_columns-1],
                           scattering->m[row][scattering->nb_columns-1],
                           beta_max->data[depth]+1);
          }
        }
        scattering = scattering->next;
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
 * Skew the loop, i.e. add an outer iterator of the loop nest multiplied by the
 * coefficient to the current loop iterator.  Use the beta-prefix length to
 * identify the "target" iteartor, use #depth to identify the outer loop
 * ("source") iterator.
 * (i, j) -> (i, j+i*coeff).
 * Skew applies only to loops by definition.  To perform the skewing
 * transformation for a statement, split it and then do the skewing, or call
 * shift directly.
 * \param[in,out] scop
 * \param[in] beta          Beta-prefix that identifies the loop to skew
 * \param[in] depth         Depth of the "source" loop iterator (between 1 and length of beta-prefix)
 * \param[in] coeff         Coefficient for the "source" iterator.
 * \param[in] options       Clay options
 * \return                  Status
 */
int clay_skew(osl_scop_p scop, 
              clay_array_p beta, unsigned int depth, unsigned int coeff,
              clay_options_p options) {

  /* Description:
   * This is a special case of shifting, where params and constant
   * are equal to zero.
   *
   * Call clay_shift with the same beta, depth = beta_size and a vector set up
   * so that it corresponds to 1*i + coeff*j shifting, i.e. it contains the
   * value coeff at depth-s index and 1 at the end, all the other elements
   * being zero.
   *
   * Individual statements cannot be skewed because we do not have information.
   * The length of the beta-prefix is used to indentify the loop to skew, which
   * would require an addiitonal parameter in case of beta-vector for
   * statement.
   */

  if (beta->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (depth <= 0 || depth >= beta->size)
    return CLAY_ERROR_DEPTH_OVERFLOW;
  if (coeff == 0)
    return CLAY_ERROR_WRONG_COEFF;

  clay_list_p vector;
  int i, ret;
  osl_statement_p statement;
  osl_relation_p scattering;

  // Check if beta matches only loops.
  statement = clay_beta_find(scop->statement, beta);
  while (statement != NULL) {
    for (scattering = statement->scattering;
         scattering != NULL;
         scattering = scattering->next) {
      if (!clay_beta_check_relation(scattering, beta))
        continue;
      CLAY_BETA_IS_LOOP(beta, scattering);
    }
    statement = statement->next;
  }

  // create the vector
  vector = clay_list_malloc();

  // empty arrays
  clay_list_add(vector, clay_array_malloc());
  clay_list_add(vector, clay_array_malloc());
  clay_list_add(vector, clay_array_malloc());

  // set output dims
  for (i = 0 ; i < depth-1 ; i++)
    clay_array_add(vector->data[0], 0);
  clay_array_add(vector->data[0], coeff);
  for (i = depth + 1; i < beta->size; i++)
    clay_array_add(vector->data[0], 0);
  clay_array_add(vector->data[0], 1);

  ret = clay_shift(scop, beta, beta->size, vector, options);
  clay_list_free(vector);

  return ret;
}


/**
 * clay_iss function:
 * Split the loop (or statement) depending of an inequation.
 * Warning: in the output part, don't put the alpha columns
 *          example: if the output dims are : 0 i 0 j 0, just do Ni, Nj
 * \param[in,out] scop
 * \param[in]  beta_loop         Beta loop
 * \param[in]  inequation array  {(([output, ...],) [param, ...],) [const]}
 * \param[out] beta_max          If NULL, the beta_max will not be returned.
 *                               If function terminated successfully, the last
 *                               beta which has the prefix beta_loop, NULL
 *                               otherwise
 * \param[in]  options
 * \return                       Status
 */
int clay_iss(osl_scop_p scop, 
             clay_array_p beta_loop, clay_list_p inequ,
             clay_array_p *ret_beta_max,
             clay_options_p options) {

  /* Description:
   * Add the inequality to each scattering relation union that corresponds to
   * the given beta.
   */

  if (beta_loop->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (inequ->size > 3)
    return CLAY_ERROR_INEQU;

  osl_statement_p statement;
  osl_relation_p scattering, clone;
  clay_array_p beta_max, source_beta;
  int nb_output_dims, nb_parameters;
  int order; // new loop order for the clones
  
  // search a statement
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;
  if (statement->scattering->nb_input_dims == 0)
    return CLAY_ERROR_BETA_NOT_IN_A_LOOP;

  // decompose the inequation
  nb_parameters = scop->context->nb_parameters;
  nb_output_dims = statement->scattering->nb_output_dims;

  if (inequ->size == 3) {
    // pad zeros
    clay_util_array_output_dims_pad_zero(inequ->data[0]);

    if (inequ->data[0]->size > nb_output_dims ||
        inequ->data[1]->size > nb_parameters ||
        inequ->data[2]->size > 1)
      return CLAY_ERROR_INEQU;

  } else if (inequ->size == 2) {
    if (inequ->data[0]->size > nb_parameters ||
        inequ->data[1]->size > 1)
      return CLAY_ERROR_INEQU;

  } else if (inequ->size == 1) {
    if (inequ->data[0]->size > 1)
      return CLAY_ERROR_INEQU;
  }

  // set the beginning of 'order'
  beta_max = clay_beta_max(statement, beta_loop);
  order = beta_max->data[beta_loop->size] + 1; // beta_loop->size is in range
                                               // since loop contains statements
  if (ret_beta_max != NULL) {
    *ret_beta_max = beta_max;
  }

  // ensure ret_beta_max is set up before returning success
  if (inequ->size == 0) {
    clay_array_free(beta_max);
    return CLAY_SUCCESS;
  }

  // For each statement that matches
  for ( ; statement != NULL; statement = statement->next) {
    if (clay_beta_check(statement, beta_loop)) {
    // Only for those scattering relations that match
      for (scattering = statement->scattering; scattering != NULL;
           scattering = scattering->next) {
        if (clay_beta_check_relation(scattering, beta_loop)) {
          CLAY_BETA_IS_LOOP(beta_loop, scattering);
          // TODO extract variables to the beginning
          // Clone the only the required relation part and insert inequation to
          // both the cloned and the original relation wrt negation.
          clone = osl_relation_nclone(scattering, 1);
          clay_util_relation_insert_inequation(scattering, inequ);
          clay_util_relation_insert_inequation(clone, inequ);
          clay_util_relation_negate_row(clone, clone->nb_rows - 1);

          // Update the beta of the original relation (matching the given
          // condition) so that it comes after any other beta that matches the
          // given beta-prefix and the relative order of statements is
          // preserved.
          source_beta = clay_beta_extract(scattering);
          source_beta->data[beta_loop->size] += order;
          clay_util_scattering_update_beta(scattering, source_beta);

          // Insert the cloned relation to the relation list, and skip it in
          // the loop since it was not in the original scop.
          clone->next = scattering->next;
          scattering->next = clone;
          scattering = clone;
        }
      }
    }
  }

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
int clay_stripmine(osl_scop_p scop, clay_array_p beta, 
                   unsigned int depth, unsigned int size, 
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
  int column = (depth-1)*2; // alpha column
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
  
  precision = statement->scattering->precision;
  if (pretty)
    statement = scop->statement; // TODO optimization...
        
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (clay_beta_check_relation(scattering, beta)) {
        CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);

        // set the strip mine
        row = clay_util_relation_get_line(scattering, column);

        osl_relation_insert_blank_column(scattering, column+1);
        osl_relation_insert_blank_column(scattering, column+1);

        osl_relation_insert_blank_row(scattering, column);
        osl_relation_insert_blank_row(scattering, column);
        osl_relation_insert_blank_row(scattering, column);

        // stripmine
        osl_int_set_si(precision, &scattering->m[row+0][column+1], -1);
        osl_int_set_si(precision, &scattering->m[row+1][column+2], -size);
        osl_int_set_si(precision, &scattering->m[row+2][column+2], size);
        osl_int_set_si(precision,
                       &scattering->m[row+2][scattering->nb_columns-1],
                       size-1);

        // inequality
        osl_int_set_si(precision, &scattering->m[row+1][0], 1);
        osl_int_set_si(precision, &scattering->m[row+2][0], 1);

        // iterator dependance
        osl_int_set_si(precision, &scattering->m[row+1][column+4], 1);
        osl_int_set_si(precision, &scattering->m[row+2][column+4], -1);

        scattering->nb_output_dims += 2;

        // reorder
        row_next = clay_util_relation_get_line(scattering, column+2);
        osl_int_assign(precision,
                       &scattering->m[row][scattering->nb_columns-1],
                       scattering->m[row_next][scattering->nb_columns-1]);

        osl_int_set_si(precision,
                       &scattering->m[row_next][scattering->nb_columns-1],
                       0);

      } else if (pretty && column < scattering->nb_output_dims) {
        // add 2 empty dimensions
        row = clay_util_relation_get_line(scattering, column);

        osl_relation_insert_blank_column(scattering, column+1);
        osl_relation_insert_blank_column(scattering, column+1);

        osl_relation_insert_blank_row(scattering, row);
        osl_relation_insert_blank_row(scattering, row);

        // -Identity
        osl_int_set_si(precision, &scattering->m[row][column+1], -1);
        osl_int_set_si(precision, &scattering->m[row+1][column+2], -1);

        scattering->nb_output_dims += 2;

        // reorder
        row_next = clay_util_relation_get_line(scattering, column+2);
        osl_int_assign(precision,
                       &scattering->m[row][scattering->nb_columns-1],
                       scattering->m[row_next][scattering->nb_columns-1]);

        osl_int_set_si(precision,
                       &scattering->m[row_next][scattering->nb_columns-1],
                       0);
      }

      scattering = scattering->next;
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
  
  for (i = 0 ; i < column ; i++)
    newnames->string[i] = names->string[i];
  
  newnames->string[i] = new_var_beta;
  newnames->string[i+1] = new_var_iter;
  
  for (i = i+2 ; i < nb_strings ; i++)
    newnames->string[i] = names->string[i-2];
  
  newnames->string[i] = NULL; // end of the list
  
  // replace the scatnames
  free(names->string);
  free(names);
  scat->names = newnames;
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/** clay_unroll function: Unroll a loop \param[in,out] scop \param[in]
 * beta_loop     Loop beta vector \param[in] factor        > 0 \param[in]
 * setepilog     if true the epilog will be added \param[in] options \return
 * Status
 */
int clay_unroll(osl_scop_p scop, clay_array_p beta_loop, unsigned int factor,
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
  
  osl_body_p body = NULL;
  osl_extbody_p ext_body = NULL;
  osl_body_p newbody = NULL;
  osl_extbody_p newextbody = NULL;
  osl_body_p tmpbody = NULL;
  osl_generic_p gen = NULL;
  char *expression;
  char **iterator;
  char *substitued;
  char *newexpression;
  char *replacement;
  char substitution[] = "@0@";
  
  int iterator_index = beta_loop->size-1;
  int iterator_size;

  int is_extbody = 0;
  
  statement = clay_beta_find(scop->statement, beta_loop);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

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
    
    if (clay_beta_check(statement, beta_loop)) {
      
      // create the body with symbols for the substitution
      original_stmt = statement;

      ext_body = osl_generic_lookup(statement->extension,
                                    OSL_URI_EXTBODY);
      if (ext_body) {
        body = ext_body->body;
        is_extbody = 1;
      } else {
        body = (osl_body_p) osl_generic_lookup(statement->extension,
                                               OSL_URI_BODY);
      }
      if (body == NULL)
        CLAY_error("Missing statement body\n");

      expression = body->expression->string[0];
      iterator[0] = (char*) body->iterators->string[iterator_index];
      iterator_size = strlen(iterator[0]);
      substitued = osl_util_identifier_substitution(expression, iterator);
      
      CLAY_malloc(replacement, char*, 1 + iterator_size + 1 + 16 + 1 + 1);
      
      if (setepilog) {
        // Clone the statment with only those parts of scattering relation
        // union, that match the given beta, and modify these parts
        // accordingly.
        epilog_stmt = osl_statement_malloc();
        epilog_stmt->domain = osl_relation_clone(original_stmt->domain);
        epilog_stmt->access = osl_relation_list_clone(original_stmt->access);
        epilog_stmt->extension = osl_generic_clone(original_stmt->extension);
        epilog_stmt->scattering = NULL;
        epilog_stmt->next = NULL;
        osl_relation_p scat = original_stmt->scattering;
        osl_relation_p ptr = NULL;
        while (scat != NULL) {
          if (clay_beta_check_relation(scat, beta_loop)) {
            CLAY_BETA_IS_LOOP(beta_loop, scat);
            if (ptr == NULL) {
              epilog_stmt->scattering = osl_relation_nclone(scat, 1);
              ptr = epilog_stmt->scattering;
            } else {
              ptr->next = osl_relation_nclone(scat, 1);
              ptr = ptr->next;
            }

            row = clay_util_relation_get_line(scat, column - 2);
            osl_int_set_si(precision,
                           &ptr->m[row][ptr->nb_columns-1],
                           order_epilog);

        //# AZ: fixing up post-iss changes to scattering union, that would be
        //translated into loop boundary changes, namely removing presumably
        //lower bound (FIXME!) conditions on the alpha dimension corresponding
        //to the unrolled iterator.  Other possible solution: take the lower
        //bound from unrolled statements, add +1 and transformed to the lower
        //bound for epilog.
            for (i = 0; i < ptr->nb_rows; i++) {
              if (osl_int_pos(precision, ptr->m[i][(iterator_index+1)*2])) {
                osl_int_set_si(precision, &ptr->m[i][(iterator_index+1)*2], 0);
              }
              int j, to_remove = 1;
              for (j = 0; j < ptr->nb_output_dims; j++) {
                if (!osl_int_zero(precision, ptr->m[i][j+1])) {
                  to_remove = 0;
                  break;
                }
              }
              if (to_remove) {
                osl_relation_remove_row(ptr, i);
                i--;
              }
            }
          }
          scat = scat->next;
        }

        // the order is not important in the statements list
        epilog_stmt->next = statement->next;
        statement->next = epilog_stmt;
        statement = epilog_stmt;
      }
      
      // modify the matrix domain
      domain = original_stmt->domain;
      
      if (setepilog) {
        epilog_domain = epilog_stmt->domain;
      }
      
      while (domain != NULL) {
      
        for (i = domain->nb_rows-1 ; i >= 0  ; i--) {
          if (!osl_int_zero(precision, domain->m[i][0])) {
          
            // remove the lower bound on the epilog statement
            if(setepilog &&
               osl_int_pos(precision, domain->m[i][iterator_index+1])) {
              osl_relation_remove_row(epilog_domain, i);
            }
            // remove the upper bound on the original statement
            if (osl_int_neg(precision, domain->m[i][iterator_index+1])) {
              osl_int_add_si(precision, 
                             &domain->m[i][domain->nb_columns-1],
                             domain->m[i][domain->nb_columns-1],
                             -factor);
            }
          }
        }
        
        // Don't add local dimension to the domain since it may affect other
        // parts of the statement scattered with different betas.
        
        domain = domain->next;
        
        if (setepilog)
          epilog_domain = epilog_domain->next;
      }

      // clone factor-1 times the original statement
     
      // FIXME: here scattering was pointing to the scattering of epilog (probably bug)
      // FIXME: I don't see much sense in this magic: it sets current_stmt to 1
      // (instead of 0) unless the last value of the beta-vector for epilog (if
      // requested) or original statement was -1.  Leave it as is.
      row = clay_util_relation_get_line(original_stmt->scattering, column);
      current_level = osl_int_get_si(scattering->precision,
                                 scattering->m[row][scattering->nb_columns-1]);
      if (last_level != current_level) {
        current_stmt++;
        last_level = current_level;
      }

      // Update those scattering relation union
      scattering = original_stmt->scattering;
      while (scattering != NULL) {
        if (clay_beta_check_relation(scattering, beta_loop)) {
          // Insert local dimension.
          osl_relation_insert_blank_column(scattering, scattering->nb_output_dims + scattering->nb_input_dims + 1);
          (scattering->nb_local_dims)++;
          osl_relation_insert_blank_row(scattering, scattering->nb_rows);
          osl_int_set_si(scattering->precision,
                         &scattering->m[scattering->nb_rows - 1][(iterator_index+1)*2],
                         1);
          osl_int_set_si(scattering->precision,
                         &scattering->m[scattering->nb_rows - 1][scattering->nb_output_dims + scattering->nb_input_dims + 1],
                         -factor);
        }
        scattering = scattering->next;
      }
      
      for (i = 0 ; i < factor-1 ; i++) {
        // Clone the expression with only matching parts.
        newstatement = osl_statement_malloc();
        newstatement->domain = osl_relation_clone(original_stmt->domain);
        newstatement->access = osl_relation_list_clone(original_stmt->access);
        newstatement->extension = osl_generic_clone(original_stmt->extension);
        newstatement->next = NULL;
        newstatement->scattering = NULL;
        osl_relation_p ptr = NULL;
        osl_relation_p scat = original_stmt->scattering;
        while (scat != NULL) {
          if (clay_beta_check_relation(scat, beta_loop)) {
            if (ptr == NULL) {
              newstatement->scattering = osl_relation_nclone(scat, 1);
              ptr = newstatement->scattering;
            } else {
              ptr->next = osl_relation_nclone(scat, 1);
              ptr = ptr->next;
            }
          }
          scat = scat->next;
        }

        scattering = newstatement->scattering;
        // reorder
        while (scattering != NULL) {
          order = current_stmt + max + nb_stmts*(i + 1) + 1;
          osl_int_set_si(precision,
                         &scattering->m[row][scattering->nb_columns-1],
                         order);
          scattering = scattering->next;
        }

        // update the body
        sprintf(replacement, "(%s+%d)", iterator[0], i+1);
        newexpression = clay_util_string_replace(substitution, replacement,
                                            substitued);

        if (is_extbody) {
          newextbody = osl_generic_lookup(newstatement->extension,
                           OSL_URI_EXTBODY);
          newbody = newextbody->body;
        }
        else {
          newbody = osl_generic_lookup(newstatement->extension, OSL_URI_BODY);
        }

        free(newbody->expression->string[0]);
        newbody->expression->string[0] = newexpression;

        // synchronize extbody and body (if both are different)
        if (is_extbody) {
          tmpbody = osl_generic_lookup(newstatement->extension, OSL_URI_BODY);
          if (tmpbody) {
            osl_generic_remove(&newstatement->extension, OSL_URI_BODY);
            tmpbody = osl_body_clone(newbody);
            gen = osl_generic_shell(tmpbody, osl_body_interface());
            osl_generic_add(&newstatement->extension, gen);
          }
        }

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
              clay_array_p beta, unsigned int depth, unsigned int depth_outer,
              unsigned int size, int pretty, clay_options_p options) {

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
  
  int ret, i;
  ret = clay_stripmine(scop, beta, depth, size, pretty, options);
  
  // Update beta to reflect the stripmine.
  clay_array_p new_beta = clay_array_malloc();
  for (i = 0; i < depth; i++) {
    clay_array_add(new_beta, beta->data[i]);
  }
  clay_array_add(new_beta, 0);
  for (i = depth; i < beta->size; i++) {
    clay_array_add(new_beta, beta->data[i]);
  }

  if (ret == CLAY_SUCCESS) {
    ret = clay_interchange(scop, new_beta, depth, depth_outer, pretty, options);
  }

  return ret;
}


/**
 * clay_shift function:
 * Shift the iteration domain
 * Warning: in the output part, don't put the alpha columns
 *          example: if the output dims are : 0 i 0 j 0, just do Ni, Nj
 * \param[in,out] scop
 * \param[in] beta          Beta vector (loop or statement)
 * \param[in] depth         >=1
 * \param[in] vector        {(([output, ...],) [param, ...],) [const]}
 * \param[in] options
 * \return                  Status
 */
int clay_shift(osl_scop_p scop, 
               clay_array_p beta, unsigned int depth, clay_list_p vector,
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
  if (vector->size > 3)
    return CLAY_ERROR_VECTOR;
  if (vector->size == 0)
    return CLAY_SUCCESS;
  
  osl_statement_p statement;
  osl_relation_p scattering;
  const int column = depth*2 - 1; // iterator column
  int nb_parameters, nb_output_dims;
  
  statement = clay_beta_find(scop->statement, beta);
  if (!statement)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // decompose the vector
  nb_parameters = scop->context->nb_parameters;
  nb_output_dims = statement->scattering->nb_output_dims;

  if (vector->size == 3) {
    // pad zeros
    clay_util_array_output_dims_pad_zero(vector->data[0]);

    if (vector->data[0]->size > nb_output_dims ||
        vector->data[1]->size > nb_parameters ||
        vector->data[2]->size > 1)
      return CLAY_ERROR_VECTOR;

  } else if (vector->size == 2) {
    if (vector->data[0]->size > nb_parameters ||
        vector->data[1]->size > 1)
      return CLAY_ERROR_VECTOR;

  } else if (vector->size == 1) {
    if (vector->data[0]->size > 1)
      return CLAY_ERROR_VECTOR;
  }

  // add the vector for each statements
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (clay_beta_check_relation(scattering, beta)) {
        CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
        if (scattering->nb_input_dims == 0) {
          return CLAY_ERROR_BETA_NOT_IN_A_LOOP;
        }

        clay_util_relation_set_vector(scattering, vector, column);
      }
      scattering = scattering->next;
    }
    statement = statement->next;
  }
  
  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return CLAY_SUCCESS;
}


/**
 * clay_peel function:
 * Removes the first or last iteration of the loop into separate code.
 * This is equivalent to an iss (without output dims) and a spliting
 * \param[in,out] scop
 * \param[in] beta_loop         Loop beta vector
 * \param[in] peeling list      { ([param, ...],) [const] }
 * \param[in] options
 * \return                      Status
 */
int clay_peel(osl_scop_p scop, 
              clay_array_p beta_loop, clay_list_p peeling,
              clay_options_p options) {

  if (beta_loop->size == 0)
    return CLAY_ERROR_BETA_EMPTY;
  if (peeling->size >= 3)
    return CLAY_ERROR_INEQU;
  if (peeling->size == 0)
    return CLAY_SUCCESS;
  
  clay_array_p arr_dims, beta_max;
  clay_list_p new_peeling;
  int i, ret;
  
  // create output dims
  arr_dims = clay_array_malloc();
  for (i = 0 ; i < beta_loop->size-1 ; i++)
    clay_array_add(arr_dims, 0);
  clay_array_add(arr_dims, 1);

  // create new peeling list
  new_peeling = clay_list_malloc();
  clay_list_add(new_peeling, arr_dims);
  if (peeling->size == 2) {
    clay_list_add(new_peeling, peeling->data[0]);
    clay_list_add(new_peeling, peeling->data[1]);
  } else {
    clay_list_add(new_peeling, clay_array_malloc()); // empty params
    clay_list_add(new_peeling, peeling->data[0]);
  }
  
  // index-set spliting
  ret = clay_iss(scop, beta_loop, new_peeling, &beta_max, options);

  // we don't free ALL with clay_list_free, because there are arrays in
  // common with `peeling'
  clay_array_free(arr_dims);
  if (peeling->size == 1)
    clay_array_free(new_peeling->data[1]);
  free(new_peeling->data);
  free(new_peeling);

  if (ret == CLAY_SUCCESS) {
    // see clay_split
    clay_beta_shift_after(scop->statement, beta_max, 
                          beta_loop->size);
    clay_array_free(beta_max);
  }

  if (options && options->normalize)
    clay_beta_normalize(scop);
  
  return ret;
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
                 &context->m[row][0],
                 vector->data[0]);
  
  j = 1 + context->nb_output_dims + context->nb_input_dims +
      context->nb_local_dims;
  for (i = 1 ; i < vector->size ; i++) {
    osl_int_set_si(precision,
                   &context->m[row][j],
                   vector->data[i]);
    j++;
  }
  
  return CLAY_SUCCESS;
}


static int clay_dimreorder_aux(osl_relation_list_p access,
                               void *args) {
  clay_array_p neworder = args;
  osl_relation_p a = access->elt;

  if (a->nb_output_dims-1 != neworder->size) {
    fprintf(stderr, "[Clay] Warning: can't reorder dims on this statement: ");
    return CLAY_ERROR_REORDER_ARRAY_SIZE;
  }

  osl_relation_p tmp = osl_relation_nclone(a, 1);
  int i, j;

  for (i = 0 ; i < neworder->size ; i++) {
    if (neworder->data[i] < 0 ||
        neworder->data[i] >= a->nb_output_dims-1)
      return CLAY_ERROR_REORDER_OVERFLOW_VALUE;

    if (i+2 != neworder->data[i]+2)
      for (j = 0 ; j < a->nb_rows ; j++)
        osl_int_assign(a->precision, 
                       &tmp->m[j][i+2],
                       a->m[j][neworder->data[i]+2]);
  }

  osl_relation_free(a);
  access->elt = tmp;

  return CLAY_SUCCESS;
}


/**
 * clay_dimreorder function:
 * Reorder the dimensions of access_ident
 * \param[in,out] scop
 * \param[in] beta
 * \param[in] access_ident   ident of the access array
 * \param[in] neworder      reorder the dims 
 * \param[in] options
 * \return                  Status
 */
int clay_dimreorder(osl_scop_p scop,
                    clay_array_p beta,
                    unsigned int access_ident,
                    clay_array_p neworder,
                    clay_options_p options) {

  /* Description
   * Swap the columns in the output dims (access arrays)
   * The first output dim is not used ( => access name) 
   */

  // core of the function : clay_dimreorder_aux

  return clay_util_foreach_access(scop, beta, access_ident, 
                                  clay_dimreorder_aux, neworder, 1);
}


static int clay_dimprivatize_aux(osl_relation_list_p access, void *args) {
  int depth = *((int*) args);
  osl_relation_p a = access->elt;

  // check if the iterator is not used
  int i;
  for (i = 0 ; i < a->nb_rows ; i++) {
    if (!osl_int_zero(a->precision, a->m[i][a->nb_output_dims + depth])) {
      fprintf(stderr, 
          "[Clay] Warning: can't privatize this statement\n"
          "                the dim (depth=%d) seems to be already used\n"
          "                the depth is the depth in the loops\n"
          "                ", depth);
      return CLAY_ERROR_CANT_PRIVATIZE;
    }
  }

  a->nb_output_dims++;

  osl_relation_insert_blank_column(a, a->nb_output_dims);
  osl_relation_insert_blank_row(a, a->nb_rows);

  osl_int_set_si(a->precision, 
                 &a->m[a->nb_rows-1][a->nb_output_dims],
                 -1);

  osl_int_set_si(a->precision,
                 &a->m[a->nb_rows-1][a->nb_output_dims + depth],
                 1);

  return CLAY_SUCCESS;
}


/**
 * clay_dimprivatize function:
 * Privatize an access
 * \param[in,out] scop
 * \param[in] beta
 * \param[in] access_ident   ident of the access array
 * \param[in] depth
 * \param[in] options
 * \return                  Status
 */
int clay_dimprivatize(osl_scop_p scop,
                      clay_array_p beta,
                      unsigned int access_ident,
                      unsigned int depth,
                      clay_options_p options) {

  /* Description
   * Add an output dim on each access which are in the beta vector
   */

  if (depth <= 0) 
    return CLAY_ERROR_DEPTH_OVERFLOW;

  osl_statement_p stmt = clay_beta_find(scop->statement, beta);
  if (!stmt)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // FIXME: asserting that depth holds for any scattering
  osl_relation_p scattering = stmt->scattering;
  for ( ; scattering != NULL; scattering = scattering->next) {
    CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
  }

  // core of the function : clay_dimprivatize_aux

  return clay_util_foreach_access(scop, beta, access_ident,
                                  clay_dimprivatize_aux, &depth, 1);
}


static int clay_dimcontract_aux(osl_relation_list_p access, void *args) {
  int depth = *((int*) args);
  osl_relation_p a = access->elt;

  int row = clay_util_relation_get_line(a, depth);
  if (row != -1) {
    osl_relation_remove_row(a, row);
    osl_relation_remove_column(a, depth+1); // remove output dim
    a->nb_output_dims--;
  }

  return CLAY_SUCCESS;
}


/**
 * clay_dimcontract function:
 * Contract an access (remove a dimension)
 * \param[in,out] scop
 * \param[in] beta
 * \param[in] access_ident   ident of the access array
 * \param[in] depth
 * \param[in] options
 * \return                  Status
 */
int clay_dimcontract(osl_scop_p scop,
                     clay_array_p beta,
                     unsigned int access_ident,
                     unsigned int depth,
                     clay_options_p options) {
  /* Description
   * Remove the line/column at the depth level
   */

  if (depth <= 0) 
    return CLAY_ERROR_DEPTH_OVERFLOW;

  osl_statement_p stmt = clay_beta_find(scop->statement, beta);
  if (!stmt)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // FIXME: asserting that depth holds for any scattering
  osl_relation_p scattering = stmt->scattering;
  for ( ; scattering != NULL; scattering = scattering->next) {
    CLAY_BETA_CHECK_DEPTH(beta, depth, scattering);
  }

  // core of the function : clay_dimcontract_aux

  return clay_util_foreach_access(scop, beta, access_ident, 
                                  clay_dimcontract_aux, &depth, 1);
}


/**
 * clay_add_array function:
 * Add a new array in the arrays extensions.
 * \param[in,out] scop
 * \param[in] name          string name
 * \param[in] result        return the new id
 * \param[in] options
 * \return                  Status
 */
int clay_add_array(osl_scop_p scop,
                  char *name,
                  int *result,
                  clay_options_p options) {

  osl_arrays_p arrays = osl_generic_lookup(scop->extension, OSL_URI_ARRAYS);
  if (!arrays)
    return CLAY_ERROR_ARRAYS_EXT_EMPTY;

  int i;
  int sz = arrays->nb_names;

  if (sz == 0)
    return CLAY_SUCCESS;

  for (i = 0 ; i < sz ; i++)
    if (strcmp(arrays->names[i], name) == 0)
      return CLAY_ERROR_ID_EXISTS;

  int id = arrays->id[0];

  for (i = 1 ; i < sz ; i++)
    if (arrays->id[i] > id)
      id = arrays->id[i];

  arrays->nb_names++;

  // I don't know why, there is a valgrind warning when I use CLAY_realloc
  arrays->id = realloc(arrays->id, sizeof(int) * (sz+1));
  arrays->names = realloc(arrays->names, sizeof(char*) * (sz+1));

  id++;
  
  arrays->id[sz] = id;
  arrays->names[sz] = strdup(name);

  *result = id;

  return CLAY_SUCCESS;
}

/**
 * clay_get_array_id function:
 * Search the array name in the arrays extension
 * \param[in,out] scop
 * \param[in] name          string name
 * \param[in] result        return the id
 * \param[in] options
 * \return                  Status
 */
int clay_get_array_id(osl_scop_p scop,
                      char *name,
                      int *result,
                      clay_options_p options) {

  osl_arrays_p arrays = osl_generic_lookup(scop->extension, OSL_URI_ARRAYS);
  if (!arrays)
    return CLAY_ERROR_ARRAYS_EXT_EMPTY;

  int i;
  int sz = arrays->nb_names;
  int id = -1;

  for (i = 0 ; i < sz ; i++)
    if (strcmp(arrays->names[i], name) == 0) {
      id = arrays->id[i];
      break;
    }

  if (id == -1)
    return CLAY_ERROR_ARRAY_NOT_FOUND;

  *result = id;

  return CLAY_SUCCESS;
}


static int clay_replace_array_aux(osl_relation_list_p access, void *args) {
  int new_id = *((int*) args);
  osl_relation_p a = access->elt;

  // here row is != -1, because the function aux shouldn't be called
  int row = clay_util_relation_get_line(a, 0); 
  osl_int_set_si(a->precision, &a->m[row][a->nb_columns-1], new_id);

  return CLAY_SUCCESS;
}

/**
 * clay_replace_array function:
 * Replace an ident array by another in each access
 * \param[in,out] scop
 * \param[in] last_id
 * \param[in] new_id
 * \param[in] options
 * \return    Status
 */
int clay_replace_array(osl_scop_p scop,
                       unsigned int last_id,
                       unsigned int new_id,
                       clay_options_p options) {

  // core of the function : clay_replace_array_aux

  clay_array_p beta = clay_array_malloc();
  int ret = clay_util_foreach_access(scop, beta, last_id,
                                     clay_replace_array_aux, &new_id, 1);
  clay_array_free(beta);

  return ret;
}


/**
 * clay_datacopy function:
 * This function will generate a loop to copy all data from the array
 * `array_id_original' to a new array `array_id_copy'. Use the function
 * add_array to insert a new array in the scop. A domain and a scattering
 * is needed to generate the loop to copy the data. They is a copy
 * from the domain/scattering of the first statement which corresponds
 * to the `beta_get_domain'. There is just a modification on the domain to
 * remove unused loops (iter >= 1 && iter <= -1 is set). The orignal id or
 * the copy id must be in this beta (in the list of access relation). Genarally
 * you will use replace_array before calling datacopy, that's why the
 * array_id_copy can be in the scop.  The first access relation which is found
 * will be used to generate an access for the original id and the copy id.
 * \param[in,out] scop
 * \param[in] array_id_copy     new variable
 * \param[in] array_id_original
 * \param[in] beta             the loop is insert after the beta
 * \param[in] beta_get_domain  domain/scattering are copied from this beta
 *                             the original or copy id must be in the list of 
 *                             access of this beta
 * \param[in] options
 * \return                     Status
 */
int clay_datacopy(osl_scop_p scop,
                  unsigned int array_id_copy,
                  unsigned int array_id_original,
                  clay_array_p beta_insert,
                  int insert_before,
                  clay_array_p beta_get_domain,
                  clay_options_p options) {

  /* Description
   * - search the statement S beta_get_domain
   * - search the array array_id_original or array_id_copy in the list of
   *   access in the given statement
   * - clone the found access array
   * - reclone this access and put the other array id (it depends which
   *   id is found in the second step)
   * - search the beta_insert
   * - shift all the beta after or before beta_insert to let the place of the
   *   new statement
   * - copy domain + scattering of S in a new statement (and add this one in
   *   the scop)
   * - for each unused iterators we put iter >= 1 && iter <= -1 in the domain
   *   to remove the loop at the generation of code
   * - put the 2 access arrays in this statement
   * - generate body
   */

  osl_relation_p scattering, ptr = NULL;
  int row, i, j;

  // TODO : global vars ??
  osl_arrays_p arrays;
  osl_scatnames_p scatnames;
  osl_strings_p params;
  osl_body_p body = NULL;
  osl_extbody_p extbody = NULL;
  int is_extbody = 0;
  osl_generic_p gen = NULL;
  arrays = osl_generic_lookup(scop->extension, OSL_URI_ARRAYS);
  scatnames = osl_generic_lookup(scop->extension, OSL_URI_SCATNAMES);
  params = osl_generic_lookup(scop->parameters, OSL_URI_STRINGS);

  if (beta_insert->size == 0)
    return CLAY_ERROR_BETA_EMPTY;

  // search the beta where we have to insert the new loop
  osl_statement_p stmt_1 = clay_beta_find(scop->statement, beta_insert);
  if (!stmt_1)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // search the beta which is need to copy the domain and scattering
  osl_statement_p stmt_2 = clay_beta_find(scop->statement, beta_get_domain);
  if (!stmt_2)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // copy the domain/scattering
  osl_statement_p copy = osl_statement_malloc();
  copy->domain = osl_relation_clone(stmt_2->domain);
  //copy->scattering = osl_relation_clone(stmt_2->scattering);
  copy->scattering = NULL;
  scattering = stmt_2->scattering;
  while (scattering != NULL) {
    if (clay_beta_check_relation(scattering, beta_get_domain)) {
      if (ptr == NULL) {
        copy->scattering = osl_relation_nclone(scattering, 1);
        ptr = copy->scattering;
      } else {
        ptr->next = osl_relation_nclone(scattering, 1);
        ptr = ptr->next;
      }
    }
    scattering = scattering->next;
  }

  // create an extbody and copy the original iterators
  osl_extbody_p ebody = osl_extbody_malloc();
  ebody->body = osl_body_malloc();

  // body string (it will be regenerated)
  ebody->body->expression = osl_strings_encapsulate(strdup("@ = @;"));

  // copy iterators
  extbody = osl_generic_lookup(stmt_2->extension, OSL_URI_EXTBODY);
  if (extbody) {
    ebody->body->iterators = osl_strings_clone(extbody->body->iterators);
    is_extbody = 1;
  }
  else {
    body = osl_generic_lookup(stmt_2->extension, OSL_URI_BODY);
    ebody->body->iterators = osl_strings_clone(body->iterators);
  }

  // 2 access coordinates
  osl_extbody_add(ebody, 0, 1);
  osl_extbody_add(ebody, 4, 1);

  gen = osl_generic_shell(ebody, osl_extbody_interface());
  osl_generic_add(&copy->extension, gen);
  // not creating a ebody's corresponding "body" in the new copy statement


  // search the array_id in the beta_get_domain
  
  osl_relation_list_p access = stmt_2->access;
  osl_relation_p a;
  int id;

  while (access) {
    a = access->elt;
    id = osl_relation_get_array_id(a);
    if (id == array_id_original || id == array_id_copy)
      break;
    access = access->next;
  }

  if (!access)
    return CLAY_ERROR_ARRAY_NOT_FOUND_IN_THE_BETA;

  // matrix of the copied array
  a = osl_relation_nclone(a, 1);
  a->type = OSL_TYPE_WRITE;
  osl_int_set_si(a->precision, &a->m[0][a->nb_columns-1], array_id_copy);
  copy->access = osl_relation_list_malloc();
  copy->access->elt = a;

  clay_util_body_regenerate_access(ebody, a, 0, arrays, scatnames, params);

  // matrix of the original array
  a = osl_relation_nclone(a, 1);
  a->type = OSL_TYPE_READ;
  copy->access->next = osl_relation_list_malloc();
  copy->access->next->elt = a;
  copy->access->next->next = NULL;
  osl_int_set_si(a->precision, &a->m[0][a->nb_columns-1], array_id_original);

  clay_util_body_regenerate_access(ebody, a, 1, arrays, scatnames, params);


  // remove the unused dim in the scatterin (modify the domain of the loop)
  for (j = 0 ; j < a->nb_input_dims ; j++) {
    int found = 0;

    // search an unused input dims (if there is only 0 on the row)
    for (i = 0 ; i < a->nb_rows ; i++) {
      if (!osl_int_zero(a->precision, a->m[i][1 + a->nb_output_dims + j])) {
        found = 1;
        break;
      }
    }

    // unused
    if (!found) {
      int k, t;
      osl_relation_p domain = copy->domain;
      for (i = 0 ; i < domain->nb_rows ; i++) {
        t = osl_int_get_si(domain->precision, domain->m[i][1 + j]);

        if (t != 0) {
          for (k = 1 ; k < domain->nb_columns-1 ; k++)
            if (k != 1+j)
              osl_int_set_si(domain->precision, &domain->m[i][k], 0);

          // set iter <= -1 && iter >= 1  ==> no solutions, so no loop !
          if (t > 0)
            osl_int_set_si(domain->precision, &domain->m[i][k], -1);
          else
            osl_int_set_si(domain->precision, &domain->m[i][k], 1);
        }
      }
    }
  }


  // let the place to the new loop

  scattering = copy->scattering;
  if (insert_before) {
    clay_beta_shift_before(scop->statement, beta_insert, 1);

    // set the beta : it's the beta[0]
    while (scattering != NULL) {
      // we know that copy has only scattering union parts that match the
      // requested beta.
      row = clay_util_relation_get_line(scattering, 0);
      osl_int_set_si(scattering->precision,
                     &scattering->m[row][scattering->nb_columns-1],
                     beta_insert->data[0]);
      scattering = scattering->next;
    }
  } else {
    clay_beta_shift_after(scop->statement, beta_insert, 1);

    // set the beta : it's the beta[0] + 1
    while (scattering != NULL) {
      row = clay_util_relation_get_line(scattering, 0);
      osl_int_set_si(scattering->precision, 
                     &scattering->m[row][scattering->nb_columns-1],
                     beta_insert->data[0]+1);
      scattering = scattering->next;
    }
  }

  copy->next = scop->statement;
  scop->statement = copy;

  if (options && options->normalize)
    clay_beta_normalize(scop); 

  return CLAY_SUCCESS;
}


/**
 * clay_block function:
 * \param[in,out] scop
 * \param[in] beta_stmt1
 * \param[in] beta_stmt2
 * \param[in] options
 * \return                     Status
 */
int clay_block(osl_scop_p scop,
               clay_array_p beta_stmt1,
               clay_array_p beta_stmt2,
               clay_options_p options) {

  /* Description
   * concat '{' + body(stmt_1) + body(stmt_2) + '}'
   * update extbody if needed
   * concat access(stmt_1) + access(stmt_2)
   * remove stmt_2
   */
  if (beta_stmt1->size != beta_stmt2->size)
    return CLAY_ERROR_BETAS_NOT_SAME_DIMS;

  // search statements and check betas
  osl_statement_p stmt_1 = clay_beta_find(scop->statement, beta_stmt1);
  if (!stmt_1)
    return CLAY_ERROR_BETA_NOT_FOUND;

  osl_statement_p stmt_2 = clay_beta_find(scop->statement, beta_stmt2);
  if (!stmt_2)
    return CLAY_ERROR_BETA_NOT_FOUND;

  // We can only merge full statements, because we change the body so it would
  // affect other parts with different scatterings.
  osl_relation_p scattering = stmt_1->scattering;
  while (scattering != NULL) {
    CLAY_BETA_IS_STMT(beta_stmt1, scattering);
    scattering = scattering->next;
  }
  scattering = stmt_2->scattering;
  while (scattering != NULL) {
    CLAY_BETA_IS_STMT(beta_stmt2, scattering);
    scattering = scattering->next;
  }

  if (!osl_relation_equal(stmt_1->domain, stmt_2->domain))
    return CLAY_ERROR_BETAS_NOT_SAME_DOMAIN;

  int i;
  int is_extbody_1 = 0;
  int is_extbody_2 = 0;

  char **expr_left;
  char **expr_right;
  char *new_expr;

  osl_extbody_p ebody_1, ebody_2;
  osl_body_p body_1, body_2;
  osl_generic_p gen = NULL;

  // get the body string

  ebody_1 = osl_generic_lookup(stmt_1->extension, OSL_URI_EXTBODY);
  if (ebody_1) {
    is_extbody_1 = 1;
  }
  else {
    body_1 = osl_generic_lookup(stmt_1->extension, OSL_URI_BODY);
  }

  ebody_2 = osl_generic_lookup(stmt_2->extension, OSL_URI_EXTBODY);
  if (ebody_2) {
    is_extbody_2 = 1;
  }
  else {
    body_2 = osl_generic_lookup(stmt_2->extension, OSL_URI_BODY);
  }

  if (is_extbody_1 != is_extbody_2)
    return CLAY_ERROR_ONE_HAS_EXTBODY;


  expr_left = is_extbody_1
    ? ebody_1->body->expression->string
    : body_1->expression->string;

  expr_right = is_extbody_2
    ? ebody_2->body->expression->string
    : body_2->expression->string;

  // update extbody
  if (is_extbody_1) {

    // shift for the '{'
    for (i = 0 ; i < ebody_1->nb_access ; i++) {
      if (ebody_1->start[i] != -1)
        ebody_1->start[i]++;
    }

    int offset = 1 + strlen(expr_left[0]);

    // shift the right part
    for (i = 0 ; i < ebody_2->nb_access ; i++) {
      if (ebody_2->start[i] != -1)
        ebody_2->start[i] += offset;

      // concat with the extbody 2
      osl_extbody_add(ebody_1, ebody_2->start[i], ebody_2->length[i]);
    }

  }

  // generate the new body string
  new_expr = (char*) malloc(strlen(expr_left[0]) + strlen(expr_right[0]) + 3);
  strcpy(new_expr, "{");
  strcat(new_expr, expr_left[0]);
  strcat(new_expr, expr_right[0]);
  strcat(new_expr, "}");

  free(expr_left[0]);
  expr_left[0] = new_expr;

  // concat all the access array
  osl_relation_list_p access = stmt_1->access;
  if (access) {
    while (access->next)
      access = access->next;
  }
  access->next = stmt_2->access;
  stmt_2->access = NULL;

  // synchronize extbody with body for stmt_1
  if (is_extbody_1) {
    body_1 = osl_generic_lookup(stmt_1->extension, OSL_URI_BODY);
    if (body_1) {
      osl_generic_remove(&stmt_1->extension, OSL_URI_BODY);
      body_1 = osl_body_clone(ebody_1->body);
      gen = osl_generic_shell(body_1, osl_body_interface());
      osl_generic_add(&stmt_1->extension, gen);
    }
  }

  // Remove the scattering parts of stmt_2 that match the beta2.
  scattering = stmt_2->scattering;
  if (scattering) {
    if (clay_beta_check_relation(scattering, beta_stmt2)) {
      osl_relation_p to_delete = scattering;
      to_delete->next = NULL;
      stmt_2->scattering = scattering->next;
      scattering = scattering->next;
      osl_relation_free(to_delete);
    }
  }
  // These branches must not be merged, scattering is overwritten inside!
  if (scattering) {
    while (scattering->next != NULL) {
      osl_relation_p to_delete = NULL;
      if (clay_beta_check_relation(scattering->next, beta_stmt2)) {
        to_delete = scattering->next;
        to_delete->next = NULL;
        scattering->next = scattering->next->next;
      }
      scattering = scattering->next;
      osl_relation_free(to_delete);
    }
  }
  // Remove stmt_2 completely if it has no more scatterings (i.e. was only a statement).
  if (stmt_2->scattering == NULL) {
    osl_statement_p s = scop->statement;
    if (s != NULL) {
      if (s == stmt_2) {
        scop->statement = s->next;
        stmt_2->next = NULL;
        osl_statement_free(stmt_2);
        stmt_2 = NULL;
        s = s->next;
      }
    }
    if (s != NULL && stmt_2 != NULL) {
      while (s->next != NULL) {
        if (s->next == stmt_2) {
          s->next = s->next->next;
          stmt_2->next = NULL; // prevent free from removing chained statements
          osl_statement_free(stmt_2);
          break;
        }
        s = s->next;
      }
    }
  }

  if (options && options->normalize)
    clay_beta_normalize(scop); 

  return CLAY_SUCCESS;
}
