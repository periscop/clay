
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
#include <osl/extensions/clay.h>
#include <osl/generic.h>
#include <osl/macros.h>
#include <clay/clay.h>
#include <clay/transformation.h>
#include <clay/array.h>
#include <parser.h>


int main(int argc, char * argv[]) {
  osl_scop_p scop;
  osl_clay_p clay_tag;
  osl_generic_p x;
  osl_generic_p last;
  FILE *input;
  FILE *script = NULL;
  int read_clay_tag = 0;
  
  if (argc > 4 || ((argc == 2) && !strcmp(argv[1], "-h")))  {
    CLAY_info("usage: clay [--script SCRIPTFILE] [SCOPFILE]");
    exit(0);
  }
  
  if (argc == 1) {
    read_clay_tag = 1;
    input = stdin;
  } else {
    if (strcmp(argv[1], "--script") == 0) {
      script = fopen(argv[2], "r");
      if (script == NULL)
        CLAY_error("cannot open script file");
      if (argc == 3)
        input = stdin;
      else
        input = fopen(argv[3], "r");
    } else {
      read_clay_tag = 1;
      input = fopen(argv[1], "r");
    }
  }
  
  if (input == NULL)
    CLAY_error("cannot open scop file");
  
  srand(time(NULL));
  
  scop = osl_scop_read(input);
  
  if (read_clay_tag) {
    // equivalent to osl_generic_lookup, but we need the last extension
    x = scop->extension;
    last = NULL;
    while (x != NULL) {
      if (osl_generic_has_URI(x, OSL_URI_CLAY))
        break;
      last = x;
      x = x->next;
    }
    
    if (x != NULL) {
      // parse the clay string
      clay_tag = x->data;
      clay_parser_string(scop, clay_tag->script);

      // remove the extension clay
      // we don't use osl_generic_free, because we need to remove only one
      // extension
      last->next = x->next;
      if (x->interface != NULL) {
        x->interface->free(x->data);
        osl_interface_free(x->interface);
      } else if (x->data != NULL) {
        OSL_warning("unregistered interface, memory leaks are possible");
        free(x->data);
      }
      free(x);
    }
  } else {
    clay_parser_file(scop, script);
    fclose(script);
  }
  
  fclose(input);
  osl_scop_print(stdout, scop);
  osl_scop_free(scop);
  
  return 0;
}

