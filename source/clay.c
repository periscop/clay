
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


#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <osl/scop.h>
#include <clay/clay.h>
#include <clay/transformation.h>
#include <clay/array.h>
#include <parser.h>


int main(int argc, char * argv[]) {
  osl_scop_p scop;
  FILE *input;
  FILE *script = NULL;
  char *script_string;
  
  if (argc > 4 || argc == 1 || ((argc == 2) && !strcmp(argv[1], "-h")))  {
    CLAY_info("usage: clay ((-f SCRIPTFILE) | (STRING)) [SCOPFILE]");
    exit(0);
  }
  
  if (strcmp(argv[1], "-f") == 0) {
    script = fopen(argv[2], "r");
    if (script == NULL)
      CLAY_error("cannot open script file");
    if (argc == 3)
      input = stdin;
    else
      input = fopen(argv[3], "r");
  } else {
    script_string = argv[1];
    if (argc == 3)
      input = fopen(argv[2], "r");
    else
      input = stdin;
  }
  
  if (input == NULL)
    CLAY_error("cannot open scop file");
  
  srand(time(NULL));
  
  scop = osl_scop_read(input);
  fclose(input);
  
  if (script) {
    clay_parser_file(scop, script);
    fclose(script);
  } else {
    clay_parser_string(scop, script_string);
  }
  
  osl_scop_print(stdout, scop);
  osl_scop_free(scop);
  
  return 0;
}

