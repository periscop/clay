
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

#include <stdio.h>
#include <stdlib.h>
#include <clay/macros.h>
#include <clay/beta.h>
#include <clay/betatree.h>
#include <osl/scop.h>
#include <osl/statement.h>


/**
 * clay_betatree_malloc function:
 * \param[in] value          value to set on the new node 
 * \param[in] alloc_memory   first allocation size 
 * \return                   new struct array
 */
clay_betatree_p clay_betatree_malloc(int value, int alloc_memory) {
  clay_betatree_p tree;
  CLAY_malloc(tree, clay_betatree_p, sizeof(clay_betatree_t));
  if (alloc_memory != 0) {
    CLAY_malloc(tree->nodes, clay_betatree_p*, 
                sizeof(clay_betatree_p)*alloc_memory);
    tree->available = alloc_memory;
  } else {
    tree->available = 0;
    tree->nodes = NULL;
  }
  tree->nbnodes = 0;
  tree->value = value;
  return tree;
}


/**
 * clay_betatree_add_node function: 
 * Add a node at the end of the current tree.
 * \param[in] tree
 * \param[in] i
 */
void clay_betatree_add_node(clay_betatree_p tree, clay_betatree_p node) {
  if (tree->nbnodes >= tree->available) {
    tree->available *= 2;
    CLAY_realloc(tree->nodes, clay_betatree_p*, 
                  sizeof(clay_betatree_p)*tree->available);
  }
  tree->nodes[tree->nbnodes] = node;
  (tree->nbnodes)++;
}


/**
 * clay_betatree_push_beta function: 
 * Add a branch in the tree containing the beta
 * \param[in] tree
 * \param[in] beta
 */
void clay_betatree_push_beta(clay_betatree_p tree, clay_array_p beta) {
  int i;
  int value;
  clay_betatree_p node;
  
  for (i = 0 ; i < beta->size ; i++) {
    value = beta->data[i];
    node = clay_betatree_search(tree, value);
    // add the beta value
    if (node == NULL) {
      node = clay_betatree_malloc(value, (i != beta->size-1));
      clay_betatree_add_node(tree, node);
    }
    tree = node;
  }
}


/**
 * clay_betatree_push_beta function: 
 * Search `value' in the nodes array in the tree
 * \param[in] tree
 * \param[in] beta
 * return     node
 */
clay_betatree_p clay_betatree_search(clay_betatree_p tree, int value) {
  // TODO : the array must be sorted
  // -> use dichotomy
  
  int i = 0;
  while (i < tree->nbnodes) {
    if (tree->nodes[i]->value == value) {
      return tree->nodes[i];
    }
    i++;
  }
  
  return NULL;
}


/**
 * clay_betatree_free function: 
 * \param[in] tree
 */
void clay_betatree_free(clay_betatree_p tree) {
  if (!tree)
    return;
  
  int i;
  if (tree->nodes) {
    for (i = 0 ; i < tree->nbnodes ; i++) {
      clay_betatree_free(tree->nodes[i]);
    }
    free(tree->nodes);
  }
  
  free(tree);
}


/**
 * clay_betatree_create function: 
 * \param[in] scop
 * \return    tree
 */
clay_betatree_p clay_betatree_create(osl_scop_p scop) {
  clay_betatree_p tree;
  clay_array_p beta;
  clay_array_p beta_next;
  
  tree      = clay_betatree_malloc(-1, CLAY_BETATREE_ALLOC_CREATE);
  beta      = clay_array_malloc();
  beta_next = clay_beta_next(scop->statement, beta, NULL);
  
  while (beta_next != NULL) {
    clay_array_free(beta);
    beta = beta_next;
    clay_betatree_push_beta(tree, beta);
    beta_next = clay_beta_next(scop->statement, beta, NULL);
  }
  
  clay_array_free(beta);
  
  return tree;
}


/**
 * clay_betatree_free function: 
 * \param[in] out    file where to print
 * \param[in] space  init to 0
 * \param[in] tree
 */
void clay_betatree_print(FILE *out, clay_betatree_p tree, int space) {
  int i;
  
  for (i = 0 ; i < space-1 ; i++) {
    fprintf(out, "  ");
  }
  fprintf(out, " ");
  
  if (space == 0) {
    fprintf(out, "*\n");
  } else {
    if (tree->nodes == 0)
      fprintf(out, "| S %d\n", tree->value);
    else
      fprintf(out, "| L %d\n", tree->value);
  }
  
  space++;
  for (i = 0 ; i < tree->nbnodes ; i++) {
    clay_betatree_print(out, tree->nodes[i], space);
  }
}

