
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
 

#ifndef CLAY_BETATREE_H
#define CLAY_BETATREE_H


#include <stdio.h>
#include <osl/scop.h>
#include <clay/array.h>


#define CLAY_BETATREE_ALLOC_CREATE 30



struct clay_betatree;
typedef struct clay_betatree  clay_betatree_t;
typedef struct clay_betatree* clay_betatree_p;

struct clay_betatree {
  clay_betatree_p *nodes;
  int nbnodes; // memory used
  int available;  // total allocated memory
  int value; // the root hasn't a value, it contains only a list of nodes
             // it corresponds to the empty beta
};


clay_betatree_p      clay_betatree_malloc(int, int);
void                 clay_betatree_free();
void                 clay_betatree_add_node(clay_betatree_p, clay_betatree_p);
void                 clay_betatree_print(FILE*, clay_betatree_p, int);
clay_betatree_p      clay_betatree_search(clay_betatree_p, int);

void                 clay_betatree_push_beta(clay_betatree_p, clay_array_p);
clay_betatree_p      clay_betatree_create(osl_scop_p);


#endif
