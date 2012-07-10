
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


#ifndef CLAY_OPTIONS_H
#define CLAY_OPTIONS_H


struct clay_options {
  FILE *input;      /**< Input file. */
  FILE *script;     /**< Input script file. */
  int from_tag;     /**< 1 read the script from the scop tag, 
                         else from the script file */
	char *input_name; /**< Input file name */
  int print_infos;  /**< 1 if a --help or --version is given */
  int normalize;    /**< 1 the scop will be re-normalized */

  #if defined(CLAN_LINKED) || defined(CLOOG_LINKED)
  int compile;    /**< 1 to create the chain clan | clay | cloog */
	#endif
	#ifdef CANDL_LINKED
	int nocandl;    /**< 1 don't check depedencies with candl*/
  #endif
};

typedef struct clay_options  clay_options_t;
typedef struct clay_options *clay_options_p;

void            clay_options_free(clay_options_p);
void            clay_options_help();
void            clay_options_version();
clay_options_p  clay_options_malloc();
clay_options_p  clay_options_read(int, char**);
void            clay_options_list_functions();


#endif
