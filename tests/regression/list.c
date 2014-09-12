#include <clay/array.h>
#include <clay/list.h>

#include <stdio.h>

int list_contains_empty() {
  clay_list_p list = clay_list_malloc();
  clay_array_p array1 = clay_array_malloc();
  clay_array_p array2 = clay_array_malloc();
  clay_array_add(array2, 1);
  clay_array_add(array2, 2);
  if (clay_list_contains(list, array1))
    return 1;
  if (clay_list_contains(list, array2))
    return 2;
  return 0;
}

int list_contains_null() {
  clay_list_p list = clay_list_malloc();
  clay_array_p array = clay_array_malloc();
  if (clay_list_contains(NULL, array))
    return 1;
  if (clay_list_contains(list, NULL))
    return 2;
  if (clay_list_contains(NULL, NULL))
    return 3;
  return 0;
}

int list_contains_self() {
  clay_list_p list = clay_list_malloc();
  clay_list_p other_list = clay_list_malloc();
  clay_array_p array = clay_array_malloc();
  clay_array_add(array, 42);
  clay_array_add(array, 1);
  clay_array_p other_array = clay_array_clone(array);
  clay_array_add(other_array, 31415);
  clay_list_add(list, array);
  clay_list_add(other_list, other_array);
  if (!clay_list_contains(list, array))
    return 1;
  if (!clay_list_contains(other_list, other_array))
    return 2;
  if (clay_list_contains(list, other_array))
    return 3;
  if (clay_list_contains(other_list, array))
    return 4;
  return 0;
}

int list_contains_beta_prefix() {
  clay_list_p list = clay_list_malloc();
  clay_array_p array = clay_array_malloc();
  clay_array_add(array, 42);
  clay_array_add(array, 1);
  clay_array_p root = clay_array_malloc();
  clay_array_p parent = clay_array_malloc();
  clay_array_add(parent, 42);
  clay_list_add(list, array);
  if (clay_list_contains(list, root))
    return 1;
  if (clay_list_contains(list, parent))
    return 2;
  return 0;
}

int list_cat_null() {
  clay_list_p list = clay_list_malloc();
  // No segfault expected.
  clay_list_cat(NULL, list);
  clay_list_cat(list, NULL);
  clay_list_cat(NULL, NULL);
  return 0;
}

int list_cat_empty() {
  clay_list_p list1 = clay_list_malloc();
  clay_list_p list2 = clay_list_malloc();
  clay_list_cat(list1, list2);
  if (list1->size != 0)
    return 1;
  if (list2->size != 0)
    return 2;
  clay_array_p array1 = clay_array_malloc();
  clay_array_add(array1, 1);
  clay_list_add(list1, array1);
  clay_list_cat(list1, list2);
  if (list1->size != 1)
    return 3;
  if (list1->data[0]->size != 1)
    return 4;
  if (list1->data[0] != array1)
    return 5;
  clay_list_p list3 = clay_list_malloc();
  clay_list_cat(list1, list3);
  if (list1->size != 1)
    return 3;
  if (list1->data[0]->size != 1)
    return 4;
  if (list1->data[0] != array1)
    return 5;
  return 0;
}

int list_cat_self() {
  clay_list_p list1 = clay_list_malloc();
  clay_list_p list2 = clay_list_malloc();
  clay_array_p array1 = clay_array_malloc();
  clay_array_add(array1, 1);
  clay_array_p array2 = clay_array_malloc();
  clay_array_add(array2, 2);
  clay_array_p array3 = clay_array_malloc();
  clay_array_add(array3, 3);
  clay_array_p array4 = clay_array_malloc();
  clay_array_add(array4, 4);
  clay_array_p array5 = clay_array_malloc();
  clay_array_add(array5, 5);
  clay_list_add(list1, array1);
  clay_list_add(list1, array2);
  clay_list_add(list2, array3);
  clay_list_add(list2, array4);
  clay_list_add(list2, array5);
  clay_list_cat(list1, list2);
  if (list1->size != 5)
    return 1;
  if (list2->size != 3)
    return 2;
  if (list1->data[0] != array1)
    return 3;
  if (list1->data[1] != array2)
    return 4;
  if (list1->data[2] != array3)
    return 5;
  if (list1->data[3] != array4)
    return 6;
  if (list1->data[4] != array5)
    return 7;
  return 0;
}

int main() {
  int result = 0;
  result += list_contains_empty();
  result += list_contains_null();
  result += list_contains_self();
  result += list_contains_beta_prefix();
  result += list_cat_null();
  result += list_cat_empty();
  result += list_cat_self();
  if (result != 0)
    fprintf(stderr, "TEST FAILED\n");
  else
    fprintf(stdout, "Test Passed\n");
  fflush(stderr);
  fflush(stdout);
  return result;
}

