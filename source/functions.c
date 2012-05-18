#include <stdlib.h>
#include <clay/functions.h>
#include <clay/prototype_function.h>


int clay_function_fission_type[]     =  {ARRAY_T, INTEGER_T};
int clay_function_reorder_type[]     =  {ARRAY_T, ARRAY_T};
int clay_function_interchange_type[] =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_reversal_type[]    =  {ARRAY_T, INTEGER_T};
int clay_function_fuse_type[]        =  {ARRAY_T};
int clay_function_skew_type[]        =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_iss_type[]         =  {ARRAY_T, ARRAY_T};
int clay_function_stripmine_type[]   =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_unroll_type[]      =  {ARRAY_T, INTEGER_T};


// Authorized functions in Clay
// That is just the prototype of each functions, so there are no 
// data for args
const clay_prototype_function_t functions[CLAY_FUNCTIONS_TOTAL] = 
{
  {
    "fission",     "fission(array beta, uint depth)",
    NULL, clay_function_fission_type, 2, 2
  },
  {
    "reorder",     "reorder(array beta_loop, array order)",
    NULL, clay_function_reorder_type, 2, 2
  },
  {
    "interchange", "interchange(array beta, uint depth_1, uint depth_2)",
    NULL, clay_function_interchange_type, 3, 3
  },
  {
    "reversal",    "reversal(array beta, uint depth)",
    NULL, clay_function_reversal_type, 2, 2
  },
  {
    "fuse",        "fuse(array beta_loop)",
    NULL, clay_function_fuse_type, 1, 1
  },
  {
    "skew",        "skew(array beta, uint depth, int coeff)",
    NULL, clay_function_skew_type, 3, 3
  },
  {
    "iss",         "iss(array beta, array equation)",
    NULL, clay_function_iss_type, 2, 2
  },
  {
    "stripmine",   "stripmine(array beta, uint block, bool pretty)",
    NULL, clay_function_stripmine_type, 3, 3
  },
  {
    "unroll",      "unroll(array beta_loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 2
  }
};
