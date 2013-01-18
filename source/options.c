
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                            options.c                               |
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
#include <stdio.h>
#include <string.h>
#include <clay/macros.h>
#include <clay/options.h>
#include <clay/prototype_function.h>
#include <clay/functions.h>

extern const clay_prototype_function_t functions[];


/**
 * clay_options_free function:
 * This function frees the allocated memory for a clay_options_t structure.
 * \param options Option structure to be freed.
 */
void clay_options_free(clay_options_p options) {
  free(options);
}


/**
 * clay_options_list_functions function:
 */
void clay_options_list_functions() {
  int i;
  printf(
  "Available functions:\n\n"
  "  ident type:\n"
  "    array:   [n1, n2, ...]\n"
  "             Refers to a beta vector, loop or statement\n"
  "    Sn:      where n >= 0\n"
  "             Refers to the `n'th statement\n"
  "    Ln:      where n >= 0\n"
  "             Refers to the `n'th loop\n"
  "             Special case : L0 <=> []\n"
  "    string:  Original iterator name of a loop\n\n");
  
  for (i = 0 ; i < CLAY_FUNCTIONS_TOTAL ; i++) {
    printf("  %s\n", functions[i].prototype);
  }
}


/**
 * clay_options_help function:
 * This function displays the quick help when the user set the option -help
 * while calling clay. Prints are cut to respect the 509 characters
 * limitation of the ISO C 89 compilers.
 */
void clay_options_help() {
  printf(
  "Usage: clay [ options | file ] ...\n");
  printf(
  "\nGeneral options:\n"
  #if defined(CLAN_LINKED) && defined(CLOOG_LINKED)
		"  -c                    Compile 'file' and create the chain clan|clay|cloog\n"
    "                        equivalent to --readc --printc\n"
  #endif

  #if defined(CLAN_LINKED)
    "  --readc               Read 'file' as a .c (with Clan)\n"
  #endif

  #if defined(CLOOG_LINKED)
    "  --printc              Print a .c (with ClooG)\n"
  #endif

	#if defined(CANDL_LINKED)
			"  --nocandl             Don't check dependencies and print the result\n"
      "  --candl-structure     Set candl structure option \n"
      "  --candl-fullcheck     Set candl fullcheck option \n"
	#endif
  "  --script <file>       Input script file. If not given the script is from\n"
  "                        the scop structure.\n"
  "  --nonormalize         Don't normalize the scop.\n"
  "                        The normalization is done when at the begining of\n"
  "                        clay and at the end of these following functions : \n"
  "                        reorder, split, fuse, iss, strimine, and unroll.\n"
  "  --list                List all the available functions.\n"
  "  -v, --version         Display the release information.\n"
  "  -h, --help            Display this help.\n\n");
  printf(
  "The 'file' is optional, if it's not given, clay will the file from\n"
  "the stdin. By default Clay read a scop and print a scop, Clay can\n"
  "be linked with Clan and Cloog.\n\n"
  "For bug reporting or any suggestions, please send an email to the author\n"
  "Joel Poudroux <joel.poudroux@u-psud.fr>.\n");
}


/**
 * clay_options_version function:
 * This function displays some version informations when the user set the
 * option --version while calling clay. Prints are cut to respect the 509
 * characters limitation of the ISO C 89 compilers.
 */
void clay_options_version() {
  printf("Clay %s       Chunky Loop Alteration wizardrY\n", CLAY_VERSION);
  printf(
  "For any information, please send an email to the author\n"
  "Joel Poudroux <joel.poudroux@u-psud.fr>.\n");
}


/**
 * clay_options_malloc function:
 * Allocate a new options structure
 */
clay_options_p clay_options_malloc() {
  clay_options_p options;
  CLAY_malloc(options, clay_options_p, sizeof(clay_options_t));
  options->from_tag     = 1;
  options->script       = NULL;
  options->input        = stdin;
  options->input_name   = NULL;
  options->print_infos  = 0;
  options->normalize    = 1;

  #if defined(CLAN_LINKED)
  options->readc = 0;
  #endif

  #if defined(CLOOG_LINKED)
  options->printc = 0;
  #endif

  #ifdef CANDL_LINKED
  options->nocandl = 0;
  options->candl_structure = 0;
  options->candl_fullcheck = 0;
  #endif

  return options;
}


/**
 * clay_options_read function:
 * This functions reads all the options. It fills a clay_options_t.
 * \param argv    Number of strings in command line.
 * \param argc    Array of command line strings.
 */
clay_options_p clay_options_read(int argc, char ** argv) {
  int i;
  clay_options_p options = clay_options_malloc();
 
  for (i = 1 ; i < argc ; i++) {
    if (strcmp(argv[i], "--script") == 0) {
      if (i >= argc-1)
        CLAY_error("no file name for --script option");
      options->script = fopen(argv[i+1], "r");
      if (options->script == NULL) {
        fprintf(stderr, "[Clay] Error: cannot open the file %s\n", argv[i+1]);
        exit(1);
      }
      options->from_tag = 0;
      i++;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      clay_options_help();
      options->print_infos = 1;

    #if defined(CLAN_LINKED) && defined(CLOOG_LINKED)
    } else if (strcmp(argv[i], "-c") == 0) {
      options->readc = 1;
      options->printc = 1;
    #endif

    #if defined(CLAN_LINKED)
    } else if (strcmp(argv[i], "--readc") == 0) {
      options->readc = 1;
    #endif

    #if defined(CLOOG_LINKED)
    } else if (strcmp(argv[i], "--printc") == 0) {
      options->printc = 1;
    #endif

	  #ifdef CANDL_LINKED
    } else if (strcmp(argv[i], "--nocandl") == 0) {
      options->nocandl = 1;
    } else if (strcmp(argv[i], "--candl-structure") == 0) {
      options->candl_structure = 1;
    } else if (strcmp(argv[i], "--candl-fullcheck") == 0) {
      options->candl_fullcheck = 1;
    #endif

    } else if (strcmp(argv[i], "--list") == 0) {
      clay_options_list_functions();
      options->print_infos = 1;
    } else if (strcmp(argv[i], "--nonormalize") == 0) {
      options->normalize = 0;
    } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
      clay_options_version();
      options->print_infos = 1;
    } else {
      options->input = fopen(argv[i], "r");
      if (options->input == NULL) {
        fprintf(stderr, "[Clay] Error: cannot open the file %s\n", argv[i]);
        exit(1);
      }
			options->input_name = argv[i];
    }
  }

  return options;
}

