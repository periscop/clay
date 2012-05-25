#include <stdlib.h>
#include <clay/functions.h>
#include <clay/prototype_function.h>


// Authorized functions in Clay


int clay_function_fission_type[]     =  {ARRAY_T, INTEGER_T};
int clay_function_reorder_type[]     =  {ARRAY_T, ARRAY_T};
int clay_function_interchange_type[] =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_reversal_type[]    =  {ARRAY_T, INTEGER_T};
int clay_function_fuse_type[]        =  {ARRAY_T};
int clay_function_skew_type[]        =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_iss_type[]         =  {ARRAY_T, ARRAY_T};
int clay_function_stripmine_type[]   =  {ARRAY_T, INTEGER_T, INTEGER_T, 
                                         INTEGER_T};
int clay_function_unroll_type[]      =  {ARRAY_T, INTEGER_T};
int clay_function_tile_type[]        =  {ARRAY_T, INTEGER_T, INTEGER_T,
                                         INTEGER_T, INTEGER_T};
int clay_function_shift_type[]       =  {ARRAY_T, INTEGER_T, ARRAY_T};
int clay_function_peel_type[]        =  {ARRAY_T, ARRAY_T, INTEGER_T};


// That is just the prototype of each functions, so there are no data for args
const clay_prototype_function_t functions[CLAY_FUNCTIONS_TOTAL] = 
{
  {
    "fission",     "fission(ident, uint depth)",
    NULL, clay_function_fission_type, 2, 2
  },
  {
    "reorder",     "reorder(ident_loop, array order)",
    NULL, clay_function_reorder_type, 2, 2
  },
  {
    "interchange", "interchange(ident, uint depth_1, uint depth_2)",
    NULL, clay_function_interchange_type, 3, 3
  },
  {
    "reversal",    "reversal(ident, uint depth)",
    NULL, clay_function_reversal_type, 2, 2
  },
  {
    "fuse",        "fuse(ident_loop)",
    NULL, clay_function_fuse_type, 1, 1
  },
  {
    "skew",        "skew(ident, uint depth, int coeff)",
    NULL, clay_function_skew_type, 3, 3
  },
  {
    "iss",         "iss(ident, array equation)",
    NULL, clay_function_iss_type, 2, 2
  },
  {
    "stripmine",   "stripmine(ident, uint depth, uint block, bool pretty)",
    NULL, clay_function_stripmine_type, 4, 4
  },
  {
    "unroll",      "unroll(ident_loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 2
  },
  {
    "tile",        "tile(ident, uint depth, uint depth_outer, uint block, bool pretty)",
    NULL, clay_function_tile_type, 5, 5
  },
  {
    "shift",      "shift(ident, uint depth, array vector)",
    NULL, clay_function_shift_type, 3, 3
  },
  {
    "peel",       "peel(ident_loop, array peeling, bool peel_before)",
    NULL, clay_function_peel_type, 3, 3
  }
};
