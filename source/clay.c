
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
#include <osl/macros.h>
#include <osl/generic.h>
#include <osl/interface.h>
#include <osl/extensions/clay.h>
#include <clay/clay.h>
#include <clay/transformation.h>
#include <clay/options.h>
#include <clay/array.h>
#include <parser.h>


int main(int argc, char * argv[]) {
  osl_scop_p scop;
  osl_generic_p x, last;
  osl_clay_p clay_tag;
  clay_options_p options;
  FILE *input = NULL;
  FILE *script = NULL;
  
  // Random number are used to generate new variables name in the function
  // clay_stripmine
  srand(time(NULL));
  
  // Read command line parameters
  options = clay_options_read(argc, argv);
  if (options->print_infos) {
    clay_options_free(options);
    exit(0);
  }
  
  // Open the scop file
  if (options->input_scop) {
    input = fopen(options->scop_name, "r");
    if (input == NULL)
      CLAY_error("cannot open the scop file");
  } else {
    input = stdin;
  }

  scop = osl_scop_read(input);
  
  // Read the script file
  if (options->input_script) {
    script = fopen(options->script_name, "r");
    if (script == NULL)
      CLAY_error("cannot open the script file");
    clay_parser_file(scop, script, options);
    fclose(script);
  
  // Read the script from the extension clay
  } else {
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
      clay_parser_string(scop, clay_tag->script, options);

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
  }

  osl_scop_print(stdout, scop);
  osl_scop_free(scop);
  clay_options_free(options);
  fclose(input);
  
  return 0;
}

