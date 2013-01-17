
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             clay.c                                 |
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
#include <clay/beta.h>
#include <clay/betatree.h>
#include <clay/options.h>
#include <clay/array.h>
#include <clay/ident.h>
#include <parser.h>

#ifdef CLAN_LINKED
#include <clan/macros.h>
#include <clan/options.h>
#include <clan/scop.h>
#endif

#ifdef CLOOG_LINKED
#define CLOOG_INT_LONG
#include <cloog/cloog.h>
#endif

#ifdef CANDL_LINKED
// Cloog works only with GMP, so piplib must to be compiled with GMP
#include <candl/candl.h>
#include <candl/scop.h>
#include <candl/dependence.h>
#include <candl/violation.h>
#include <osl/extensions/dependence.h>
#endif


void      clay_parser_string(osl_scop_p, char*, clay_options_p);
void      clay_parser_file(osl_scop_p, FILE*, clay_options_p);


int main(int argc, char * argv[]) {
  osl_scop_p scop = NULL;
  osl_generic_p x, last;
  osl_clay_p clay_tag;
  clay_options_p options;
  
  // Read command line parameters
  options = clay_options_read(argc, argv);
  if (options->print_infos) {
    clay_options_free(options);
    exit(0);
  }

  // Open the scop file
  #ifdef CLAN_LINKED
  if (options->compile) {
    clan_options_p clan_opt = clan_options_malloc();
		clan_opt->precision = OSL_PRECISION_MP;
		clan_opt->name = options->input_name;
    scop = clan_scop_extract(options->input, clan_opt);
    //clan_options_free(clan_opt); // bug, the name is also freed
    free(clan_opt);
    fclose(options->input);
  }
  else
  #endif
  {
    scop = osl_scop_read(options->input);
    if (options->input != stdin)
      fclose(options->input);
  }

  if (options->normalize) {
    clay_beta_normalize(scop);
  }

	#if defined(CANDL_LINKED)
	osl_scop_p orig_scop = NULL;
	if (!options->nocandl && scop != NULL) {
		orig_scop = osl_scop_clone(scop);
		candl_scop_usr_init(orig_scop);
	}
	#endif
  
  // Execute the script ...
  // do nothing if the scop is NULL
  if (scop != NULL) {
    // Read the script file
    if (!options->from_tag) {
      clay_parser_file(scop, options->script, options);
      fclose(options->script);
    
    // Read the script from the extension clay
    } else {
      // equivalent to osl_generic_lookup, but we need the last extension
      // to remove the clay extension in the list
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
        last->next = x->next;
        x->next = NULL;
        osl_generic_free(x);
      }
    }
  }

	#ifdef CANDL_LINKED
	int is_violated = 0;
	// Check dependencies
	if (!options->nocandl && scop != NULL) {
		candl_options_p candl_opt = candl_options_malloc();
    if (options->candl_fullcheck)
      candl_opt->fullcheck = 1;
		osl_dependence_p dep = candl_dependence(orig_scop, candl_opt);
		candl_violation_p violation = candl_violation(orig_scop, dep, scop, candl_opt);

		is_violated = (violation != NULL);
		if (is_violated) {
      candl_violation_pprint(stdout, violation);
      if (options->candl_structure)
			  candl_violation_dump(stdout, violation);
    }

		candl_options_free(candl_opt);
		candl_violation_free(violation);
 		osl_dependence_free(dep);
		candl_scop_usr_cleanup(orig_scop);
		osl_scop_free(orig_scop);
	}

	if (!is_violated) // print the scop or the .c file by cloog
	#endif
	
	{
    #ifdef CLOOG_LINKED
		if (options->compile && scop != NULL) {
			CloogState *state = cloog_state_malloc();
			CloogOptions *cloogoptions = cloog_options_malloc(state);
			cloogoptions->openscop = 1;
			CloogInput *clooginput = cloog_input_from_osl_scop(cloogoptions->state, 
                                                                           scop);
			cloog_options_copy_from_osl_scop(scop, cloogoptions);
			CloogProgram *program = cloog_program_alloc(clooginput->context, 
					                            clooginput->ud, cloogoptions);
			free(clooginput);
			cloog_program_generate(program, cloogoptions);
			
			cloog_program_pprint(stdout, program, cloogoptions);
			cloog_program_free(program);
			cloogoptions->scop = NULL; // don't free the scop
			cloog_options_free(cloogoptions);
			cloog_state_free(state);
		}
		else
		#endif
		{
			osl_scop_print(stdout, scop);
		}
	}

  osl_scop_free(scop);
  clay_options_free(options);

  return 0;
}

