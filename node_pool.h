/*
 * Copyright 2018-2019 Shunji Tanaka.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  $Id: node_pool.h,v 1.2 2021/03/23 09:15:47 tanaka Exp tanaka $
 *  $Revision: 1.2 $
 *  $Date: 2021/03/23 09:15:47 $
 *  $Author: tanaka $
 *
 */
#ifndef NODE_POOL_H
#define NODE_POOL_H
#include "define.h"
#include "problem.h"
#include "solution.h"

struct node_t {
  struct node_t *next;
  struct node_t *prev;
  uchar active;
  int depth;
  state_t *state;
  solution_t *partial_solution;
};

typedef struct node_t node_t;

typedef struct {
  int n;
  node_t *unused;
  node_t *node;
} node_pool_t;

typedef struct {
  int total_n;
  int *n;
  int min_depth;
  int max_depth;
  node_t *unused;
  node_t **node;
} backtrack_node_pool_t;

#define push_node(x, y) {			\
    if(y != NULL) {				\
      if(x != NULL) {				\
	(y)->prev = (x)->prev;			\
	(x)->prev = y;				\
	(y)->next = x;				\
      }						\
      x = y;					\
    }						\
  }

#define append_node(x, y) {			\
    if(y != NULL) {				\
      if(x == NULL) {				\
	x = y;					\
      } else {					\
	(y)->prev = (x)->prev;			\
	(x)->prev->next = y;			\
	(x)->prev = y;				\
      }						\
    }						\
  }

#define concatenate_list(x, y) {		\
    if(x == NULL) {				\
      x = y;					\
    } else if(y != NULL) {			\
      node_t *tmpn = (x)->prev;			\
      tmpn->next = y;				\
      (x)->prev = (y)->prev;			\
      (y)->prev = tmpn;				\
    }						\
  }

#define pop_node(x, y) {			\
    if(x == NULL) {				\
      y = create_node(problem);			\
    } else {					\
      y = x;					\
      x = (x)->next;				\
      if(x != NULL) {				\
	(x)->prev = (y)->prev;			\
      }						\
      (y)->prev = y;				\
      (y)->next = NULL;				\
      (y)->active = True;			\
    }						\
  }

#define pop_tail_node(x, y) {			\
    if(x == NULL) {				\
      y = create_node(problem);			\
    } else {					\
      y = x->prev;				\
      if(y->prev != y) {			\
	(x)->prev = (y)->prev;			\
	(x)->prev->next = NULL;			\
	(y)->prev = y;				\
	(y)->next = NULL;			\
      }	else {					\
        x = NULL;				\
      }						\
      (y)->active = True;			\
    }						\
  }

#define free_list(x) {				\
    node_t *tmp;				\
    while(x != NULL) {				\
      tmp = x->next;				\
      free_node(x);				\
      x = tmp;					\
    }						\
  }

node_t *create_node(problem_t *);
void free_node(node_t *);
node_pool_t *create_node_pool(void);
void free_node_pool(node_pool_t *);
backtrack_node_pool_t *create_backtrack_node_pool(int);
void free_backtrack_node_pool(backtrack_node_pool_t *);
void append_node_by_element(problem_t *, node_t **, node_t **, state_t *,
			    solution_t *);
#endif /* !NODE_POOL_H */
