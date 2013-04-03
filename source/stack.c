
   /*--------------------------------------------------------------------+
    |                              Clay                                  |
    |--------------------------------------------------------------------|
    |                             stack.y                                |
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
#include <clay/macros.h>
#include <clay/stack.h>
#include <clay/data.h>

#include <clay/array.h>

// The stack is growing
// When we push a data, it's copied at the next case in the stack
// so we start at -1

inline void clay_stack_init(clay_stack_p s) {
  s->sp = -1;
}

void clay_stack_push(clay_stack_p s, clay_data_p d) {
  if (s->sp == CLAY_STACK_MAX)
    CLAY_error("Stack overflow, please recompile with a higher CLAY_STACK_MAX");
  s->sp++;

  s->stack[s->sp].type = d->type;
  s->stack[s->sp].data = d->data;
}

clay_data_p clay_stack_pop(clay_stack_p s) {
  if (s->sp == -1)
    CLAY_error("Stack overflow, can't pop");

  return &s->stack[s->sp--];
}

void clay_stack_clear(clay_stack_p s) {
  int i;
  for (i = 0 ; i <= s->sp ; i++)
    clay_data_clear(&s->stack[i]);
  s->sp = -1;
}

