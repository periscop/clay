#include "bench.h"

#include <clay/beta.h>
#include <osl/osl.h>
#include <stdio.h>
#include <stdlib.h>

TEST_BENCH;

TEST1(beta_sanity_check, char *filename) {
  FILE *f = fopen(filename, "r");
  osl_scop_p scop = osl_scop_read(f);
  fclose(f);

  EXPECT_TRUE(clay_beta_sanity_check(scop), "");
  osl_scop_free(scop);
}

TEST(beta_sanity_check_fail) {
  FILE *f = fopen("malformed.scop", "r");
  osl_scop_p scop = osl_scop_read(f);
  fclose(f);

  EXPECT_TRUE(!clay_beta_sanity_check(scop),
              "Malformed scop passed the sanity check");
  osl_scop_free(scop);
}

TEST(beta_next_interleave) {
  FILE *f = fopen("trickypeel.after.scop", "r");
  ASSERT_TRUE(f != NULL, "Failed to open file trickypeel.after.scop");
  osl_scop_p scop = osl_scop_read(f);
  osl_statement_p stmt;

  clay_array_p *betas = (clay_array_p *) malloc(sizeof(clay_array_p *) * 4);
  betas[0] = clay_array_malloc();
  betas[1] = clay_array_malloc();
  betas[2] = clay_array_malloc();
  betas[3] = clay_array_malloc();

  clay_array_add(betas[0], 0);
  clay_array_add(betas[0], 0);

  clay_array_add(betas[1], 1);
  clay_array_add(betas[1], 0);

  clay_array_add(betas[2], 2);
  clay_array_add(betas[2], 0);

  clay_array_add(betas[3], 3);
  clay_array_add(betas[3], 0);

  clay_array_p beta = clay_array_malloc(), b1;
  int i;
  for (i = 0; i < 4; i++) {
    b1 = clay_beta_next(scop->statement, beta, &stmt);
    clay_array_free(beta);
    beta = b1;
    EXPECT_TRUE(clay_beta_equals(betas[i], beta), "Unexpected next beta");
    if ((i % 2) == 0) {
      EXPECT_TRUE(stmt == scop->statement, "Wrong statement found");
    } else {
      EXPECT_TRUE(stmt == scop->statement->next, "Wrong statement found");
    }
  }
  b1 = clay_beta_next(scop->statement, beta, &stmt);
  clay_array_free(beta);
  EXPECT_TRUE(b1 == NULL, "Next-beta cycle did not end with NULL");
  EXPECT_TRUE(stmt == NULL,
      "Next-beta cycle did not reutrn pointer to NULL statement at the end");

  free(betas);
  osl_scop_free(scop);
}

TEST(beta_next_empty) {
  osl_scop_p scop = osl_scop_malloc();
  scop->statement = osl_statement_malloc();
  osl_statement_p stmt;
  scop->statement->scattering = osl_relation_malloc(0, 4);

  clay_array_p beta = clay_array_malloc();
  clay_array_p next = clay_beta_next(scop->statement, beta, &stmt);
  EXPECT_TRUE(next == NULL, "Found a beta for an empty statement");
  EXPECT_TRUE(stmt == NULL, "Found a next statement for an empty one");

  clay_array_free(beta);
  osl_scop_free(scop);
}

TEST(beta_next_null) {
  osl_statement_p statement = osl_statement_malloc(), stmt;
  statement->scattering = osl_relation_malloc(0, 2);
  clay_array_p beta = clay_array_malloc(), next;

  next = clay_beta_next(NULL, beta, &stmt);
  EXPECT_TRUE(next == NULL, "Found a beta for a NULL statement");
  EXPECT_TRUE(stmt == NULL, "Found a next statement for a NULL one");

  next = clay_beta_next(statement, NULL, &stmt);
  EXPECT_TRUE(next == NULL, "Found next beta for a NULL beta");
  EXPECT_TRUE(stmt == NULL, "Found a next statement for a NULL beta");

  next = clay_beta_next(NULL, NULL, &stmt);
  EXPECT_TRUE(next == NULL, "Found a beta for a NULL statement");
  EXPECT_TRUE(stmt == NULL, "Found a next statement for a NULL one");

  clay_array_free(beta);
  osl_statement_free(statement);
}

TEST(statement_relations_interleave) {
  FILE *f = fopen("trickypeel.after.scop", "r");
  ASSERT_TRUE(f != NULL, "Failed to open file trickypeel.after.scop");
  osl_scop_p scop = osl_scop_read(f);
  fclose(f);

  // stmt1 = beta1, beta3;  stmt2 = beta2, beta4;  betas are ordered
  clay_array_p beta3 = clay_beta_extract(scop->statement->scattering);
  clay_array_p beta4 = clay_beta_extract(scop->statement->next->scattering);
  clay_array_p beta1 = clay_beta_extract(scop->statement->scattering->next);
  clay_array_p beta2 = clay_beta_extract(scop->statement->next->scattering->next);

  int i;
  for (i = 0; i < 2; i++) {
    EXPECT_TRUE(!clay_statement_is_after(scop->statement, beta1),
                "Statement is after itself");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement, beta2),
                "Statement is after another, which interleaves it");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement, beta3),
                "Statement is after itself");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement, beta4),
                "Statement is after another, but should be before");

    EXPECT_TRUE(!clay_statement_is_before(scop->statement, beta1),
                "Statement is before itself");
    EXPECT_TRUE(!clay_statement_is_before(scop->statement, beta2),
                "Statement is before another, which interleaves it");
    EXPECT_TRUE(!clay_statement_is_before(scop->statement, beta3),
                "Statement is before itself");
    EXPECT_TRUE(clay_statement_is_before(scop->statement, beta4),
                "Statement is not before the following beta");

    EXPECT_TRUE(clay_statement_is_after(scop->statement->next, beta1),
                "Statement is not after the previous beta");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement->next, beta2),
                "Statement is after itself");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement->next, beta3),
                "Statement is after another, which interleaves it");
    EXPECT_TRUE(!clay_statement_is_after(scop->statement->next, beta4),
                "Statement is after itself");

    EXPECT_TRUE(!clay_statement_is_before(scop->statement->next, beta1),
                "Statement is before another, but should be after");
    EXPECT_TRUE(!clay_statement_is_before(scop->statement->next, beta2),
                "Statement is before itself");
    EXPECT_TRUE(!clay_statement_is_before(scop->statement->next, beta3),
                "Statement is before another, which interleaves it");
    EXPECT_TRUE(!clay_statement_is_before(scop->statement->next, beta4),
                "Statement is before itself");

    EXPECT_TRUE(!clay_statement_interleaves(scop->statement, beta1),
                "Statement interleaves itself");
    EXPECT_TRUE(clay_statement_interleaves(scop->statement, beta2),
                "Statement doesn't interleave the beta it should");
    EXPECT_TRUE(!clay_statement_interleaves(scop->statement, beta3),
                "Statement interleaves itself");
    EXPECT_TRUE(!clay_statement_interleaves(scop->statement, beta4),
                "Statement interleaves the following beta");

    EXPECT_TRUE(!clay_statement_interleaves(scop->statement->next, beta1),
                "Statement interleaves the previous beta");
    EXPECT_TRUE(!clay_statement_interleaves(scop->statement->next, beta2),
                "Statement interleaves itself");
    EXPECT_TRUE(clay_statement_interleaves(scop->statement->next, beta3),
                "Statement doesn't interleave the beta it should");
    EXPECT_TRUE(!clay_statement_interleaves(scop->statement->next, beta4),
              "Statement interleaves itself");
    beta1->size = 1;
    beta2->size = 1;
    beta3->size = 1;
    beta4->size = 1;
  }


  clay_array_free(beta1);
  clay_array_free(beta2);
  clay_array_free(beta3);
  clay_array_free(beta4);
  osl_scop_free(scop);
}

int main() {
  RUN_TEST(beta_next_interleave);
  RUN_TEST(beta_next_null);
  RUN_TEST(beta_next_empty);
  RUN_TEST(statement_relations_interleave);
  RUN_TEST1(beta_sanity_check, "trickypeel.before.scop");
  RUN_TEST1(beta_sanity_check, "trickypeel.after.scop");
  RUN_TEST1(beta_sanity_check, "peel.before.scop");
  RUN_TEST1(beta_sanity_check, "peel.after.scop");
  RUN_TEST(beta_sanity_check_fail);
  TEST_SUMMARY;
  return TEST_RESULT;
}
