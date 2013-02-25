
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                            ident.c                                 |
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
#include <string.h>

#include <osl/scop.h>

#include <clay/macros.h>
#include <clay/array.h>
#include <clay/beta.h>
#include <clay/betatree.h>
#include <clay/util.h>
#include <clay/ident.h>


/**
 * clay_ident_find_stmt function:
 * Search the corresponding beta of the `ident'th statement
 * \param[in] scop
 * \param[in] ident       >= 0
 * \return
 */
clay_array_p clay_ident_find_stmt(osl_scop_p scop, int ident) {
  
  if (ident < 0)
    return NULL;
  
  osl_statement_p sout;
  clay_array_p beta;
  clay_array_p beta_last;
  int i = 0;
  
  beta_last = clay_array_malloc(); // empty beta
  beta = clay_beta_next(scop->statement, beta_last, &sout);
  
  while (i < ident) {
    clay_array_free(beta_last);
    beta_last = beta;
    beta = clay_beta_next(scop->statement, beta_last, &sout);
    
    if (beta == NULL) {
      clay_array_free(beta_last);
      return NULL;
    }
    
    i++;
  }
  
  clay_array_free(beta_last);
  
  return beta;
}


/**
 * clay_ident_find_iterator function:
 * Search the first loop which has the `iter' in original iterator
 * \param[in] scop
 * \param[in] iter       name of the original iterator we want to search
 * \return
 */
clay_array_p clay_ident_find_iterator(osl_scop_p scop, char *iter) {
  
  osl_statement_p sout;
  clay_array_p beta;
  clay_array_p beta_last;
  int i = -1;
  
  beta_last = clay_array_malloc(); // empty beta
  beta = clay_beta_next(scop->statement, beta_last, &sout);
  
  if (beta == NULL) {
    clay_array_free(beta_last);
    return NULL;
  }
  
  while ((i = clay_util_statement_find_iterator(sout, iter)) == -1) {
    clay_array_free(beta_last);
    beta_last = beta;
    beta = clay_beta_next(scop->statement, beta_last, &sout);
    
    if (beta == NULL) {
      clay_array_free(beta_last);
      return NULL;
    }
  }
  
  clay_array_free(beta_last);
  
  if (i == -1) {
    clay_array_free(beta);
  } else {
    beta->size = i+1;
  }
  
  return beta;
}


/**
 * clay_ident_find_loop function:
 * Search the `ident'th loop
 * /!\ Assume that all nodes are sorted in ascending order 
 * \param[in] scop
 * \param[in] ident       >= 1
 * \return
 */
clay_array_p clay_ident_find_loop(clay_betatree_p tree, int ident) {
  int *count;
  clay_array_p beta;
  
  CLAY_malloc(count, int*, sizeof(int));
  *count = 0;
  
  beta = clay_ident_find_loop_aux(tree, ident, count);
  free(count);
  
  return beta;
}


/**
 * clay_ident_find_loop_aux function:
 * Required by clay_ident_find_loop
 */
clay_array_p clay_ident_find_loop_aux(clay_betatree_p tree, int ident,
                                      int *count) {
  
  if (tree->nbnodes == 0 || ident < 0)
    return NULL;
  
  if (*count > ident) {
    free(count);
    return NULL;
  }
  
  int i;
  clay_betatree_p node;
  clay_array_p beta;
  clay_array_p beta_rest;
  
  beta = clay_array_malloc();
  
  // if we are not at the root (the root has no value)
  if (*count != 0)
    clay_array_add(beta, tree->value);

  // OK the beta is found
  if (*count == ident)
    return beta;
  
  (*count)++;
  
  // TODO : we need to sort the nodes before
  // No problems for now because when we create a tree from a scop, the nodes
  // are sorted by ascending order
  for (i = 0 ; i < tree->nbnodes ; i++) {
    node = tree->nodes[i];
    
    // get the rest of the branch
    // if not null, we have found the `ident'th loop
    beta_rest = clay_ident_find_loop_aux(node, ident, count);
    
    if (beta_rest != NULL) {
      clay_array_concat(beta, beta_rest);
      clay_array_free(beta_rest);
      return beta;
    }
    
    clay_array_free(beta_rest);
  }
  
  clay_array_free(beta);
  
  return NULL;
}
