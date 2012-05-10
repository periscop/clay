
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
  #include <parser.h>
  #include <clay/array.h>
  #include <clay/prototype_function.h>

  void        clay_scanner_free();
  void        clay_scanner_initialize();
  
  int         scanner_line;
  
  extern clay_prototype_function_p  params;

%}


%option noyywrap
SEPARATOR [ \t]*


%%

{SEPARATOR}#(.|{SEPARATOR})*
                    {
                      return COMMENT;
                    }

-?[0-9]+              {
                      yylval.ival = atoi(yytext);
                      return INTEGER;
                    }

[a-z]+              {
                      yylval.sval = strdup(yytext);
                      return FUNCNAME;
                    }

{SEPARATOR}         { ; }
\[                  {
                      //fprintf(stderr, "NEW ARRAY\n");
                      clay_array_p tmp = clay_array_malloc();
                      clay_prototype_function_args_add(params, tmp, ARRAY_T);
                      return '[';
                    }
.                   { return yytext[0]; }
\n                  { scanner_line++; return '\n'; }

%%


/**
 * clay_scanner_free function:
 * this function frees the memory allocated for the scanner. It frees
 * flex's buffer (it supposes there is only one buffer) since flex does
 * never free it itself.
 * WARNING: this is probably *not* portable...
 */
void clay_scanner_free() {
  yy_delete_buffer(YY_CURRENT_BUFFER);
  free(yy_buffer_stack);
}

/**
 * clay_scanner_initialize function:
 * this function initialises the scanner global variables with default values.
 */
void clay_scanner_initialize() {
  yy_flush_buffer(YY_CURRENT_BUFFER);
  scanner_line = 0;
}


