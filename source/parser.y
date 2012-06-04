
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
  #include <clay/errors.h>
  #include <clay/functions.h>
  #include <clay/ident.h>
  
  // Yacc stuff.
  int                 yylex(void);
  void                yy_scan_string(char*);
  
  // Scanner declarations
  void                clay_scanner_free();
  void                clay_scanner_initialize();
  extern int          clay_scanner_line;
  
  
  // Arguments list of the current scanned function
  clay_prototype_function_p  clay_params;
  
  // Current scop
  osl_scop_p          clay_parser_scop;
  
  // If we need to search a beta
  clay_array_p        clay_parser_beta;
  
  // Command line options
  clay_options_p      clay_parser_options;
  
  // Other
  int                 yyerror();
  extern              FILE* yyin;
  
  // parser functions
  void                clay_parser_exec_function(char *name);
  void                clay_parser_string(osl_scop_p, char*, clay_options_p);
  void                clay_parser_file(osl_scop_p, FILE*, clay_options_p);
  void                clay_parser_print_error(int);
  
  // Authorized functions in Clay
  extern const clay_prototype_function_t functions[];
  
%}

%union { int ival; } 
%union { char *sval; }

%token <ival> INTEGER
%token <sval> IDENT_NAME 
%token <ival> IDENT_STMT
%token <ival> IDENT_LOOP
%token COMMENT

%start start

%%

start:
	|
	  line start
	;

line:
    COMMENT
  |
    IDENT_NAME '(' args ')' ';'
    {
      clay_parser_exec_function($1);
      free($1);
    }
  ;

args:
  | // integer
    args ',' INTEGER 
    {
      int *tmp;
      CLAY_malloc(tmp, int*, sizeof(int));
      *tmp = $3;
      clay_prototype_function_args_add(clay_params, tmp, INTEGER_T);
    }
  |
    INTEGER
    {
      int *tmp;
      CLAY_malloc(tmp, int*, sizeof(int));
      *tmp = $1;
      clay_prototype_function_args_add(clay_params, tmp, INTEGER_T);
    }
  | // Snum
    IDENT_STMT
    {
      clay_parser_beta = clay_ident_find_stmt(clay_parser_scop, $1);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_STMT_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
    }
  |
    args ',' IDENT_STMT
    {
      clay_parser_beta = clay_ident_find_stmt(clay_parser_scop, $3);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_STMT_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
    }
  | // iterator name
    IDENT_NAME
    {
      clay_parser_beta = clay_ident_find_iterator(clay_parser_scop, $1);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
      free($1);
    }
  |
    args ',' IDENT_NAME
    {
      clay_parser_beta = clay_ident_find_iterator(clay_parser_scop, $3);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
      free($3);
    }
  | // Lnum
    IDENT_LOOP
    {
      // create the tree
      // we need to recompute it because after each function the tree changes
      clay_betatree_p tree = clay_betatree_create(clay_parser_scop);
      clay_parser_beta = clay_ident_find_loop(tree, $1);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
      clay_betatree_free(tree);
    }
  |
    args ',' IDENT_LOOP
    {
      // create the tree
      // we need to recompute it because after each function the tree changes
      clay_betatree_p tree = clay_betatree_create(clay_parser_scop);
      clay_parser_beta = clay_ident_find_loop(tree, $3);
      if (!clay_parser_beta) {
        clay_parser_print_error(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
      }
      clay_prototype_function_args_add(clay_params, clay_parser_beta, ARRAY_T);
      clay_betatree_free(tree);
    }
  | // default beta vector
    args ',' array
  |
    array
  ;

// the array is allocated in the scanner.l
array:
    '[' list ']'
  ;

// The array is created in the lex file
list:
    list ',' INTEGER
    {
      clay_array_add((clay_array_p) clay_params->args[clay_params->argc-1], $3);
    }
  |
    INTEGER
    {
      clay_array_add((clay_array_p) clay_params->args[clay_params->argc-1], $1);
    }
  |
  ;

%%


/**
 * yyerror function
 */
int yyerror(void) {
  fprintf(stderr,"[Clay] Error: syntax on line %d, maybe you forgot a `;'\n",
          clay_scanner_line-1);
  exit(1);
}


/**
 * clay_parser_file function:
 * \param[in] scop
 * \param[in] input    Input file of the script
 * \param[in] options
 */
void clay_parser_file(osl_scop_p scop, FILE *input, clay_options_p options) {
  clay_parser_scop = scop; // the scop is not NULL
  clay_parser_options = options;
  
  // List of parameters of the current scanned function
  clay_params = clay_prototype_function_malloc();
  
  yyin = input;
  clay_scanner_initialize();
  yyparse();
  
  // Quit
  clay_scanner_free();
  clay_prototype_function_free(clay_params);
}


/**
 * clay_parser_string function:
 * \param[in] scop
 * \param[in] input    Input string 
 * \param[in] options
 */
void clay_parser_string(osl_scop_p scop, char *input, clay_options_p options) {
  clay_parser_scop = scop; // the scop is not NULL
  clay_parser_options = options;
  
  // List of parameters of the current scanned function
  clay_params = clay_prototype_function_malloc();
  
  yy_scan_string(input);
  clay_scanner_line = 1;
  yyparse();
  
  // Quit
  clay_scanner_free();
  clay_prototype_function_free(clay_params);
}


/**
 * clay_parser_exec_function:
 * \param[in] name        function name
 */
void clay_parser_exec_function(char *name) {
  int i, j, k;
  int val;
  clay_array_p tmp;
  
  // search the function name
  i = 0;
  while (i < CLAY_FUNCTIONS_TOTAL) {
    if (strcmp(functions[i].name, name) == 0)
      break;
    i++;
  }
  
  if (i == CLAY_FUNCTIONS_TOTAL) {
    // check if an alias exists
    if (strcmp(name, "fission") == 0) {
      i = CLAY_FUNCTION_SPLIT;
    } else if (strcmp(name, "distribute") == 0) {
      i = CLAY_FUNCTION_SPLIT;
    } else if (strcmp(name, "merge") == 0) {
      i = CLAY_FUNCTION_FUSE;
    } else { // Undefined function
      fprintf(stderr, "[Clay] Error: line %d, unknown function `%s'\n", 
              clay_scanner_line, name);
      exit(CLAY_ERROR_UNKNOWN_FUNCTION);
    }
  }
  
  // Different number of parameters
  if (clay_params->argc != functions[i].argc) {
      fprintf(stderr, 
        "[Clay] Error: line %d, in `%s' takes %d arguments\n[Clay] \
prototype is: %s\n", 
        clay_scanner_line, name, functions[i].argc, functions[i].prototype);
      exit(CLAY_ERROR_NB_ARGS);
  }
  
  // check types
  j = 0;
  while (j < functions[i].argc) {
    
    if (functions[i].type[j] == ARRAY_OR_INTEGER_T) {
      
      // convert INTEGER_T to ARRAY_T
      switch(clay_params->type[j]) {
        case INTEGER_T:
          tmp = clay_array_malloc();
          for (k = 0 ; k < clay_parser_scop->context->nb_parameters ; k++) {
            clay_array_add(tmp, 0);
          }
          
          val = *((int*)clay_params->args[j]);
          clay_array_add(tmp, val);
          
          free(clay_params->args[j]);
          clay_params->args[j] = tmp;
          
          clay_params->type[j] = ARRAY_T;
          break;
      }
      
    } else if (clay_params->type[j] != functions[i].type[j]) {
      break;
    }
    
    j++;
  }
  
  // Invalid type
  if (j != functions[i].argc) {
      fprintf(stderr, 
        "[Clay] Error: line %d, in function `%s' invalid type on argument \
%d\n[Clay] prototype is: %s\n", 
        clay_scanner_line, name, j+1, functions[i].prototype);
      exit(CLAY_ERROR_INVALID_TYPE);
  }

  // exec function  
  int status_result = 0;
  switch (i) {
    case CLAY_FUNCTION_SPLIT:
      status_result = clay_split(clay_parser_scop, 
                                 clay_params->args[0], 
                                 *((int*)clay_params->args[1]),
                                 clay_parser_options);
      break;
    case CLAY_FUNCTION_REORDER:
      status_result = clay_reorder(clay_parser_scop, 
                                   clay_params->args[0], 
                                   clay_params->args[1],
                                   clay_parser_options);
      break;
    case CLAY_FUNCTION_INTERCHANGE:
      status_result = clay_interchange(clay_parser_scop, 
                                       clay_params->args[0], 
                                       *((int*)clay_params->args[1]),
                                       *((int*)clay_params->args[2]),
                                       clay_parser_options);
      break;
    case CLAY_FUNCTION_REVERSE:
      status_result = clay_reverse(clay_parser_scop, 
                                   clay_params->args[0],
                                   *((int*)clay_params->args[1]),
                                   clay_parser_options);
      break;
    case CLAY_FUNCTION_FUSE:
      status_result = clay_fuse(clay_parser_scop,
                                clay_params->args[0],
                                clay_parser_options);
      break;
    case CLAY_FUNCTION_SKEW:
      status_result = clay_skew(clay_parser_scop,
                                clay_params->args[0], 
                                *((int*)clay_params->args[1]),
                                *((int*)clay_params->args[2]),
                                clay_parser_options);
      break;
    case CLAY_FUNCTION_ISS:
      status_result = clay_iss(clay_parser_scop,
                               clay_params->args[0], 
                               clay_params->args[1],
                               clay_parser_options);
      break;
    case CLAY_FUNCTION_STRIPMINE:
      status_result = clay_stripmine(clay_parser_scop,
                                     clay_params->args[0], 
                                     *((int*)clay_params->args[1]),
                                     *((int*)clay_params->args[2]),
                                     *((int*)clay_params->args[3]),
                                     clay_parser_options);
      break;
    case CLAY_FUNCTION_UNROLL:
      status_result = clay_unroll(clay_parser_scop,
                                  clay_params->args[0], 
                                  *((int*)clay_params->args[1]),
                                  1,
                                  clay_parser_options);
      break;
    case CLAY_FUNCTION_UNROLL_NOEPILOG:
      status_result = clay_unroll(clay_parser_scop,
                                  clay_params->args[0], 
                                  *((int*)clay_params->args[1]),
                                  0,
                                  clay_parser_options);
      break;
    case CLAY_FUNCTION_TILE:
      status_result = clay_tile(clay_parser_scop,
                                clay_params->args[0], 
                                *((int*)clay_params->args[1]),
                                *((int*)clay_params->args[2]),
                                *((int*)clay_params->args[3]),
                                *((int*)clay_params->args[4]),
                                clay_parser_options);
      break;
    case CLAY_FUNCTION_SHIFT:
      status_result = clay_shift(clay_parser_scop,
                                 clay_params->args[0], 
                                 *((int*)clay_params->args[1]),
                                 clay_params->args[2], 
                                 clay_parser_options);
      break;
    case CLAY_FUNCTION_PEEL_FIRST:
      status_result = clay_peel(clay_parser_scop,
                                clay_params->args[0], 
                                clay_params->args[1],
                                1,
                                clay_parser_options);
      break;
    case CLAY_FUNCTION_PEEL_LAST:
      status_result = clay_peel(clay_parser_scop,
                                clay_params->args[0], 
                                clay_params->args[1],
                                0,
                                clay_parser_options);
      break;
    case CLAY_FUNCTION_CONTEXT:
      status_result = clay_context(clay_parser_scop,
                                   clay_params->args[0], 
                                   clay_parser_options);
      break;
  }
  
  // check errors
  if (status_result != CLAY_SUCCESS) {
    clay_parser_print_error(status_result);
  }

  clay_prototype_function_args_clear(clay_params);
}


/**
 * clay_parser_print_error function:
 * \param[in] status
 * \return
 */
void clay_parser_print_error(int status_result) {
  switch (status_result) {
    case CLAY_ERROR_BETA_NOT_FOUND:
      fprintf(stderr,"[Clay] Error: line %d: the beta vector was not found\n",
              clay_scanner_line);
      exit(CLAY_ERROR_BETA_NOT_FOUND);
      break;
    case CLAY_ERROR_NOT_BETA_LOOP:
      fprintf(stderr,"[Clay] Error: line %d: the beta is not a loop\n",
              clay_scanner_line);
      exit(CLAY_ERROR_NOT_BETA_LOOP);
      break;
    case CLAY_ERROR_NOT_BETA_STMT:
      fprintf(stderr,"[Clay] Error: line %d, the beta is not a statement\n", 
              clay_scanner_line);
      exit(3);
      break;
    case CLAY_ERROR_REORDER_ARRAY_TOO_SMALL:
      fprintf(stderr,"[Clay] Error: line %d, the order array is too small\n", 
              clay_scanner_line);
      exit(CLAY_ERROR_REORDER_ARRAY_TOO_SMALL);
      break;
    case CLAY_ERROR_DEPTH_OVERFLOW:
      fprintf(stderr,"[Clay] Error: line %d, depth overflow\n",
              clay_scanner_line);
      exit(CLAY_ERROR_DEPTH_OVERFLOW);
      break;
    case CLAY_ERROR_WRONG_COEFF:
      fprintf(stderr,"[Clay] Error: line %d, wrong coefficient\n",
              clay_scanner_line);
      exit(CLAY_ERROR_WRONG_COEFF);
      break;
    case CLAY_ERROR_BETA_EMPTY:
      fprintf(stderr,"[Clay] Error: line %d, the beta vector is empty\n",
              clay_scanner_line);
      exit(CLAY_ERROR_BETA_EMPTY);
      break;
    case CLAY_ERROR_BETA_NOT_IN_A_LOOP:
      fprintf(stderr,"[Clay] Error: line %d, the beta need to be in a loop\n",
              clay_scanner_line);
      exit(CLAY_ERROR_BETA_EMPTY);
      break;
    case CLAY_ERROR_WRONG_BLOCK_SIZE:
      fprintf(stderr,"[Clay] Error: line %d, block value is incorrect\n",
              clay_scanner_line);
      exit(CLAY_ERROR_WRONG_BLOCK_SIZE);
      break;
    case CLAY_ERROR_WRONG_FACTOR:
      fprintf(stderr,"[Clay] Error: line %d, wrong factor\n",
              clay_scanner_line);
      exit(CLAY_ERROR_WRONG_FACTOR);
      break;
    case CLAY_ERROR_DEPTH_OUTER:
      fprintf(stderr,"[Clay] Error: line %d, the depth is not 'outer'\n",
              clay_scanner_line);
      exit(CLAY_ERROR_DEPTH_OUTER);
      break;
    case CLAY_ERROR_VECTOR_EMPTY:
      fprintf(stderr,"[Clay] Error: line %d, the vector is empty\n",
              clay_scanner_line);
      exit(CLAY_ERROR_VECTOR_EMPTY);
      break;
    case CLAY_ERROR_IDENT_NAME_NOT_FOUND:
      fprintf(stderr,"[Clay] Error: line %d, the iterator name was not found\n",
              clay_scanner_line);
      exit(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
      break;
    case CLAY_ERROR_IDENT_STMT_NOT_FOUND:
      fprintf(stderr,"[Clay] Error: line %d, the statement was not found\n",
              clay_scanner_line);
      exit(CLAY_ERROR_IDENT_STMT_NOT_FOUND);
      break;
    case CLAY_ERROR_INEQU:
      fprintf(stderr,"[Clay] Error: line %d, the inequality or equality seems \
to be wrong\n",
              clay_scanner_line);
      exit(CLAY_ERROR_INEQU);
      break;
    case CLAY_ERROR_VECTOR:
      fprintf(stderr,"[Clay] Error: line %d, the vector seems to be wrong\n",
              clay_scanner_line);
      exit(CLAY_ERROR_VECTOR);
      break;
  }
}
