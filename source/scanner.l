
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                            scanner.l                               |
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
  #include <ctype.h>
  #include "parser.h"
  #include <clay/array.h>
  #include <clay/list.h>
  #include <clay/macros.h>

  void clay_scanner_free();
  void clay_scanner_initialize();
  
  extern int is_in_a_list;

  char* trim(char*);


//extern YYLTYPE clay_yylloc;
//extern YYSTYPE clay_yylval;;



/*
  int yycolumn = 1;
  #define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; \
      yylloc.first_column = yycolumn; yylloc.last_column = yycolumn + yyleng - 1; \
      yycolumn += yyleng; \
      yylval.str = strdup(yytext);
  */
//#define YY_USER_ACTION clay_yylloc.first_line = clay_yylloc.last_line = clay_yylineno;

//%option outfile="scanner.c" header-file="scanner.h"
%}

%option yylineno
%option prefix="clay_yy"
%option noyywrap
%option noinput
%option nounput
SEPARATOR ([ \t]|\xc2\xa0)*

%%

{SEPARATOR}#(.|{SEPARATOR})*\n { return COMMENT; }

-?[0-9]+           {
                     clay_yylval.ival = atoi(clay_yytext);
                     return INTEGER;
                   }

true               {
                     clay_yylval.ival = 1;
                     return INTEGER;
                   }

false              {
                     clay_yylval.ival = 0;
                     return INTEGER;
                   }

[a-zA-Z_][a-zA-Z0-9_]*{SEPARATOR}\({SEPARATOR}\) {
                     // remove the ')'
                     clay_yytext[strlen(clay_yytext)-1] = '\0';
                     char *tmp = trim(clay_yytext);
                     // remove the '('
                     tmp[strlen(tmp)-1] = '\0';
                     clay_yylval.sval = strdup(trim(tmp));
                     return IDENT_FUNCTION_NO_ARGS;
                  }

[a-zA-Z_][a-zA-Z0-9_]*{SEPARATOR}\( {
                     // remove the '('
                     clay_yytext[strlen(clay_yytext)-1] = '\0';
                     clay_yylval.sval = strdup(trim(clay_yytext));
                     return IDENT_FUNCTION;
                   }

[a-zA-Z_][a-zA-Z0-9_]* {
                     clay_yylval.sval = strdup(clay_yytext);
                     return IDENT;
                   }

{SEPARATOR}        { ; }

\n                 { }
.                  { return clay_yytext[0]; }

%%


/**
 * clay_scanner_free function:
 * this function frees the memory allocated for the scanner. It frees
 * flex's buffer (it supposes there is only one buffer) since flex does
 * never free it itself.
 * WARNING: this is probably *not* portable...
 */
void clay_scanner_free() {
  clay_yy_delete_buffer(YY_CURRENT_BUFFER);
  free(yy_buffer_stack);
}

/**
 * clay_scanner_initialize function:
 * this function initialises the scanner global variables with default values.
 */
void clay_scanner_initialize() {
  clay_yy_flush_buffer(YY_CURRENT_BUFFER);
}

char *trim(char *str) {
  // taken here :
  // http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
