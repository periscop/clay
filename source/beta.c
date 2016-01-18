
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             beta.c                                 |
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
#include <limits.h>

#include <osl/scop.h>
#include <osl/statement.h>
#include <osl/relation.h>

#include <clay/array.h>
#include <clay/list.h>
#include <clay/beta.h>
#include <clay/macros.h>
#include <clay/util.h>

/**
 * \page betaDescription Beta-vectors and beta-prefixes
 * Beta-vector identifies a statement in the *source code* that corresponds to
 * the SCoP.  After a transformation was applied, one statement may have
 * multiple beta-vectors in the internal representation, i.e. each part of the
 * scattering relation may potentially encode a different beta-vector.  Please
 * note that if the code was regenerated, this information is not preserved.
 * On the other hand, a beta-vector is guaranteed to identify a unique
 * statement.  A beta-prefix is a beta-vector with at least one last element
 * removed.  It is used as a wildcard to identify multiple statements.
 *
 * A \b beta-vector identifies a statement in the code with all depth levels.
 *
 * A \b beta-prefix identifies a loop in the code, possibly containing or nested
 * within another loop; it may be used to address all the statements and loops
 * nested inside the loop identified by it.
 *
 * A \b beta is used to reference either beta-vector or beta-prefix in contexts
 * where they can be used interchangeably.
 *
 * A beta matches another beta, if their common elements are equal.
 */

/**
 * \brief Reassings each statement (scattering relation union part) with a
 * beta-vector such that its values are directly following the previous
 * beta-vector with respect to the statement depth.
 * For example, [0], [1,0], [1,1,0], [1,2] are normalized beta-vectors for the code <pre>
 * S0; //[0]
 * for {
 *   S1; //[1,0]
 *   for {
 *     S2; //[1,1,0]
 *   }
 *   S3; //[1,2]
 * } </pre>
 */
void clay_beta_normalize(osl_scop_p scop) {
  if (!scop || !scop->statement)
    return;

  osl_statement_p next_statement, stmt;
  clay_array_p beta, beta_next, new_beta;
  int difference, depth;
  int i;
  int counter_loops[CLAY_MAX_BETA_SIZE];
  for (i = 0; i < CLAY_MAX_BETA_SIZE; i++) {
    counter_loops[i] = 0;
  }

  // Take the first possible beta.
  beta = clay_array_malloc();
  beta_next = clay_beta_next(scop->statement, beta, &next_statement);

  while (beta_next != NULL) {
    clay_array_free(beta);
    beta = beta_next;
    new_beta = clay_array_malloc();
    for (i = 0; i < beta->size; i++) {
      clay_array_add(new_beta, counter_loops[i]);
    }
    stmt = next_statement;

    beta_next = clay_beta_next(scop->statement, beta, &next_statement);
    clay_util_statement_replace_beta(stmt, beta, new_beta);
    clay_array_free(new_beta);
    if (beta_next == NULL)
      break;

    difference = clay_beta_compare(beta_next, beta, &depth);
    if (depth != beta_next->size && depth >= 0) {
      counter_loops[depth]++;
    }

    for (i = depth + 1; i < beta->size; i++) {
      counter_loops[i] = 0;
    }
  }
  clay_array_free(beta);
}

/**
 * Internal function to compute either minimum or maximum beta-vector matching
 * the given beta-prefix.
 * \param [in] statement Pointer to the first statement in the linked list that
 * matches the beta-prefix #beta
 * \param [in] beta The beta-prefix
 * \param [in] sign \c -1 to compute the minimum, \c +1 to compute the maximum
 * \returns A minimum or maximum beta-prefix.
 */
static clay_array_p clay_beta_minmax(osl_statement_p statement,
                                     clay_array_p beta, int sign) {
  statement = clay_beta_find(statement, beta);
  if (!statement || !statement->scattering)
    return NULL;

  osl_relation_p scattering;
  clay_array_p found_beta;
  clay_array_p beta_min = clay_beta_extract(statement->scattering);

  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (clay_beta_check_relation(scattering, beta)) {
        found_beta = clay_beta_extract(scattering);
        if (clay_beta_compare(beta_min, found_beta, NULL) * sign > 0) {
          clay_array_free(beta_min);
          beta_min = clay_array_clone(found_beta);
        }
        clay_array_free(found_beta);
      }
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  return beta_min;
}

/**
 * \brief Find the minimum beta-vector that matches the given beta-prefix.
 * \param[in] statement     List of statements
 * \param[in] beta          Beta-prefix
 * \return                  The minimum beta-prefix.
 */
clay_array_p clay_beta_min(osl_statement_p statement, clay_array_p beta) {
  return clay_beta_minmax(statement, beta, -1);
}

/**
 * \brief Find the maximum beta-vector that matches the given beta-prefix.
 * \param[in] statement     List of statements
 * \param[in] beta          Beta-prefix
 * \return                  The maximum beta-prefix.
 */
clay_array_p clay_beta_max(osl_statement_p statement, clay_array_p beta) {
  return clay_beta_minmax(statement, beta, 1);
}

/**
 * \brief Find the beta-vector of the statement that follows the given
 * beta-vector or beta-prefix.  Equivalent to clay_beta_min(osl_statement_p,
 * clay_array_p) in case of a beta-prefix.
 * \param[in] statement     List of statements
 * \param[in] beta          Beta-vector
 * \param[out] sout         Pointer to the statement matching the returned
 *                          beta-vector; please note that the scattering 
 *                          relation union part that matches the beta-vector is
 *                          not necesserily the first in the list.
 * \return                  The next beta-vector.
 */
clay_array_p clay_beta_next(osl_statement_p statement, clay_array_p beta,
                            osl_statement_p *sout) {
  //XXX: this function is an inefficient brute force search; I don't see any other option
  if(!statement || !beta)
    return NULL;
  clay_array_p beta_next = NULL;

  int difference, depth;
  osl_relation_p scattering;
  clay_array_p found_beta;
  while (statement != NULL) {
    scattering = statement->scattering;
    // There is no sense in checking "strictly before" with current
    // implementation since it will go inside and check all scattering union
    // parts, doing the same here.
    while (scattering != NULL) {
      found_beta = clay_beta_extract(scattering);
      difference = clay_beta_compare(beta, found_beta, &depth);
      if ((difference == 0 && found_beta->size > beta->size) ||
          difference > 0) {
        // Found a beta-vector matching or following the given beta-prefix.
        // Check if it is less then the current found, they must be different
        // since neither found nor current next are not beta-prefixes.
        difference = clay_beta_compare(found_beta, beta_next, &depth);
        if (difference > 0) {
          if (beta_next != NULL)
            clay_array_free(beta_next);
          beta_next = clay_array_clone(found_beta);
          if (sout) *sout = statement;
        }
      }
      clay_array_free(found_beta);
      scattering = scattering->next;
    }
    statement = statement->next;
  }
  
  if (beta_next == NULL) {
    if (sout) *sout = NULL;
    return NULL;
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
  if (!statement)
    return NULL;

  osl_relation_p scattering;
  clay_array_p beta_next = NULL, scattering_beta;
  int min_difference = INT_MAX, difference, depth;

  // Search for the following beta-prefix or beta-vector
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      scattering_beta = clay_beta_extract(scattering);
      difference = clay_beta_compare(beta, scattering_beta, &depth);
      if (depth == beta->size - 1 && difference > 0 && difference < min_difference) {
        min_difference = difference;
        clay_array_free(beta_next);
        beta_next = clay_array_clone(scattering_beta);
      }
      clay_array_free(scattering_beta);
      scattering = scattering->next;
    }
    statement = statement->next;
  }
  
  return beta_next;
}


// XXX: this function had an unclear comment about "first checked and not the
// first in the list", while it exactly was doing linear list scan and returned
// the first matching statement.
/**
 * \brief Find the first statement in the list that matches the given
 * beta-vector or beta-prefix.  Please note that the fact that the statement
 * matches the beta-prefix does not necesserily mean that all the scattering
 * relation union parts match the given beta-prefix.
 * \param[in] statement     A linked list of statements.
 * \param[in] beta          A beta-prefix or beta-vector.
 * \return                  The first statement in the list matching the given
 *                          beta-vector or beta-prefix.
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
 * Get the number of parts (either statement or loops) matching the
 * given beta in the next depth level.
 * It doesn't count statements inside sub loops
 * Example: <pre>
 * for(i) {
 *   S1(i)
 *   for(j) {
 *     S2(i,j)
 *     S3(i,j)
 *   }
 * } </pre>
 * nb_parts in <tt>for(i) = 2 (S1 and for(j))</tt>
 * Put differently, it returns the number of distinct beta-prefixes or
 * beta-vectors that have one supplementary element comparing to the given
 * beta.
 * \param[in] statement     Statements list
 * \param[in] beta          Vector to search
 * \return
 */
int clay_beta_nb_parts(osl_statement_p statement, clay_array_p beta) {
  int nb_parts;
  osl_relation_p scattering;
  clay_array_p scattering_beta;
  clay_list_p part_betas = clay_list_malloc();
  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      if (clay_beta_check_relation(scattering, beta)) {
        scattering_beta = clay_beta_extract(scattering);
        // Check if the beta is a statement.
        if (scattering_beta->size == beta->size) {
          clay_array_free(scattering_beta);
          return 1;
        }
        if (scattering_beta->size < beta->size) {
          CLAY_error("beta_check_relation works incorrectly");
        }
        scattering_beta->size = beta->size + 1;
        if (!clay_list_contains(part_betas, scattering_beta)) {
          clay_list_add(part_betas, clay_array_clone(scattering_beta));
        }
        clay_array_free(scattering_beta);
      }
      scattering = scattering->next;
    }
    statement = statement->next;
  }
  nb_parts = part_betas->size;
  return nb_parts;
}


// Might seem reasonable to check if the next-after-given beta exists before
// moving it, but it would require one more full search; function twice
// expensive if it indeed exists...
/**
 * Replace all beta-vectors after the given with their respective next
 * beta-vectors at a given depth, i.e. increment the value at the given depth
 * of the beta-vector.  The modification may include or exclude the given beta
 * itself depending on the #inclusive parameter.  If a beta is a prefix, all
 * the beta-vectors that match it will be affected.
 * \param [in]  statement  A linked list of statements.
 * \param [in]  beta       A reference beta-vector or beta-prefix
 */
static void clay_beta_shift(osl_statement_p statement, clay_array_p beta,
                     int depth, int inclusive) {
  if (beta->size == 0)
    return;
  osl_relation_p scattering;
  const int column = (depth-1)*2;  // alpha column
  int row;
  int precision = statement->scattering->precision;
  int difference, difference_depth;

  clay_array_p beta_parent = clay_array_clone(beta),
               scattering_beta;
  beta_parent->size = depth-1;

  while (statement != NULL) {
    scattering = statement->scattering;
    while (scattering != NULL) {
      scattering_beta = clay_beta_extract(scattering);
      difference = clay_beta_compare(beta, scattering_beta, &difference_depth);
      // Shift only those betas, that have common beta-prefix and differ in depth's
      // dimension.
      if ((difference > 0 || (difference == 0 && inclusive))&&
          column < scattering->nb_output_dims) {
        row = clay_util_relation_get_line(scattering, column);
        osl_int_increment(precision, 
                          &scattering->m[row][scattering->nb_columns-1],
                          scattering->m[row][scattering->nb_columns-1]);
      }
      clay_array_free(scattering_beta);
      scattering = scattering->next;
    }
    statement = statement->next;
  }

  clay_array_free(beta_parent);
}

/**
 * clay_beta_shift_before function:
 * Replace all the beta-vectors greater or equal to the given with their direct
 * successors at the given depth.  Increments the depth-s element of each
 * beta-vector that is equal to or greater than the given beta.  Shift betas so
 * that there is a place to insert new statement having exactly the given beta.
 * If the beta is empty, the function does nothing.
 * \param[in,out] statement Statements list
 * \param[in] beta          Beta vector 
 * \param[in] depth         Depth level to shift, >= 1
 * \see clay_beta_shift_after
 */
void clay_beta_shift_before(osl_statement_p statement, clay_array_p beta,
                           int depth) {
  clay_beta_shift(statement, beta, depth, 1);
}

/**
 * clay_beta_shift_after function:
 * Replace all the beta-vectors greater or equal to the given with their direct
 * successors at the given depth.  Increments the depth-s element of each
 * beta-vector that is greater than the given beta.  Shift betas so that there
 * is a place to insert new statement with the beta following the given beta at
 * the given depth.  If the beta is empty, the function does nothing.
 * \param[in,out] statement Statements list
 * \param[in] beta          Beta vector 
 * \param[in] depth         Depth level to shift, >= 1
 */
void clay_beta_shift_after(osl_statement_p statement, clay_array_p beta,
                           int depth) {
  clay_beta_shift(statement, beta, depth, 0);
}

// interleaves != contains
/**
 * Checks if a statement interleaves with the given beta.  The statement
 * interleaves with a beta if the statement has at least one beta-vector that
 * precedes the given beta-vector and at least one beta that succeeds it.
 * \param[in]  statement   Statement to check (all the next statements are ignored).
 * \param[in]  beta        Beta-vector to check against
 * \returns \c 1 if the given
 */
int clay_statement_interleaves(osl_statement_p statement, clay_array_p beta) {
  // XXX: inefficient: two calls to clay_statement_strict_order inside
  return !clay_beta_check(statement, beta) &&
         !clay_statement_is_before(statement, beta) &&
         !clay_statement_is_after(statement, beta);
}

// order = -1: strictly before
// order =  1: strictly after
static int clay_statement_strict_order(osl_statement_p statement,
                                              clay_array_p beta,
                                              int order) {
  if (clay_beta_check(statement, beta))
    return 0;

  int i, difference;
  clay_list_p statement_betas = clay_statement_betas(statement);
  for (i = 0; i < statement_betas->size; i++) {
    difference = clay_beta_compare(statement_betas->data[i], beta, NULL);
    if (difference * order > 0) {
      return 0;
    }
  }
  clay_list_free(statement_betas);
  return 1;
}

// strictly before means that any beta of this statement is strictly before the given beta
/**
 * clay_statement_is_before function:
 * Return true if the statement is before (strictly) the beta
 * \param[in] statement
 * \param[in] beta
 * \return
 */
int clay_statement_is_before(osl_statement_p statement, clay_array_p beta) {
  return clay_statement_strict_order(statement, beta, -1);
}


/**
 * clay_statement_is_after function:
 * Return true if statement is after (strictly) the beta
 * \param[in] statement
 * \param[in] beta
 * \return
 */
int clay_statement_is_after(osl_statement_p statement, clay_array_p beta) {
  return clay_statement_strict_order(statement, beta, 1);
}

/**
 * Check if a beta-vector encoded in the given relation matches the given
 * beta-vector.
 * \param[in]  relation   Relation encoding a beta-vector
 * \param[in]  beta       A beta-vector or beta-prefix to match against
 */
int clay_beta_check_relation(osl_relation_p relation,
                             clay_array_p beta) {
  clay_array_p extracted_beta = clay_beta_extract(relation);
  int difference = clay_beta_compare(beta, extracted_beta, NULL);
  clay_array_free(extracted_beta);
  return !difference;
}

/**
 * Check if any of the betas of the given statement match the given beta-vector
 * or beta-prefix.  \c true if at least one part of the scattering relation
 * union part matches the given beta.
 * \param[in] statement     Statement to test
 * \param[in] beta          Vector to search
 * \return                  true if correct
 */
int clay_beta_check(osl_statement_p statement, clay_array_p beta) {
  osl_relation_p scattering = statement->scattering;
  int checks = 0;
  while (scattering != NULL) {
    checks = checks || clay_beta_check_relation(scattering, beta);
    if (checks)
      return 1;
    scattering = scattering->next;
  }
  return checks;
}

int (*clay_beta_check_any)(osl_statement_p, clay_array_p) = clay_beta_check;

/**
 * Check if all the betas of the given statement match the given beta.
 * \c true if all scattering relation union parts match the given beta.
 */
int clay_beta_check_all(osl_statement_p statement, clay_array_p beta) {
  osl_relation_p scattering = statement->scattering;
  int checks = 1;
  while (scattering != NULL) {
    checks = checks && clay_beta_check_relation(scattering, beta);
    if (!checks)
      return 0;
    scattering = scattering->next;
  }
  return checks;
}

/**
 * clay_scattering_check_zeros function:
 * check if the scattering on the first statement contains only zero at line i 
 * and column j, but not on (i,j), on the first column (equality column), and on
 * the last column.
 * Used to check if the line is a line of a beta vector
 * \param[in] relation
 * \param[in] i
 * \param[in] j
 * \return                  true if correct
 */
int clay_scattering_check_zeros(osl_relation_p scattering, int i, int j) {
  if (scattering == NULL)
    return 1;

  int precision = scattering->precision;
  int t;
  osl_relation_p scattering_next;

  for (t = i+1 ; t < scattering->nb_rows ; t++) {
    if (!osl_int_zero(precision, scattering->m[t][j])) {
      fprintf(stderr, "[Clay] Warning: the scattering is incorrect (column %d)\n",
              j);
      fprintf(stderr, "[Clay] a non-zero value appear\n");
      scattering_next = scattering->next;
      scattering->next = NULL;
      osl_relation_dump(stderr, scattering);
      scattering->next = scattering->next;
      return 0;
    }
  }
  for (t = j+1 ; t < scattering->nb_columns-1 ; t++) {
    if (!osl_int_zero(precision, scattering->m[i][t])) {
      fprintf(stderr, "[Clay] Warning: the scattering is incorrect (line %d)\n",
              i+1);
      fprintf(stderr, "[Clay] a non-zero value appear\n");
      scattering_next = scattering->next;
      scattering->next = NULL;
      osl_relation_dump(stderr, scattering);
      scattering->next = scattering->next;
      return 0;
    }
  }

  return 1;
}

/**
 * Compare two beta-vectors or beta-prefixes.  Performs lexicographic
 * comparison of two beta-vectors using the number of \b common dimensions.
 * Returns a non-zero value in case they differ and an index at which they
 * differ.  Empty beta-vector compares to everything, \p nested is then equal
 * to \c -1.  \c NULL doesn't compare to everything and results in \c INT_MAX
 * difference and \p nested assigned \c -2.
 * \param[in]  beta1   First beta-vector.
 * \param[in]  beta2   Second beta-vector.
 * \param[out] nested  If different, first differing dimension; minimum size - 1 otherwise.
 * \returns  Difference between values of the first differing values in the
 * beta-vector.
 */
int clay_beta_compare(clay_array_p beta1, clay_array_p beta2, int *nested) {
  if (beta1 == NULL || beta2 == NULL) {
    if (nested != NULL)
      *nested = -2;
    return INT_MAX;
  }
  int size = beta1->size < beta2->size ? beta1->size : beta2->size;
  int i, difference;
  if (nested != NULL)
    *nested = -1;
  for (i = 0; i < size; i++) {
    difference = beta2->data[i] - beta1->data[i];
    if (nested != NULL) {
      *nested = i;
    }
    if (difference != 0) {
      return difference;
    }
  }
  return 0;
}

/**
 * Compare two beta-vectors for exact equality.  Beta-vectors are considered
 * equal if their sizes are equals and elements are equal pairwise.  Two empty
 * beta-vectors are equal.  \c NULL is not equal to anything, including other
 * \c NULL, use pointer comparison for that.
 * \param[in] beta1  First beta-vector.
 * \param[in] beta2  Second beta-vector.
 * \returns  \c 1 if two vectors are equal, \c 0 otherwise.
 */
int clay_beta_equals(clay_array_p beta1, clay_array_p beta2) {
  return (clay_beta_compare(beta1, beta2, NULL) == 0 &&
          beta1->size == beta2->size);
}

/**
 * Compare if \p child beta-vector is nested in \p parent beta-vector.
 * A beta-vector is considered nested if it is an exact beta-prefix of that
 * vector, i.e. the \p child starts with all of the elements of \p parent
 * beta-vector in the same order, and it has at least one more element.
 * \param[in] child  Nested beta-vector.
 * \param[in] parent Nesting beta-vector or beta-prefix.
 * \returns  \c 1 if \p parent is a beta-prefix of \p child, \c 0 otherwise.
 */
int clay_beta_is_nested(clay_array_p child, clay_array_p parent) {
  int depth;
  if (child->size <= parent->size) {
    return 0;
  }
  if (clay_beta_compare(child, parent, &depth) == 0) {
    if (depth == parent->size - 1) {
      return 1;
    }
  }
  return 0;
}

/**
 * Extracts a beta-vector from a scattering relation.
 * In case of scattering relation union considers only the union part being
 * pointed to by \p relation.
 * \param[in] relation  Part of scattering union of relations.
 * \returns  Beta vector corresponding to the scattering relation union part.
 * \warning Malformed scattering relation (not enough proper equalities) will
 * result in an error.
 */
clay_array_p clay_beta_extract(osl_relation_p relation) {
  int i, j, row, value;
  clay_array_p beta = clay_array_malloc();
  for (i = 0; i < relation->nb_output_dims; i += 2) {
    row = clay_util_relation_get_line(relation, i);
    if (row == -1)
      CLAY_error("Scattering matrix corrupted");
    // TODO: replace with _check_zeros when it is refactored
    //clay_scattering_check_zeros(relation, row, i + 1);
    
    for (j = 1; j < relation->nb_columns - 1; j++) {
      if (j != i + 1 &&
          !osl_int_zero(relation->precision, relation->m[row][j])) {
        osl_relation_dump(stderr, relation);
        CLAY_error("Scattering matrix corrupted");
      }
    }
    
    value = osl_int_get_si(relation->precision,
                           relation->m[row][relation->nb_columns - 1]);
    clay_array_add(beta, value);
  }
  return beta;
}

/**
 * Extracts all beta-vectors of a given statement.
 * Potentially each part of a scattering relation union corresponds to a
 * separate beta-vector.  This function returns a list of all unique
 * beta-vectors associated with the given statement.
 * \param[in] statement  Statement to find associated beta-vectors.
 * \returns  List of unique beta-vectors (set).
 */
clay_list_p clay_statement_betas(osl_statement_p statement) {
  osl_relation_p scattering;
  clay_list_p statement_betas = clay_list_malloc();
  scattering = statement->scattering;
  while (scattering != NULL) {
    clay_array_p beta = clay_beta_extract(scattering);
    if (!clay_list_contains(statement_betas, beta)) {
      clay_list_add(statement_betas, beta);
    }
    scattering = scattering->next;
  }
  return statement_betas;
}

/**
 * Checks beta-vector invariants in the scop.
 * Different statements must not have identical beta-statemetns.
 * \param[in] scop  Scop to check.
 * \returns  \c 1 if the scop is well-formed; \c 0 otherwise
 */
int clay_beta_sanity_check(osl_scop_p scop) {
  clay_list_p used_betas = clay_list_malloc(),
              statement_betas = clay_list_malloc();
  osl_statement_p statement = scop->statement;
  int i;
  while (statement != NULL) {
    // If any of the betas of this statement was used in any of previous
    // statements, the beta-vector uniquiness is violated and the scop is
    // malformed.
    statement_betas = clay_statement_betas(statement);
    for (i = 0; i < statement_betas->size; i++) {
      if (clay_list_contains(used_betas, statement_betas->data[i])) {
        fprintf(stderr, "[Clay] scop sanity check failed\n");
        // Cleanup before return
        clay_list_free(used_betas);
        clay_list_free(statement_betas);
        return 0;
      }
    }
    clay_list_cat(used_betas, statement_betas);
    statement_betas->size = 0;
    statement = statement->next;
  }
  clay_list_free(used_betas);
  return 1;
}

void clay_beta_list_sort(clay_list_p list) {
  int i, j, unused;
  clay_array_p tmp;
  for (i = 0; i < list->size; i++) {
    for (j = i + 1; j < list->size; j++) {
      if (clay_beta_compare(list->data[i], list->data[j], &unused) < 0) {
        tmp = list->data[i];
        list->data[i] = list->data[j];
        list->data[j] = tmp;
      }
    }
  }
}

