#include <stdlib.h>
#include <clay/functions.h>
#include <clay/prototype_function.h>


// Authorized functions in Clay


int clay_function_split_type[]           =  {ARRAY_T, INTEGER_T};
int clay_function_reorder_type[]         =  {ARRAY_T, ARRAY_T};
int clay_function_interchange_type[]     =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_reverse_type[]         =  {ARRAY_T, INTEGER_T};
int clay_function_fuse_type[]            =  {ARRAY_T};
int clay_function_skew_type[]            =  {ARRAY_T, INTEGER_T, INTEGER_T};
int clay_function_iss_type[]             =  {ARRAY_T, ARRAY_T};
int clay_function_stripmine_type[]       =  {ARRAY_T, INTEGER_T, INTEGER_T, 
                                             INTEGER_T};
int clay_function_unroll_type[]          =  {ARRAY_T, INTEGER_T};
int clay_function_tile_type[]            =  {ARRAY_T, INTEGER_T, INTEGER_T,
                                             INTEGER_T, INTEGER_T};
int clay_function_shift_type[]           =  {ARRAY_T, INTEGER_T,
                                             ARRAY_OR_INTEGER_T};
int clay_function_peel_type[]            =  {ARRAY_T, ARRAY_OR_INTEGER_T};
int clay_function_context_type[]         =  {ARRAY_T};


// That is just the prototype of each functions, so there are no data for args
const clay_prototype_function_t functions[CLAY_FUNCTIONS_TOTAL] = 
{
  {
    "split",       "split(ident, uint depth)",
    NULL, clay_function_split_type, 2, 2
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
    "reverse",     "reverse(ident, uint depth)",
    NULL, clay_function_reverse_type, 2, 2
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
    "stripmine",   "stripmine(ident, uint depth, uint size, bool pretty)",
    NULL, clay_function_stripmine_type, 4, 4
  },
  {
    "unroll",      "unroll(ident_loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 2
  },
  {
    "unroll_noepilog", "unroll_noepilog(ident_loop, uint factor)",
    NULL, clay_function_unroll_type, 2, 2
  },
  {
    "tile",        "tile(ident, uint depth, uint depth_outer, uint size, bool pretty)",
    NULL, clay_function_tile_type, 5, 5
  },
  {
    "shift",      "shift(ident, uint depth, array|int vector)",
    NULL, clay_function_shift_type, 3, 3
  },
  {
    "peel_first", "peel_first(ident_loop, array|int peeling)",
    NULL, clay_function_peel_type, 2, 2
  },
  {
    "peel_last",  "peel_last(ident_loop, array|int peeling)",
    NULL, clay_function_peel_type, 2, 2
  },
  {
    "context", "context(array vector)",
    NULL, clay_function_context_type, 1, 1
  }
};
