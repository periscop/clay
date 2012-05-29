
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
  "    Sn:      where n >= 1\n"
  "             Refers to the `n'th statement\n"
  "    string:  Iterator name of loop\n\n");
  
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
  "  --script <file>       Input script file. If not given the script is from\n"
  "                        the scop structure.\n"
  "  --nonormalize         Specify to not normalize after each functions\n"
  "                        The normalization is done at the beginning and at the"
  "                        end of the following functions : reorder, fission,\n"
  "                        fuse, iss, strimine, and unroll\n"
  "  --list                List all the available functions.\n"
  "  -v, --version         Display the release information.\n"
  "  -h, --help            Display this help.\n\n");
  printf(
  "The 'file' is optional, if it's not given, clay will read the scop from\n"
  "the stdin.\n\n"
  "For bug reporting or any suggestions, please send an email to the author\n"
  "Joel Poudroux <joel.poudroux@u-psud.fr>.\n");
}


/**
 * clay_options_version function:
 * This function displays some version informations when the user set the
 * option -version while calling clay. Prints are cut to respect the 509
 * characters limitation of the ISO C 89 compilers.
 */
void clay_options_version() {
  printf("Clay %s ", CLAY_VERSION);
  printf("       Chunky Loop Alteration wizardrY\n");
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
  options->input_scop   = 0;
  options->input_script = 0;
  options->script_name  = NULL;
  options->scop_name    = NULL;
  options->print_infos  = 0;
  options->normalize    = 1;
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
        CLAY_error("no file name for -script option");
      options->script_name  = argv[i+1];
      options->input_script = 1;
      i++;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      clay_options_help();
      options->print_infos = 1;
    } else if (strcmp(argv[i], "--list") == 0) {
      clay_options_list_functions();
      options->print_infos = 1;
    } else if (strcmp(argv[i], "--nonormalize") == 0) {
      options->normalize = 0;
    } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
      clay_options_version();
      options->print_infos = 1;
    } else {
      options->scop_name  = argv[i];
      options->input_scop = 1;
    }
  }

  return options;
}

