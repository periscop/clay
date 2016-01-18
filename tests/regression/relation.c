#include "bench.h"

#include <osl/osl.h>
#include <clay/clay.h>
#include <clay/util.h>
#include <clay/relation.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

TEST_BENCH;

TEST1(alpha_normalization_random, osl_relation_p relation) {
  int i, k;
  osl_int_t tmp;
  clay_array_p alpha_equalities = clay_array_malloc();

  osl_relation_p original = osl_relation_clone(relation);

  // Add lines with random coefficients to alpha lines and verify the normalized
  // form is still the same.
  clay_relation_normalize_alpha(relation);
  osl_int_init(original->precision, &tmp);
  for (i = 0; i < original->nb_rows; i++) {
    if (osl_int_zero(original->precision, original->m[i][0]) &&
        !clay_util_is_row_beta_definition(original, i)) {
      clay_array_add(alpha_equalities, i);
    }
  }
  for (i = 0; i < 20; i++) {
    int line_1 = rand() % alpha_equalities->size;
    int line_2 = rand() % alpha_equalities->size;
    int coefficient = (rand() % 10) - 5;
    // avoid cancelling itself out
    if (line_1 == line_2 && coefficient == -1) {
      coefficient = 1;
    }
    line_1 = alpha_equalities->data[line_1];
    line_2 = alpha_equalities->data[line_2];

    for (k = 1; k < original->nb_columns; k++) {
      if (k < 1 + original->nb_output_dims && (k % 2) == 1)
        continue;

      osl_int_mul_si(original->precision, &tmp,
                     original->m[line_2][k], coefficient);
      osl_int_add(original->precision, &original->m[line_1][k],
                  original->m[line_1][k], tmp);
    }

    osl_relation_p other = osl_relation_clone(original);
    clay_relation_normalize_alpha(other);
    EXPECT_TRUE(osl_relation_equal(relation, other),
                "Normalized forms are not equal after invariant transformation");
    osl_relation_free(other);
  }

  clay_array_free(alpha_equalities);
  osl_relation_free(original);
}


int main() {
  char *relation1_str = 
"SCATTERING\n"
"5 11 5 2 0 2\n"
"# e/i| c1   c2   c3   c4   c5 |  i    j |  N    M |  1  \n"
"   0   -1    0    0    0    0    0    0    0    0    0    ## c1 == 0\n"
"   0    0    0    0   -3    0    0    3    0    0    0    ## 3*c4 == 3*j\n"
"   0    0    0   -1    0    0    0    0    0    0    0    ## c3 == 0\n"
"   0    0   -4    0   -1    0    4    1    0    0    0    ## 4*c2+c4 == 4*i+j\n"
"   0    0    0    0    0   -1    0    0    0    0    0    ## c5 == 0\n";

  char *relation2_str = 
"SCATTERING\n"
"9 19 9 4 0 4\n"
"# e/i| c1   c2   c3   c4   c5   c6   c7   c8   c9 |  i    j    k    l |  N    M    X    Y |  1  \n"
"   0   -1    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    ## c1 == 0\n"
"   0    0   -1    0    0    0    0    0    0    0    1    0    0    0    0    0    0    0    0    ## c2 == i\n"
"   0    0    0   -1    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    ## c3 == 0\n"
"   0    0    0    0   -1    0    0    0    0    0    0    1    0    0    0    0    0    0    0    ## c4 == j\n"
"   0    0    0    0    0   -1    0    0    0    0    0    0    0    0    0    0    0    0    0    ## c5 == 0\n"
"   0    0    0    0    0    0   -1    0    0    0    0    0    1    0    0    0    0    0    0    ## c6 == k\n"
"   0    0    0    0    0    0    0   -1    0    0    0    0    0    0    0    0    0    0    0    ## c7 == 0\n"
"   0    0    0    0    0    0    0    0   -1    0    0    0    0    1    0    0    0    0    0    ## c8 == l\n"
"   0    0    0    0    0    0    0    0    0   -1    0    0    0    0    0    0    0    0    0    ## c9 == 0\n";

  osl_relation_p relation1 = osl_relation_sread(&relation1_str);
  osl_relation_p relation2 = osl_relation_sread(&relation2_str);

  srand(time(0));
  RUN_TEST1(alpha_normalization_random, relation1);
  RUN_TEST1(alpha_normalization_random, relation2);
  TEST_SUMMARY;

  osl_relation_free(relation1);
  osl_relation_free(relation2);

  return TEST_RESULT;
}
