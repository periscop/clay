
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             parser.y                               |
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
  #include <assert.h>

  #include <osl/scop.h>
  #include <osl/statement.h>
  #include <osl/vector.h>

  #include <clay/clay.h>
  #include <clay/macros.h>
  #include <clay/array.h>
  #include <clay/list.h>
  #include <clay/transformation.h>
  #include <clay/stack.h>
  #include <clay/data.h>
  #include <clay/errors.h>
  #include <clay/functions.h>
  #include <clay/ident.h>
  
  // Yacc stuff.
  int          clay_yylex(void);
  int          clay_yyerror();
  struct yy_buffer_state *clay_yy_scan_string(char*);
  void         clay_yy_delete_buffer(struct yy_buffer_state *);
  extern FILE* clay_yyin;
  void         clay_yyrestart(FILE *);
  void         yy_scan_string(char*);
  extern int   clay_yylineno;
  
  // Scanner declarations
  extern void clay_scanner_free();
  extern void clay_scanner_initialize();
  
  // Current scop
  osl_scop_p clay_parser_scop;
  
  // Command line options
  clay_options_p clay_parser_options;

  // Arrays are allocated differently
  int is_in_a_list;

  // The script is a stack system execution
  clay_stack_t clay_parser_stack;

  // Current data wich will be pushed on the stack
  clay_data_t clay_parser_current;

  // For the moment only 26 variables are availables (a-z)
  clay_data_p clay_parser_vars[26] = {NULL};

  // Current nb args of a clay function
  int nb_args = 0;

  // Function call level, for now we can't do this f(g())
  // we have to use a temp variable
  int level = 0;

  // Save the index in the stack on which data are unref
  // it's use to get the data of variables
  // when the function is finish the data will not to be to freed
  clay_array_p clay_parser_unref;

  // Authorized functions in Clay (defined in functions.c)
  extern const clay_prototype_t functions[];

  // Error message.
  char *clay_error_message = NULL;

  // parser functions
  void clay_set_error_message(int);
  int clay_parser_exec_function(char *name);
%}

%name-prefix "clay_yy"

%union { int ival; } 
%union { char *sval; }

%token <ival> INTEGER
%token <sval> IDENT 
%token <sval> IDENT_FUNCTION 
%token <sval> IDENT_FUNCTION_NO_ARGS 
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
    IDENT '=' expr ';'
    {
      if (strlen($1) > 1 ||
          $1[0] < 'a' ||
          $1[0] > 'z') {
        clay_set_error_message(CLAY_ERROR_UNK_VAR);
        YYABORT;
      }

      int id = (int)$1[0] - 'a';
      if (clay_parser_vars[id] != NULL)
        clay_data_free(clay_parser_vars[id]);

      clay_data_p tmp = clay_stack_pop(&clay_parser_stack);

      // if the type of expr is a ref (a variable), we need to
      // dupplicate the data
      // otherwise, we just put an invalid type on the stack
      // to not free the allocated data

      if (tmp->type == REF_T) {
        if (tmp->data.obj == NULL) {
          clay_set_error_message(CLAY_ERROR_VAR_NULL);
          YYABORT;
        }

        clay_parser_vars[id] = clay_data_clone(tmp->data.obj);

      } else {
        clay_parser_vars[id] = clay_data_malloc(tmp->type);
        clay_parser_vars[id]->data = tmp->data;
        tmp->type = UNDEF_T; 
      }

      clay_stack_clear(&clay_parser_stack);
      free($1);

      level = 0;
    }

  | expr ';'
    {
      clay_stack_clear(&clay_parser_stack);
      level = 0;
    }
  ;

expr:
    IDENT_FUNCTION_NO_ARGS
    {
      if (level >= 1) {
        clay_set_error_message(CLAY_ERROR_CANT_CALL_SUBFUNC);
        YYABORT;
      }

      nb_args = 0;
      level++;

      if (strcmp($1, "break") == 0) {
        free($1);
        YYACCEPT;
      }

      int result = clay_parser_exec_function($1);
      if (result != 0)
        YYABORT;
      free($1);
    }
  |
    IDENT_FUNCTION 
    {
      if (level >= 1) {
        clay_set_error_message(CLAY_ERROR_CANT_CALL_SUBFUNC);
        YYABORT;
      }

      nb_args = 0;
      level++;
    }
    args ')'
    {
      int result = clay_parser_exec_function($1);
      if (result != 0)
        YYABORT;
      free($1);
    }

  |
    IDENT
    {
      if (strlen($1) > 1 ||
          $1[0] < 'a' ||
          $1[0] > 'z') {
        clay_set_error_message(CLAY_ERROR_UNK_VAR);
        YYABORT;
      }
      
      int id = (int)$1[0] - 'a';
      if (clay_parser_vars[id] == NULL) {
        clay_set_error_message(CLAY_ERROR_VAR_NULL);
        YYABORT;
      }

      clay_parser_current.type = REF_T;
      clay_parser_current.data.obj = clay_parser_vars[id];
      
      clay_stack_push(&clay_parser_stack, &clay_parser_current);
      free($1);
    }

  |
    INTEGER 
    {
      clay_parser_current.type = INTEGER_T;
      clay_parser_current.data.integer = $1;
      clay_stack_push(&clay_parser_stack, &clay_parser_current);
    }

  | // string (can contains only variable name)
    '"' IDENT '"'
    {
      clay_parser_current.type = STRING_T;
      clay_parser_current.data.obj = $2;
      clay_stack_push(&clay_parser_stack, &clay_parser_current);
    }

  | 
    '{' 
    {
      is_in_a_list = 1;
      clay_parser_current.type = LIST_T;
      clay_list_p l = clay_list_malloc();
      clay_parser_current.data.obj = l;
      clay_list_add(l, clay_array_malloc());
    }
    list_of_array
    '}'
    {
      is_in_a_list = 0;
      clay_stack_push(&clay_parser_stack, &clay_parser_current);
    }

  | 
    '[' 
    {
      clay_parser_current.type = ARRAY_T;
      clay_parser_current.data.obj = clay_array_malloc();
    }
    list_of_integer
    ']'
    {
      clay_stack_push(&clay_parser_stack, &clay_parser_current);
    }
  ;


args:
    args ',' expr { nb_args++; }
  | expr { nb_args++; }
  ;


list_of_array:
    list_of_array '|' 
    {
      clay_list_add(clay_parser_current.data.obj, clay_array_malloc());
    }
    list_of_integer
  |
    list_of_integer
  ;


list_of_integer:
    list_of_integer ',' INTEGER
    {
      if (is_in_a_list) {
        clay_list_p l = clay_parser_current.data.obj;
        clay_array_add(l->data[l->size-1], $3);
      } else {
        clay_array_p a = clay_parser_current.data.obj;
        clay_array_add(a, $3);
      }
    }
  |
    INTEGER
    {
      if (is_in_a_list) {
        clay_list_p l = clay_parser_current.data.obj;
        clay_array_add(l->data[l->size-1], $1);
      } else {
        clay_array_p a = clay_parser_current.data.obj;
        clay_array_add(a, $1);
      }
    }
  |
  ;

%%


/**
 * clay_yyerror function
 */
int clay_yyerror(void) {
  fprintf(stderr,"[Clay] Error: syntax on line %d, maybe you forgot a `;'\n",
          clay_yylineno);
  return 1;
}


/**
 * clay_parser_clear_vars function
 */
void clay_parser_free_vars() {
  int i;
  
  for (i = 0 ; i < 26 ; i++)
    if (clay_parser_vars[i])
      clay_data_free(clay_parser_vars[i]);
}


/**
 * clay_parser_file function:
 * \param[in] scop
 * \param[in] input    Input file of the script
 * \param[in] options
 */
int clay_parser_file(osl_scop_p scop, FILE *input, clay_options_p options) {
  clay_parser_scop = scop; // the scop is not NULL
  clay_parser_options = options;
  int ret = 0;
  
  clay_parser_unref = clay_array_malloc();
  clay_stack_init(&clay_parser_stack);
  is_in_a_list = 0;
  level = 0;
  nb_args = 0;
  clay_yyin = input;
  clay_yyrestart(clay_yyin);
  clay_scanner_initialize();
  int result = clay_yyparse();
  if (result != 0) {
    scop = NULL;
    ret = 1;
  }
  
  // Quit
  //clay_scanner_free();
  clay_parser_free_vars();
  clay_array_free(clay_parser_unref);
  return ret;
}


/**
 * clay_parser_string function:
 * \param[in] scop
 * \param[in] input    Input string 
 * \param[in] options
 */
int clay_parser_string(osl_scop_p scop, char *input, clay_options_p options) {
  clay_parser_scop = scop; // the scop is not NULL
  clay_parser_options = options;
  int ret = 0;

  clay_parser_unref = clay_array_malloc();
  clay_stack_init(&clay_parser_stack);
  is_in_a_list = 0;
  level = 0;
  nb_args = 0;
  struct yy_buffer_state *yy_buffer = clay_yy_scan_string(input);

  int result = clay_yyparse();
  clay_yy_delete_buffer(yy_buffer);
  if (result != 0) {
    scop = NULL;
    ret = 1;
  }

  // Quit
  //clay_scanner_free();
  clay_parser_free_vars();
  clay_array_free(clay_parser_unref);
  return ret;
}


/**
 * clay_parser_exec_function:
 * \param[in] name        function name
 */
int clay_parser_exec_function(char *name) {
  int i, j, k;

  int top = clay_parser_stack.sp;
  clay_data_p tmp;


  
  // search the function name
  i = 0;
  while (i < CLAY_FUNCTIONS_TOTAL) {
    if (strcmp(functions[i].name, name) == 0)
      break;
    i++;
  }
  
  // unknown or alias
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
              clay_yylineno, name);
      return CLAY_ERROR_UNKNOWN_FUNCTION;
    }
  }
  
  // Different number of parameters
  
  if (nb_args != functions[i].argc) {
      fprintf(stderr, 
        "[Clay] Error: line %d, `%s' takes %d arguments\n"
        "[Clay] prototype is: %s\n", 
        clay_yylineno, name, functions[i].argc, functions[i].string);
      return CLAY_ERROR_NB_ARGS;
  }
  
  // check types
  j = 0;
  k = top - functions[i].argc + 1;
  while (j < functions[i].argc) {
    
    // unref the data on the stack
    // at the end of the function will be UNDEF_T on the stack
    // to not free the referenced data
    if (functions[i].args[j] != MULTI_T) {
      if (clay_parser_stack.stack[k].type == REF_T) {
        tmp = (clay_data_p) clay_parser_stack.stack[k].data.obj;
        clay_parser_stack.stack[k].type = tmp->type;
        clay_parser_stack.stack[k].data = tmp->data;
        clay_array_add(clay_parser_unref, k);
      }

      if (clay_parser_stack.stack[k].type != functions[i].args[j]) {
        fprintf(stderr, 
          "[Clay] Error: line %d, in function `%s' invalid type on argument %d\n"
          "[Clay] prototype is: %s\n", 
          clay_yylineno, name, j+1, functions[i].string);
        return CLAY_ERROR_INVALID_TYPE;
      }
    }
    k++;
    j++;
  }
  
  // exec function  
  int status_result = 0;
  clay_betatree_p tree;
  int integer;
  void *data;
  clay_data_t result;
  result.type = UNDEF_T;

  switch (i) {
    case CLAY_FUNCTION_SPLIT:
      status_result = clay_split(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_GRAIN:
      status_result = clay_grain(
          clay_parser_scop,
          clay_parser_stack.stack[top-2].data.obj,
          clay_parser_stack.stack[top-1].data.integer,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_DENSIFY:
      status_result = clay_densify(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_RESHAPE:
      status_result = clay_reshape(
          clay_parser_scop,
          clay_parser_stack.stack[top-3].data.obj,
          clay_parser_stack.stack[top-2].data.integer, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_REORDER:
      status_result = clay_reorder(
          clay_parser_scop, 
          clay_parser_stack.stack[top-1].data.obj, 
          clay_parser_stack.stack[top].data.obj, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_INTERCHANGE:
      status_result = clay_interchange(
          clay_parser_scop, 
          clay_parser_stack.stack[top-3].data.obj, 
          clay_parser_stack.stack[top-2].data.integer, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_REVERSE:
      status_result = clay_reverse(
          clay_parser_scop, 
          clay_parser_stack.stack[top-1].data.obj, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_FUSE:
      status_result = clay_fuse(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_EMBED:
      status_result = clay_embed(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_UNEMBED:
      status_result = clay_unembed(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_SIEVE:
      status_result = clay_sieve(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_SKEW:
      status_result = clay_skew(
        clay_parser_scop,
        clay_parser_stack.stack[top-3].data.obj,
        clay_parser_stack.stack[top-2].data.integer,
        clay_parser_stack.stack[top-1].data.integer,
        clay_parser_stack.stack[top].data.integer,
        clay_parser_options);
      break;

    case CLAY_FUNCTION_ISS:
      status_result = clay_iss(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj, 
          clay_parser_stack.stack[top].data.obj, 
          NULL,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_COLLAPSE:
      status_result = clay_collapse(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_STRIPMINE:
      status_result = clay_stripmine(
          clay_parser_scop,
          clay_parser_stack.stack[top-2].data.obj, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_LINEARIZE:
      status_result = clay_linearize(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_UNROLL:
      status_result = clay_unroll(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_TILE:
      status_result = clay_tile(
          clay_parser_scop,
          clay_parser_stack.stack[top-3].data.obj, 
          clay_parser_stack.stack[top-2].data.integer, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_SHIFT:
      status_result = clay_shift(
          clay_parser_scop,
          clay_parser_stack.stack[top-3].data.obj,
          clay_parser_stack.stack[top-2].data.integer,
          clay_parser_stack.stack[top-1].data.obj,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_PEEL:
      status_result = clay_peel(
        clay_parser_scop,
        clay_parser_stack.stack[top-1].data.obj, 
        clay_parser_stack.stack[top].data.obj, 
        clay_parser_options);
      break;

    case CLAY_FUNCTION_CONTEXT:
      status_result = clay_context(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_DIMREORDER:
      status_result = clay_dimreorder(
        clay_parser_scop,
        clay_parser_stack.stack[top-2].data.obj, 
        clay_parser_stack.stack[top-1].data.integer, 
        clay_parser_stack.stack[top].data.obj, 
        clay_parser_options);
      break;

    case CLAY_FUNCTION_DIMPRIVATIZE:
      status_result = clay_dimprivatize(
          clay_parser_scop,
          clay_parser_stack.stack[top-2].data.obj, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_DIMCONTRACT:
      status_result = clay_dimcontract(
          clay_parser_scop,
          clay_parser_stack.stack[top-2].data.obj, 
          clay_parser_stack.stack[top-1].data.integer, 
          clay_parser_stack.stack[top].data.integer, 
          clay_parser_options);
      break;

    case CLAY_FUNCTION_ADDARRAY:
      result.type = INTEGER_T;
      status_result = clay_add_array(
          clay_parser_scop,
          clay_parser_stack.stack[top].data.obj,
          &result.data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_GETBETALOOP:
      tree = clay_betatree_create(clay_parser_scop);
      integer = clay_parser_stack.stack[top].data.integer;
      result.type = ARRAY_T;
      result.data.obj = clay_ident_find_loop(tree, integer);

      if (!result.data.obj) {
        clay_set_error_message(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
        return 1;
      }

      clay_betatree_free(tree);
      break;

    case CLAY_FUNCTION_GETBETASTMT:
      integer = clay_parser_stack.stack[top].data.integer;
      result.type = ARRAY_T;
      result.data.obj = clay_ident_find_stmt(clay_parser_scop, integer);

      if (!result.data.obj) {
        clay_set_error_message(CLAY_ERROR_IDENT_STMT_NOT_FOUND);
        return 1;
      }
      break;

    case CLAY_FUNCTION_GETBETALOOPBYNAME:
      data = clay_parser_stack.stack[top].data.obj;
      result.type = ARRAY_T;
      result.data.obj = clay_ident_find_iterator(clay_parser_scop,  
                                                 (char*) data);
      if (!result.data.obj) {
        clay_set_error_message(CLAY_ERROR_IDENT_NAME_NOT_FOUND);
        return 1;
      }
      break;

    case CLAY_FUNCTION_GETARRAYID:
      data = clay_parser_stack.stack[top].data.obj;
      result.type = INTEGER_T;
      status_result = clay_get_array_id(
          clay_parser_scop, 
          (char*) data,
          &result.data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_PRINT:
      clay_data_print(stderr, &clay_parser_stack.stack[top]);
      break;

    case CLAY_FUNCTION_REPLACEARRAY:
      status_result = clay_replace_array(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.integer,
          clay_parser_stack.stack[top].data.integer,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_DATACOPY:
      status_result = clay_datacopy(
          clay_parser_scop,
          clay_parser_stack.stack[top-4].data.integer,
          clay_parser_stack.stack[top-3].data.integer,
          clay_parser_stack.stack[top-2].data.obj,
          clay_parser_stack.stack[top-1].data.integer,
          clay_parser_stack.stack[top].data.obj,
          clay_parser_options);
      break;

    case CLAY_FUNCTION_BLOCK:
      status_result = clay_block(
          clay_parser_scop,
          clay_parser_stack.stack[top-1].data.obj,
          clay_parser_stack.stack[top].data.obj,
          clay_parser_options);
      break;

    default:
      fprintf(stderr, "[Clay] Error: can't call the function %s (%s).\n", 
              functions[i].name, __func__);
      return 1;
      break;
  }

  // don't free data which are unref
  for (i = 0 ; i < clay_parser_unref->size ; i++)
    clay_parser_stack.stack[clay_parser_unref->data[i]].type = UNDEF_T;

  clay_parser_unref->size = 0; // small optimization, don't remalloc each time

  // clear args on the stack
  for (i = 0 ; i < nb_args ; i++)
    clay_data_clear(clay_stack_pop(&clay_parser_stack));

  // push result
  if (result.type != UNDEF_T)
    clay_stack_push(&clay_parser_stack, &result);

  // check errors
  if (status_result != CLAY_SUCCESS) {
    clay_set_error_message(status_result);
    return 1;
  }
  return 0;
}

void clay_set_error_message(int status_result) {
  char *str = clay_error_message_text(status_result);
  unsigned long size = strlen(str) + strlen("Error: line :") + 32;
  char *message;
  if (status_result != CLAY_SUCCESS) {
    message = (char *) malloc(size);
    snprintf(message, size, "Error: line %d: %s", clay_yylineno, str);
  } else {
    message = NULL;
  }
  if (clay_error_message != NULL)
    free(clay_error_message);
  clay_error_message = message;

  free(str);
}

char *clay_get_error_message() {
  return clay_error_message;
}


