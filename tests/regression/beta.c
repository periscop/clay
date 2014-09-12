#include <clay/clay.h>
#include <clay/beta.h>

#include <stdio.h>
#include <limits.h>

// Empty beta compares to anything
int beta_compare_self() {
  int depth;
  clay_array_p beta1 = clay_array_malloc();
  clay_array_p beta2 = clay_array_malloc();
  clay_array_p beta3 = clay_array_malloc();
  clay_array_p beta4 = clay_array_malloc();
  clay_array_add(beta2, 42);
  clay_array_add(beta2, 214);

  clay_array_add(beta3, 42);
  clay_array_add(beta3, 214);
  clay_array_add(beta3, 0);

  clay_array_add(beta4, 21);
  clay_array_add(beta4, 214);

  // Compare empty beta with itself.
  if (clay_beta_compare(beta1, beta1, &depth) != 0) {
    return 1;
  }
  if (depth != -1) {
    return 101;
  }

  // Compare filled beta with itself.
  if (clay_beta_compare(beta2, beta2, &depth) != 0) {
    return 2;
  }
  if (depth != beta2->size - 1) {
    return 102;
  }

  // Compare empty to non-emtpy beta.
  if (clay_beta_compare(beta1, beta2, &depth) != 0) {
    return 3;
  }
  if (depth != -1) {
    return 103;
  }
  if (clay_beta_compare(beta2, beta1, &depth) != 0) {
    return 4;
  }
  if (depth != -1) {
    return 104;
  }

  // Compare betas with same prefix.
  if (clay_beta_compare(beta2, beta3, &depth) != 0) {
    return 5;
  }
  if (depth != 1) {
    return 105;
  }
  if (clay_beta_compare(beta3, beta2, &depth) != 0) {
    return 6;
  }
  if (depth != 1) {
    return 106;
  }

  // Compare different betas.
  if (clay_beta_compare(beta2, beta4, &depth) != -21) {
    return 7;
  }
  if (depth != 0) {
    return 107;
  }
  if (clay_beta_compare(beta4, beta2, &depth) != 21) {
    return 8;
  }
  if (depth != 0) {
    return 108;
  }
  return 0;
}

int beta_compare_null() {
  int depth;
  clay_array_p beta = clay_array_malloc();
  if (clay_beta_compare(beta, NULL, &depth) != INT_MAX) {
    return 1;
  }
  if (depth != -2) {
    return 101;
  }

  if (clay_beta_compare(NULL, beta, &depth) != INT_MAX) {
    return 2;
  }
  if (depth != -2) {
    return 102;
  }
  if (clay_beta_compare(NULL, NULL, &depth) != INT_MAX) {
    return 3;
  }
  if (depth != -2) {
    return 103;
  }
  return 0;
}

int beta_equals_self() {
  clay_array_p beta1 = clay_array_malloc();
  clay_array_add(beta1, 1);
  clay_array_add(beta1, 42);
  clay_array_p beta2 = clay_array_malloc();
  clay_array_p beta3 = clay_array_clone(beta1);
  clay_array_p beta4 = clay_array_malloc();
  clay_array_add(beta4, 1);
  // Compare to self.
  if (!clay_beta_equals(beta1, beta1)) {
    return 1;
  }
  if (!clay_beta_equals(beta1, beta3)) {
    return 2;
  }
  if (!clay_beta_equals(beta3, beta1)) {
    return 3;
  }
  if (!clay_beta_equals(beta2, beta2)) {
    return 4;
  }
  if (clay_beta_equals(beta1, beta2)) {
    return 5;
  }
  if (clay_beta_equals(beta2, beta1)) {
    return 6;
  }
  if (clay_beta_equals(beta1, beta4)) {
    return 7;
  }
  if (clay_beta_equals(beta4, beta1)) {
    return 8;
  }
  clay_array_add(beta4, 43);
  if (clay_beta_equals(beta1, beta4)) {
    return 9;
  }
  if (clay_beta_equals(beta4, beta1)) {
    return 10;
  }
  return 0;
}

int beta_equals_null() {
  clay_array_p beta = clay_array_malloc();
  if (clay_beta_equals(NULL, beta)) {
    return 1;
  }
  if (clay_beta_equals(beta, NULL)) {
    return 2;
  }
  if (clay_beta_equals(NULL, NULL)) {
    return 3;
  }
  return 0;
}

int main() {
  return beta_compare_self() + beta_compare_null() + beta_equals_self() +
         beta_equals_null();
}
