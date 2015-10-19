
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                           relation.c                               |
    |--------------------------------------------------------------------|
    |                    First version: 01/10/2015                       |
    +--------------------------------------------------------------------+

 +--------------------------------------------------------------------------+
 |  / __)(  )    /__\ ( \/ )                                                |
 | ( (__  )(__  /(__)\ \  /         Chunky Loop Alteration wizardrY         |
 |  \___)(____)(__)(__)(__)                                                 |
 +--------------------------------------------------------------------------+
 | Copyright (c) 2012 University of Paris-Sud                               |
 | Copyright (c) 2015 Inria                                                 |
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
 | Written by Oleksandr Zinenko, oleksandr.zinenko@inria.fr                 |
 +--------------------------------------------------------------------------*/

#include <clay/array.h>
#include <clay/errors.h>
#include <clay/macros.h>
#include <clay/relation.h>
#include <clay/util.h>

#include <osl/relation.h>

// aware of beta structure
static int clay_relation_line_is_zero(osl_relation_p relation, int row,
                                      int begin, int end) {
  int i;
  int all_zero = 1;
  for (i = begin; i < end; i++) {
    if (i >= 1 && i < 1 + relation->nb_output_dims && (i % 2) == 1) {
      continue; // ignore betas
    }
    all_zero = all_zero && osl_int_zero(relation->precision,
                                        relation->m[row][i]);
    if (!all_zero) {
      break;
    }
  }
  return all_zero;
}

// aware of beta structure
osl_int_t clay_relation_line_gcd(osl_relation_p relation, int line, int column_start, int column_end) {
  osl_int_t gcd, tmp;
  int i;
  int gcd_assigned = 0;

  osl_int_init(relation->precision, &gcd);
  osl_int_init(relation->precision, &tmp);

  if (column_end - column_start < 1 ||
      column_start >= relation->nb_columns ||
      column_end > relation->nb_columns) {
    osl_int_set_si(relation->precision, &gcd, 0);
    return gcd;
  } else if (column_end - column_start == 1) {
    osl_int_assign(relation->precision, &gcd, relation->m[line][column_start]);
    return gcd;
  }

  for (i = column_start; i < column_end; i++) {
    if (i >= 1 && i < relation->nb_output_dims + 1 && (i % 2) == 1) {
      continue; // ignore betas
    }
    if (osl_int_zero(relation->precision, relation->m[line][i])) {
      continue; // ignore zeros
    }
    osl_int_abs(relation->precision, &tmp, relation->m[line][i]);

    if (!gcd_assigned) {
      osl_int_assign(relation->precision, &gcd, tmp);
      gcd_assigned = 1;
    } else {
      osl_int_gcd(relation->precision, &gcd, gcd, tmp);
    }
  }

  if (!gcd_assigned) { // if gcd zero or not found, default to 1.
    osl_int_set_si(relation->precision, &gcd, 1);
  }

  osl_int_clear(relation->precision, &tmp);
  return gcd;
}

// assumes beta-structure; depth 1-based
// TODO: do not return, possible leak
osl_int_t clay_relation_gcd(osl_relation_p relation, int depth) {
  osl_int_t gcd;
  int row, col;
  int gcd_assigned = 0;
  int column = 2*depth;

  osl_int_init(relation->precision, &gcd);
  if (depth < -1 || depth == 0 || depth > relation->nb_output_dims / 2) {
  CLAY_debug("Called clay_relation_gcd with column outside bounds");
    osl_int_set_si(relation->precision, &gcd, 0);
    return gcd;
  }

  for (row = 0; row < relation->nb_rows; row++) {
    if (depth != -1 && osl_int_zero(relation->precision, relation->m[row][column])) {
      continue;
    }

    for (col = 2; col < relation->nb_columns; col++) {
      // if beta, ignore
      if (col >= 1 && col < relation->nb_output_dims + 1 && (col % 2) == 1) {
        continue;
      }
      if (col == column) {
        continue;
      }
      if (gcd_assigned) {
        osl_int_gcd(relation->precision, &gcd, gcd, relation->m[row][col]);
      } else {
        if (!osl_int_zero(relation->precision, relation->m[row][col])) {
          osl_int_assign(relation->precision, &gcd, relation->m[row][col]);
          gcd_assigned = 1;
        }
      }
    }
  }
  return gcd;
}

void clay_relation_normalize_alpha(osl_relation_p relation) {
  int row, col;
  osl_int_t gcd;

  osl_int_init(relation->precision, &gcd);

  // Normalize equalities.
  clay_relation_output_form(relation);

  // Normalize inequalities.
  for (row = 0; row < relation->nb_rows; row++) {
    if (osl_int_zero(relation->precision, relation->m[row][0])) {
      continue; // ignore equalities
    }
    gcd = clay_relation_line_gcd(relation, row, 1, relation->nb_columns);
    for (col = 1; col < relation->nb_columns; col++) {
      if (col >= 1 && col < relation->nb_output_dims + 1 && (col % 2) == 1) {
        continue; // ignore beta dimensions
      }
      osl_int_div_exact(relation->precision, &relation->m[row][col],
                        relation->m[row][col], gcd);
    }
  }

  clay_relation_sort_rows(relation);

  osl_int_clear(relation->precision, &gcd);
}

// make i-th coefficient in the row_i zero by subtracting row_j multiplied by
// a constant factor from row_i.
void clay_relation_zero_coefficient(osl_relation_p relation,
                                    int row_i, int row_j, int col) {
  int k;
  osl_int_t lcm, multiplier_i, multiplier_j;

  osl_int_init(relation->precision, &lcm);
  osl_int_init(relation->precision, &multiplier_i);
  osl_int_init(relation->precision, &multiplier_j);

  // If the target coefficient is already zero,
  if (osl_int_zero(relation->precision, relation->m[row_j][col])) {
    // do nothing.
  }
  // If the source coefficient is zero, but the target one is not,
  else if (osl_int_zero(relation->precision, relation->m[row_i][col])) {
    // swap lines.
    for (k = 1; k < relation->nb_columns; k++) {
      osl_int_swap(relation->precision, &relation->m[row_i][k],
                   &relation->m[row_j][k]);
    }
  } else {
    osl_int_lcm(relation->precision, &lcm,
                relation->m[row_i][col], relation->m[row_j][col]);
    osl_int_div_exact(relation->precision, &multiplier_i,
                      lcm, relation->m[row_i][col]);
    osl_int_div_exact(relation->precision, &multiplier_j,
                      lcm, relation->m[row_j][col]);
    for (k = 1; k < relation->nb_columns; k++) {
      if (k < relation->nb_output_dims + 1 && (k % 2) == 1) {
        // ignore beta dimensions
        continue;
      }
      osl_int_mul(relation->precision, &relation->m[row_i][k],
                  relation->m[row_i][k], multiplier_i);
      osl_int_mul(relation->precision, &relation->m[row_j][k],
                  relation->m[row_j][k], multiplier_j);
      osl_int_sub(relation->precision, &relation->m[row_j][k],
                  relation->m[row_j][k], relation->m[row_i][k]);
    }
  }
  // Divide by constant factor if possible (similar to densify transformation).
  multiplier_i = clay_relation_line_gcd(relation, row_i, 1,
                                        relation->nb_columns);
  multiplier_j = clay_relation_line_gcd(relation, row_j, 1,
                                        relation->nb_columns);
  for (k = 1; k < relation->nb_columns; k++) {
    if (k < relation->nb_output_dims + 1 && (k % 2) == 1) {
      // ignore beta dimensions
      continue;
    }
    if (!osl_int_zero(relation->precision, multiplier_i)) {
      osl_int_div_exact(relation->precision, &relation->m[row_i][k],
                        relation->m[row_i][k], multiplier_i);
    }
    if (!osl_int_zero(relation->precision, multiplier_j)) {
      osl_int_div_exact(relation->precision, &relation->m[row_j][k],
                        relation->m[row_j][k], multiplier_j);
    }
  }

  osl_int_clear(relation->precision, &lcm);
  osl_int_clear(relation->precision, &multiplier_i);
  osl_int_clear(relation->precision, &multiplier_j);
}

void clay_relation_alpha_equation_rows(clay_array_p equation_rows,
                                       osl_relation_p relation) {
  int i;

  // Take only alpha equations.
  for (i = 0; i < relation->nb_rows; i++) {
    if (osl_int_zero(relation->precision, relation->m[i][0])) {
      if (!clay_util_is_row_beta_definition(relation, i)) {
        clay_array_add(equation_rows, i);
      }
    }
  }
}

// assumes alpha-beta form is preserved, but makes no assumption on the
// (in)equality order of appearance.
int clay_relation_output_form(osl_relation_p relation) {
  clay_array_p equation_rows;
  int i, j, k, row_i, row_j;
  int nb_equations, nb_alpha_dims, nb_linearly_dependent_rows;
  int nb_steps;

  equation_rows = clay_array_malloc();
  clay_relation_alpha_equation_rows(equation_rows, relation);

  nb_equations = equation_rows->size;
  nb_alpha_dims = (relation->nb_output_dims - 1) / 2;
  nb_linearly_dependent_rows = 0;

  nb_steps = nb_alpha_dims < nb_equations ? nb_alpha_dims : nb_equations;

  // Forward pass of integer Gauss elimination: make the alpha-part of the
  // relation matrix upper triangular by subtracting lines with coefficients.
  // Multiply both lines by the GCD to avoid having fractional coefficients,
  // then divide the remaining numbers by their GCD to avoid explosion.
  for (i = 0; i < nb_steps; i++) {
    for (j = i + 1; j < nb_equations; j++) {
      row_i = equation_rows->data[i];
      row_j = equation_rows->data[j];
      clay_relation_zero_coefficient(relation, row_i, row_j, 2*i + 2);
    }
  }

  // Linearly dependent rows should now contain zeros.
  for (i = 0; i < nb_equations; i++) {
    row_i = equation_rows->data[i];
    if (clay_relation_line_is_zero(relation, row_i, 1,
                                   1 + relation->nb_output_dims)) {
      nb_linearly_dependent_rows += 1;
    }
  }

  // Reverse pass of integer Gauss elimination: make the alpha-part of the
  // relation matrix diagonal.
  for (i = nb_steps - 1; i >= 0; i--) {
    for (j = i - 1; j >= 0; j--) {
      row_i = equation_rows->data[i];
      row_j = equation_rows->data[j];
      clay_relation_zero_coefficient(relation, row_i, row_j, 2*i + 2);
    }
  }

  // Flip diagonal elements if necessary.
  for (i = 0; i < nb_steps; i++) {
    row_i = equation_rows->data[i];
    if (!osl_int_neg(relation->precision, relation->m[row_i][2*i + 2])) {
      for (k = 1; k < relation->nb_columns; k++) {
        if (k < 1 + relation->nb_output_dims && (k % 2) == 1) {
          continue; // ignore betas
        }
        osl_int_oppose(relation->precision, &relation->m[row_i][k],
                       relation->m[row_i][k]);
      }
    }
  }

  if (nb_equations - nb_linearly_dependent_rows > nb_alpha_dims) {
    CLAY_debug("relation is defined by overdetermined matrix");
    return -1;
  } else if (nb_equations - nb_linearly_dependent_rows < nb_alpha_dims) {
    // Relation is defined by underdetermined matrix, but this may be okay if
    // dimensions are defined implicitly.
  }

  // Check that diagonal elements are 1.  They should be after gcd compression,
  // otherwise the corresponding output dimension is invalid (fractional).
  for (i = 0; i < nb_equations; i++) {
    row_i = equation_rows->data[i];
    if (!osl_int_one(relation->precision, relation->m[row_i][2*i + 2])) {
      CLAY_debug("output form contains fractional dimension");
      return -2;
    }
  }

  clay_array_free(equation_rows);
  return 0;
}

// Do a copy of relation before intrusive rank computation.
int clay_relation_rank(osl_relation_p relation) {
  osl_relation_p copy = osl_relation_clone(relation);
  int rank = clay_relation_rank_intrusive(copy);
  osl_relation_free(copy);
  return rank;
}

// Converts relation into output form that turns linearly dependent lines zero
// as a side effect.
int clay_relation_rank_intrusive(osl_relation_p relation) {
  int row;
  int independent_lines = 0;
  clay_relation_output_form(relation);

  for (row = 0; row < relation->nb_rows; row++) {
    if (osl_int_one(relation->precision, relation->m[row][0]))
      continue; // skip inequalities
    if (clay_util_is_row_beta_definition(relation, row))
      continue; // skip beta equations
    if (!clay_relation_line_is_zero(relation, row, 1,
                                    1 + relation->nb_output_dims)) {
      ++independent_lines;
    }
  }

  return independent_lines;
}

int clay_relation_nb_explicit_dim(osl_relation_p relation) {
  int result;
  osl_relation_p copy = osl_relation_clone(relation);
  result = clay_relation_nb_explicit_dim_intrusive(copy);
  osl_relation_free(copy);
  return result;
}

int clay_relation_nb_explicit_dim_intrusive(osl_relation_p relation) {
  int i, j, row_i, row_j;
  clay_array_p equation_rows = clay_array_malloc();
  int rank = 0;

  clay_relation_alpha_equation_rows(equation_rows, relation);
  for (i = 0; i < relation->nb_input_dims; i++) {
    for (j = i + 1; j < equation_rows->size; j++) {
      row_i = equation_rows->data[i];
      row_j = equation_rows->data[j];
      clay_relation_zero_coefficient(relation, row_i, row_j,
                                     i + 1 + relation->nb_output_dims);
    }
  }

  for (i = 0; i < equation_rows->size; i++) {
    row_i = equation_rows->data[i];
    if (!clay_relation_line_is_zero(relation, row_i,
                    1 + relation->nb_output_dims,
                    1 + relation->nb_output_dims + relation->nb_input_dims)) {
      rank += 1;
    }
  }

  clay_array_free(equation_rows);
  return rank;
}

static int row_preceeds(osl_relation_p relation, int row_i, int row_j) {
  int k;

  for (k = 0; k < relation->nb_columns; k++) {
    if (osl_int_gt(relation->precision,
                   relation->m[row_i][k], relation->m[row_j][k])) {
      return 0;
    } else if (!osl_int_eq(relation->precision,
                           relation->m[row_i][k], relation->m[row_j][k])) {
      return 1;
    }
  }
  return 1;
}

void clay_relation_sort_rows(osl_relation_p relation) {
  // we do not need to sort equalities and inequalities separately since the e/i
  // flag already groups them; we assume it is in output form.
  int i, j, k;

  // XXX: Bubble sort is inefficient in time, but has tiny code, works in-place
  // and avoids creating sub-relations with dynamic memory allocation.
  for (i = 0; i < relation->nb_rows; i++) {
    for (j = i + 1; j < relation->nb_rows; j++) {
      if (row_preceeds(relation, j, i)) {
        // swap raws
        for (k = 0; k < relation->nb_columns; k++) {
          osl_int_swap(relation->precision,
                       &relation->m[i][k], &relation->m[j][k]);
        }
      }
    }
  }
}

