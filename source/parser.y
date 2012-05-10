
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             Clay.c                                 |
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


%{

  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <osl/scop.h>
  #include <osl/statement.h>
  #include <clay/macros.h>
  #include <clay/array.h>
  #include <clay/transformation.h>
  #include <clay/prototype_function.h>
  #include <parser.h>
  
  
  // Scanner declarations
  void                clay_scanner_free();
  void                clay_scanner_initialize();
  extern int          scanner_line;
  
  
  // Arguments list of the current scanned function
  clay_prototype_function_p  params;
  
  // Current scop
  osl_scop_p          parser_scop;
  
  // Other
  int                 yyerror();
  extern              FILE* yyin;
  
  // parser functions
  void                clay_parser(osl_scop_p, FILE*);
  void                clay_parser_exec_function(char *name);
  
  
  // Authorized functions in Clay
  
  #define CLAY_PROTOTYPE_FUNCTIONS_TOTAL        9
  #define CLAY_PROTOTYPE_FUNCTION_FISSION       0
  #define CLAY_PROTOTYPE_FUNCTION_REORDER       1
  #define CLAY_PROTOTYPE_FUNCTION_INTERCHANGE   2
  #define CLAY_PROTOTYPE_FUNCTION_REVERSAL      3
  #define CLAY_PROTOTYPE_FUNCTION_FUSE          4
  #define CLAY_PROTOTYPE_FUNCTION_SKEW          5
  #define CLAY_PROTOTYPE_FUNCTION_ISS           6
  #define CLAY_PROTOTYPE_FUNCTION_STRIPMINE     7
  #define CLAY_PROTOTYPE_FUNCTION_UNROLL        8

  int parser_fission_type[]     =  {ARRAY_T, INTEGER_T};
  int parser_reorder_type[]     =  {ARRAY_T, ARRAY_T};
  int parser_interchange_type[] =  {ARRAY_T, INTEGER_T, INTEGER_T};
  int parser_reversal_type[]    =  {ARRAY_T};
  int parser_fuse_type[]        =  {ARRAY_T};
  int parser_skew_type[]        =  {ARRAY_T, INTEGER_T, INTEGER_T};
  int parser_iss_type[]         =  {ARRAY_T, ARRAY_T};
  int parser_stripmine_type[]   =  {ARRAY_T, INTEGER_T, INTEGER_T};
  int parser_unroll_type[]      =  {ARRAY_T, INTEGER_T};
  
  // That is just the prototype of each functions, so there are no 
  // data for args
  const clay_prototype_function_t functions[CLAY_PROTOTYPE_FUNCTIONS_TOTAL] = 
    {
      {
        "fission",     "fission(array beta, uint depth)",
        NULL, parser_fission_type, 2, 2
      },
      {
        "reorder",     "retorder(array beta_loop, array order)",
        NULL, parser_reorder_type, 2, 2
      },
      {
        "interchange", "interchange(array beta, uint depth_1, uint depth_2)",
        NULL, parser_interchange_type, 3, 3
      },
      {
        "reversal",    "reversal(array beta)",
        NULL, parser_reversal_type, 1, 1
      },
      {
        "fuse",        "fuse(array beta_loop)",
        NULL, parser_fuse_type, 1, 1
      },
      {
        "skew",        "skew(array beta, uint depth, int coeff)",
        NULL, parser_skew_type, 3, 3
      },
      {
        "iss",         "iss(array beta, array equation)",
        NULL, parser_iss_type, 2, 2
      },
      {
        "stripmine",   "stripmine(array beta, uint block, bool pretty)",
        NULL, parser_stripmine_type, 3, 3
      },
      {
        "unroll",      "unroll(array beta, uint factor)",
        NULL, parser_unroll_type, 2, 2
      }
    };
  
  
  
%}

%union { int ival; } 
%union { char *sval; }

%token <ival> INTEGER
%token <sval> FUNCNAME 
%token COMMENT

%start start

%%

start:
	|
	  line start
	;

line: '\n'
  | 
    COMMENT '\n'
  |
    FUNCNAME '(' args ')' ';' COMMENT '\n'
    {
      clay_parser_exec_function($1);
      free($1);
    }
  |
    FUNCNAME '(' args ')' ';'
    {
      clay_parser_exec_function($1);
      free($1);
    }
  ;

args: // epsilon
  | 
    args ',' INTEGER 
    {
      int *tmp;
      CLAY_malloc(tmp, int*, sizeof(int));
      *tmp = $3;
      clay_prototype_function_args_add(params, tmp, INTEGER_T);
      //fprintf(stderr, "INTEGER %d\n", $3);
    }
  |
    INTEGER
    {
      int *tmp;
      CLAY_malloc(tmp, int*, sizeof(int));
      *tmp = $1;
      clay_prototype_function_args_add(params, tmp, INTEGER_T);
      //fprintf(stderr, "INTEGER %d\n", $1);
    }
  |
    args ',' array
  | array
  ;

array:
    '[' list ']'
  ;

// The array is created in the lex file
list:
    list ',' INTEGER
    {
      clay_array_add((clay_array_p) params->args[params->argc-1], $3);
      //fprintf(stderr, "ADD IN ARRAY[] %d\n", $3);
    }
  |
    INTEGER
    {
      clay_array_add((clay_array_p) params->args[params->argc-1], $1);
      //fprintf(stderr, "ADD IN ARRAY[] %d\n", $1);
    }
  |
  ;

%%


/**
 * yyerror function
 */
int yyerror(void) {
  fprintf(stderr,"[Clay] Error: syntax on line %d, maybe you forgot a `;'\n",
          scanner_line+1);
  exit(1);
}


/**
 * clay_parser_file function:
 * \param[in] input    Input file of the script
 */
void clay_parser_file(osl_scop_p scop, FILE *input) {
  parser_scop = scop;
  
  // List of parameters of the current scanned function
  params = clay_prototype_function_malloc();
  
  yyin = input;
  clay_scanner_initialize();
  yyparse();
  
  // Quit
  clay_scanner_free();
  clay_prototype_function_free(params);
}


/**
 * clay_parser_string function:
 * \param[in] input    Input string 
 */
void clay_parser_string(osl_scop_p scop, char *input) {
  parser_scop = scop;
  
  // List of parameters of the current scanned function
  params = clay_prototype_function_malloc();
  
  yy_scan_string(input);
  scanner_line = 0;
  yyparse();
  
  // Quit
  clay_scanner_free();
  clay_prototype_function_free(params);
}


/**
 * clay_parser_exec_function:
 */
void clay_parser_exec_function(char *name) {
  int i, j;
   
  /*
  fprintf(stderr, "%s\n", $1);
  fprintf(stderr, "nb args : %d\n", params->argc);
  
  if (params->argc > 0) {
    fprintf(stderr, "args ");
    clay_array_p tmp;
    for (i = 0 ; i < params->argc ; i++) {
      switch (params->type[i]) {
        case INTEGER_T:
          fprintf(stderr, "%d,", *(((int**)params->args)[i]));
          break;
        case ARRAY_T:
          tmp = (clay_array_p) params->args[i];
          clay_array_print(stderr, tmp);
          fprintf(stderr, ",");
          break;
      }
    }
    fprintf(stderr, "\n");
  }
  */
  
  i = 0;
  while (i < CLAY_PROTOTYPE_FUNCTIONS_TOTAL) {
    if (strcmp(functions[i].name, name) == 0)
      break;
    i++;
  }
  
  // Undefined function
  if (i == CLAY_PROTOTYPE_FUNCTIONS_TOTAL) {
      fprintf(stderr, "[Clay] Error: line %d, unknown function `%s'\n", 
              scanner_line, name);
      exit(1);
  }
  
  // Different number of parameters
  if (params->argc != functions[i].argc) {
      fprintf(stderr, 
        "[Clay] Error: line %d, in `%s' takes %d arguments\n[Clay] \
prototype is: %s\n", 
        scanner_line, name, functions[i].argc, functions[i].prototype);
      exit(1);
  }
  
  j = 0;
  while (j < functions[i].argc) {
    if (params->type[j] != functions[i].type[j])
      break;
    j++;
  }
  
  // Invalid type
  if (j != functions[i].argc) {
      fprintf(stderr, 
        "[Clay] Error: line %d, in function `%s' invalid type on argument \
%d\n[Clay] prototype is: %s\n", 
        scanner_line, name, j+1, functions[i].prototype);
      exit(1);
  }
  
  //fprintf(stderr, "[Clay] Exec %s\n", function[i].name);
  
  int status_result;
  switch (i) {
    case CLAY_PROTOTYPE_FUNCTION_FISSION:
      status_result = clay_fission(parser_scop, 
                                   params->args[0], 
                                   *((int*)params->args[1]));
      break;
    case CLAY_PROTOTYPE_FUNCTION_REORDER:
      status_result = clay_reorder(parser_scop, 
                                   params->args[0], 
                                   params->args[1]);
      break;
    case CLAY_PROTOTYPE_FUNCTION_INTERCHANGE:
      status_result = clay_interchange(parser_scop, 
                                       params->args[0], 
                                       *((int*)params->args[1]),
                                       *((int*)params->args[2]));
      break;
    case CLAY_PROTOTYPE_FUNCTION_REVERSAL:
      status_result = clay_reversal(parser_scop, 
                                    params->args[0]);
      break;
    case CLAY_PROTOTYPE_FUNCTION_FUSE:
      status_result = clay_fuse(parser_scop,
                                params->args[0]);
      break;
    case CLAY_PROTOTYPE_FUNCTION_SKEW:
      status_result = clay_skew(parser_scop,
                                params->args[0], 
                                *((int*)params->args[1]),
                                *((int*)params->args[2]));
      break;
    case CLAY_PROTOTYPE_FUNCTION_ISS:
      status_result = clay_iss(parser_scop,
                               params->args[0], 
                               params->args[1]);
      break;
    case CLAY_PROTOTYPE_FUNCTION_STRIPMINE:
      status_result = clay_stripmine(parser_scop,
                                     params->args[0], 
                                     *((int*)params->args[1]),
                                     *((int*)params->args[2]));
      break;
    case CLAY_PROTOTYPE_FUNCTION_UNROLL:
      status_result = clay_unroll(parser_scop,
                                  params->args[0], 
                                  *((int*)params->args[1]));
      break;
  }
  
  switch (status_result) {
    case CLAY_TRANSF_BETA_NOT_FOUND:
      fprintf(stderr,"[Clay] Error: line %d: the beta vector was not found\n",
              scanner_line);
      exit(1);
      break;
    case CLAY_TRANSF_NOT_BETA_LOOP:
      fprintf(stderr,"[Clay] Error: line %d: the beta is not a loop\n",
              scanner_line);
      exit(2);
      break;
    case CLAY_TRANSF_NOT_BETA_STMT:
      fprintf(stderr,"[Clay] Error: line %d, the beta is not a statement\n", 
              scanner_line);
      exit(3);
      break;
    case CLAY_TRANSF_REORDER_ARRAY_TOO_SMALL:
      fprintf(stderr,"[Clay] Error: line %d, the order array is too small\n", 
              scanner_line);
      exit(4);
      break;
    case CLAY_TRANSF_DEPTH_OVERFLOW:
      fprintf(stderr,"[Clay] Error: line %d, depth overflow\n",
              scanner_line);
      exit(5);
      break;
    case CLAY_TRANSF_WRONG_COEFF:
      fprintf(stderr,"[Clay] Error: line %d, wrong coefficient\n",
              scanner_line);
      exit(6);
      break;
    case CLAY_TRANSF_BETA_EMPTY:
      fprintf(stderr,"[Clay] Error: line %d, the beta vector is empty\n",
              scanner_line);
      exit(7);
      break;
    case CLAY_TRANSF_BETA_NOT_IN_A_LOOP:
      fprintf(stderr,"[Clay] Error: line %d, the beta need to be in a loop\n",
              scanner_line);
      exit(8);
      break;
    case CLAY_TRANSF_WRONG_BLOCK_SIZE:
      fprintf(stderr,"[Clay] Error: line %d, block value is incorrect\n",
              scanner_line);
      exit(9);
      break;
    case CLAY_TRANSF_WRONG_FACTOR:
      fprintf(stderr,"[Clay] Error: line %d, wrong factor\n",
              scanner_line);
      exit(10);
      break;
  }
  
  clay_prototype_function_args_clear(params);
}
