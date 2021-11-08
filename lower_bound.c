/*
 * Copyright 2014-2019 Shunji Tanaka.  All rights reserved.
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
 *  $Id: lower_bound.c,v 1.5 2020/12/03 16:11:14 tanaka Exp tanaka $
 *  $Revision: 1.5 $
 *  $Date: 2020/12/03 16:11:14 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "define.h"
#include "lower_bound.h"
#include "print.h"
#include "problem.h"
#include "solution.h"

#if LOWER_BOUND == 4
#if 1
static void lb_sub(problem_t *, int, int, int *, int, int, stack_state_t *,
		   int, int *);
#else
static void lb_sub(problem_t *, int, int, int *, int, int *, int, int *);
#endif
#endif /* LOWER_BOUND == 4 */

#if LOWER_BOUND == 2
int lower_bound2(problem_t *problem, state_t *state)
{
  int i;
  int lb = state->lb1;
  int src_stack = state->sstate[0].no;
  int max_priority = state->sstate[problem->n_stack - 1].min_priority;

  for(i = 0; i < state->sstate[0].n_stacked; ++i) {
    if(max_priority
       < state->bay[src_stack][state->sstate[0].n_tier - i].block.priority) {
      ++lb;
    }
  }

  return(lb);
}
#elif LOWER_BOUND == 3
/* Zhu, W., Qin, H., Lim, A., Zhang, H. (2012). */
int lower_bound3(problem_t *problem, state_t *state, stack_state_t *csstate,
		 int ub)
{
  int i;
  int lb = state->lb1;
  int n_block = state->n_block;
  stack_state_t *sstate = state->sstate;

  memcpy((void *) csstate, (void *) sstate,
	 (size_t) problem->n_stack*sizeof(stack_state_t));

  state->sstate = csstate;

  while(state->n_block > 0) {
    int src_stack = csstate[0].no;
    int n_stacked = csstate[0].n_stacked;
    int max_priority;

    for(i = problem->n_stack - 1;
	i > 0 && csstate[i].n_tier == problem->s_height; --i);
    if(i == 0) {
      fprintf(stderr, "No space.\n");
      exit(1);
    }
    max_priority = csstate[i].min_priority;

    for(i = 0; i < n_stacked; ++i) {
      if(max_priority
	 < state->bay[src_stack][csstate[0].n_tier--].block.priority) {
	++lb;
      }
    }
#if 1    
    if(lb >= ub) {
      break;
    }
#endif
    state->n_block -= n_stacked;
    csstate[0].n_stacked = 0;

    retrieve_all_blocks(problem, state, NULL);
  }

  state->n_block = n_block;
  state->sstate = sstate;

  return(lb);
}
#elif LOWER_BOUND == 4
/* Tanaka, S., Takii, K. (2016). */
int lower_bound4(problem_t *problem, state_t *state, stack_state_t *csstate,
		 int *pr, int ub)
{
  int i;
  int lb = state->lb1;
  int n_block = state->n_block;
  stack_state_t *sstate = state->sstate;

  memcpy((void *) csstate, (void *) sstate,
	 (size_t) problem->n_stack*sizeof(stack_state_t));

  state->sstate = csstate;

  while(state->n_block > 0) {
    int n_pr = 0, max_i;
    int src_stack = csstate[0].no;
    int n_stacked = csstate[0].n_stacked;
    int max_priority, min_priority = problem->max_priority;
    bay_state_t *src_stackp = state->bay[src_stack];

    for(max_i = problem->n_stack - 1;
	max_i > 0 && csstate[max_i].n_tier == problem->s_height;
	--max_i);
    if(max_i == 0) {
      fprintf(stderr, "No space.\n");
      exit(1);
    }
    max_priority = csstate[max_i].min_priority;

    for(i = 0; i < n_stacked; ++i) {
      int priority = src_stackp[csstate[0].n_tier--].block.priority;

      if(max_priority < priority) {
	++lb;
      } else {
	pr[n_pr++] = priority;
	min_priority = min(min_priority, priority);
      }
    }
#if 1
    if(lb >= ub) {
      break;
    }
#endif
    if(n_pr > 1) {
      int j, k;
      int f = n_pr - 1;
      for(j = 1;
	  j < problem->n_stack && csstate[j].n_tier == problem->s_height; ++j);
      for(k = j; k < problem->n_stack
	    && (csstate[k].n_tier == problem->s_height
		|| csstate[k].min_priority < min_priority);
	  ++k);
      if(max_i - k >= 0) {
	lb_sub(problem, 0, n_pr, pr, k - j, max_i - j + 1,  csstate + j, 0, &f);
	lb += f;
      }
    }
#if 1
    if(lb >= ub) {
      break;
    }
#endif
    state->n_block -= n_stacked;
    csstate[0].n_stacked = 0;

    retrieve_all_blocks(problem, state, NULL);
  }

  state->n_block = n_block;
  state->sstate = sstate;

  return(lb);
}

void lb_sub(problem_t *problem, int depth, int n, int *pr, int s,
	    int w, stack_state_t *sstate, int f, int *ub)
{
  int i;

  if(depth == n) {
    *ub = f;
    return;
  }

  for(i = s; i < w
	&& (sstate[i].min_priority < pr[depth]
	    || sstate[i].n_tier == problem->s_height); ++i);
  if(i < w) {
    int prev_priority = sstate[i].min_priority;
    sstate[i].min_priority = pr[depth];
    lb_sub(problem, depth + 1, n, pr, s, w, sstate, f, ub);
    sstate[i].min_priority = prev_priority;

    if(*ub == 0) {
      return;
    }
  }

  if(i > 0) {
    if(++f < *ub) {
      lb_sub(problem, depth + 1, n, pr, s, w, sstate, f, ub);
    }
  }
}
#elif LOWER_BOUND == 5
/* Quispe, K.E.Y., Lintzmayer, C.N., Xavier, E.C. (2018). */
int lower_bound5(problem_t *problem, state_t *state, stack_state_t *csstate,
		 int *lis, int ub)
{
  int i, j, k;
#if 0
  int max_s, max_priority, max_priority2;
#else
  int max_s, max_priority;
#endif
  int lb = state->lb1;
  int n_block = state->n_block;
#if 0
  int *n_available_stack = lis + problem->s_height;
#endif
  stack_state_t *sstate = state->sstate;

  memcpy((void *) csstate, (void *) sstate,
	 (size_t) problem->n_stack*sizeof(stack_state_t));

  state->sstate = csstate;

  while(state->n_block > 0) {
    int src_stack = csstate[0].no;
    int n_stacked = csstate[0].n_stacked;
    int lb_inc = 0;
    bay_state_t *src_stackp
      = state->bay[src_stack] + (csstate[0].n_tier - n_stacked + 1);

    for(max_s = problem->n_stack - 1;
	max_s > 0 && csstate[max_s].n_tier == problem->s_height;
	--max_s);
    max_priority = csstate[max_s].min_priority;

#if 0
    for(i = max_s, k = 0; i > 0; --i) {
      if(csstate[i].n_tier < problem->s_height) {
	++k;
      }
      for(j = csstate[i].min_priority;
	  j > csstate[i - 1].min_priority; n_available_stack[j] = k, --j);
    }
#endif

#if 0
    if(csstate[max_s].n_tier > csstate[max_s].n_stacked + 1) {
      max_priority2 = csstate[0].min_priority;
      for(i = max_s - 1; i >= 0; --i) {
	bay_state_t *stackp = state->bay[csstate[i].no];

	j = csstate[i].n_tier;
	do {
	  j -= stackp[j].n_stacked + 1;
	} while(stackp[j].min_priority < max_priority);

	if(max_priority2 < stackp[j].min_priority) {
	  max_priority2 = stackp[j].min_priority;
	  if(max_priority2 >= problem->max_priority) {
	    break;
	  }
	}
      }
    } else {
      max_priority2 = problem->max_priority + 1;
    }
#endif

#if 1
    csstate[0].n_tier -= n_stacked;
    for(i = 0; i < n_stacked; ++i) {
      int priority = src_stackp[i].block.priority;

      if(priority <= max_priority) {
	lis[i] = 1;
	for(j = 0; j < i; ++j) {
	  if(src_stackp[j].block.priority >= priority) {
	    if(lis[j] + 1 > lis[i]) {
 	      lis[i] = lis[j] + 1;
	    }
	  }
	}
#if 0
	if(lis[i] - n_available_stack[priority] > lb_inc) {
	  lb_inc = lis[i] - n_available_stack[priority];
#if 0
	  if(lb + lb_inc >= ub) {
	    break;
	  }
#endif
	}
#else
	if(lis[i] > lb_inc + 1) {
	  k = 0;
	  for(j = max_s; csstate[j].min_priority >= priority; --j) {
	    if(csstate[j].n_tier < problem->s_height) {
	      ++k;
	    }
	  }
	  
	  if(lis[i] - k > lb_inc) {
	    lb_inc = lis[i] - k;
#if 0
	    if(lb + lb_inc >= ub) {
	      break;
	    }
#endif
	  }
	}
#endif
      } else {
	lis[i] = -1;
	++lb;
#if 0
	if(priority > max_priority2) {
	  ++lb;
	}
#endif
#if 0
	if(lb >= ub) {
	  break;
	}
#endif
      }
    }
#else
    for(i = n_stacked - 1; i >= 0; --i) {
      int priority = src_stackp[i].block.priority;

      if(priority <= max_priority) {
#if 0
	lis[i] = 1 - n_available_stack[priority];
#else
	lis[i] = 1;
	for(j = max_s; csstate[j].min_priority >= priority; --j) {
	  if(csstate[j].n_tier < problem->s_height) {
	    --lis[i];
	  }
	}
#endif
	for(j = n_stacked - 1; j > i; --j) {
	  if(src_stackp[j].block.priority <= priority) {
	    if(lis[j] + 1 > lis[i]) {
 	      lis[i] = lis[j] + 1;
	    }
	  }
	}

	if(lis[i] > lb_inc) {
	  lb_inc = lis[i];
#if 0
	  if(lb + lb_inc >= ub) {
	    break;
	  }
#endif
	}
      } else {
	lis[i] = - problem->n_stack;
	++lb;
#if 0
	if(priority > max_priority2) {
	  ++lb;
	}
#endif
#if 0
	if(lb >= ub) {
	  break;
	}
#endif
      }
    }
    csstate[0].n_tier -= n_stacked;
#endif

    lb += lb_inc;
#if 1
    if(lb >= ub) {
      break;
    }
#endif   
    state->n_block -= n_stacked;
    csstate[0].n_stacked = 0;

    retrieve_all_blocks(problem, state, NULL);
  }

  state->n_block = n_block;
  state->sstate = sstate;

  return(lb);
}
#elif LOWER_BOUND == 6
/* Bacci, T., Mattia, S. Ventura, P. (2019). */
int lower_bound6(problem_t *problem, state_t *state, stack_state_t *csstate,
		 int *demand, int ub)
{
  int i, j;
  int max_s, max_priority;
  int lb = state->lb1;
  int n_block = state->n_block;
  stack_state_t *sstate = state->sstate;

  memcpy((void *) csstate, (void *) sstate,
	 (size_t) problem->n_stack*sizeof(stack_state_t));

  state->sstate = csstate;

  while(state->n_block > 0) {
    int src_stack = csstate[0].no;
    int n_stacked = csstate[0].n_stacked;
    int cumulative_demand = 0, cumulative_supply = 0;
    int lb_add = 0;
    bay_state_t *src_stackp
      = state->bay[src_stack] + (csstate[0].n_tier - n_stacked + 1);

    for(max_s = problem->n_stack - 1;
	max_s > 0 && csstate[max_s].n_tier == problem->s_height;
	--max_s);
    max_priority = csstate[max_s].min_priority;

    memset((void *) demand, 0, (size_t) problem->n_stack*sizeof(int));

    csstate[0].n_tier -= n_stacked;
    for(i = 0; i < n_stacked; ++i) {
      int priority = src_stackp[i].block.priority;

      if(priority > max_priority) {
	++lb;
      } else {
	for(j = 1; j < problem->n_stack; ++j) {
	  if(csstate[j].n_tier < problem->s_height
	     && csstate[j].min_priority >= priority) {
	    break;
	  }
	}
	++demand[j];
      }
    }

#if 1
    if(lb >= ub) {
      break;
    }
#endif   
    
    for(i = max_s; i > 0; --i) {
      cumulative_demand += demand[i];
      cumulative_supply += problem->s_height - csstate[i].n_tier;
      lb_add = max(lb_add, cumulative_demand - cumulative_supply);
    }

    lb += lb_add;

#if 1
    if(lb >= ub) {
      break;
    }
#endif   

    state->n_block -= n_stacked;
    csstate[0].n_stacked = 0;

    retrieve_all_blocks(problem, state, NULL);
  }

  state->n_block = n_block;
  state->sstate = sstate;

  return(lb);
}
#elif LOWER_BOUND == 7
/* Tanaka, S. (2019) */
int lower_bound7(problem_t *problem, state_t *state, stack_state_t *csstate,
		 int *stacked_block, int ub)
{
  int i, j;
  int max_s, max_priority;
  int lb = state->lb1;
  int n_block = state->n_block;
  int *lis = stacked_block + problem->n_block;
  int *lisp = lis + problem->n_block;
#if 0
  int *n_available_stack = lisp + problem->n_block;
#endif
  int n_stacked_block = 0;
  stack_state_t *sstate = state->sstate;

  memcpy((void *) csstate, (void *) sstate,
	 (size_t) problem->n_stack*sizeof(stack_state_t));

  state->sstate = csstate;

  while(state->n_block > 0) {
    int n = 0;
    int src_stack = csstate[0].no;
    int n_stacked = csstate[0].n_stacked;
    bay_state_t *src_stackp
      = state->bay[src_stack] + (csstate[0].n_tier - n_stacked + 1);

    for(max_s = problem->n_stack - 1;
	max_s > 0 && csstate[max_s].n_tier == problem->s_height;
	--max_s);
    max_priority = csstate[max_s].min_priority;

    for(i = n_stacked - 1; i >= 0; --i) {
      if(src_stackp[i].block.priority <= max_priority) {
	stacked_block[n_stacked_block + n++] = src_stackp[i].block.priority;
      } else {
	++lb;
#if 1
	if(lb >= ub) {
	  break;
	}
#endif
      }
    }

    if(lb >= ub) {
      break;
    }

    if(n > 0) {
      int target_priority = csstate[0].min_priority;
#if 1
      int lb_inc = 1, lb_end = -1, lb_n = problem->n_stack;
#else
      int lb_inc = 0, lb_end = -1;
#endif

      n_stacked_block += n;
#if 0
      for(i = max_s, n = 0; i > 0; --i) {
	if(csstate[i].n_tier < problem->s_height) {
	  ++n;
	}
	for(j = csstate[i].min_priority;
	    j > csstate[i - 1].min_priority; n_available_stack[j] = n, --j);
      }
#endif
#if 1
      for(i = n_stacked_block - 1; i >= 0; --i) {
	lisp[i] = -1;
	if(stacked_block[i] <= target_priority) {
	  lis[i] = -1;
	} else {
	  lis[i] = 1;
	  for(j = n_stacked_block - 1; j > i; --j) {
	    if(stacked_block[j] >= stacked_block[i]) {
	      if(lis[j] + 1 >= lis[i]) {
		lis[i] = lis[j] + 1;
		lisp[i] = j;
	      }
	    }
	  }
#if 0
	  n = n_available_stack[stacked_block[i]];
	  if(lis[i] - n > lb_inc || (lis[i] - n == lb_inc && n <= lb_n)) {
	    lb_inc = lis[i] - n;
	    lb_end = i;
	    lb_n = n;
#if 1
	    if(lb + lb_inc >= ub) {
	      break;
	    }
#endif
	  }
#else
	  if(lis[i] >= lb_inc + 1) {
	    n = 0;
	    for(j = max_s; csstate[j].min_priority >= stacked_block[i]; --j) {
	      if(csstate[j].n_tier < problem->s_height) {
		++n;
	      }
	    }

	    if(lis[i] - n > lb_inc || (lis[i] - n == lb_inc && n <= lb_n)) {
	      lb_inc = lis[i] - n;
	      lb_end = i;
	      lb_n = n;
#if 1
	      if(lb + lb_inc >= ub) {
		break;
	      }
#endif
	    }
	  }
#endif
	}
      }
#else
      for(i = 0; i < n_stacked_block; ++i) {
	lisp[i] = -1;
	if(stacked_block[i] <= target_priority) {
	  lis[i] = - problem->n_stack;
	} else {
#if 0
	  lis[i] = 1 - n_available_stack[stacked_block[i]];
#else
	  lis[i] = 1;
	  for(j = max_s; csstate[j].min_priority >= stacked_block[i]; --j) {
	    if(csstate[j].n_tier < problem->s_height) {
	      --lis[i];
	    }
	  }
#endif
	  for(j = 0; j < i; ++j) {
	    if(stacked_block[j] <= stacked_block[i]) {
	      if(lis[j] + 1 > lis[i]) {
		lis[i] = lis[j] + 1;
		lisp[i] = j;
	      }
	    }
	  }
	}

	if(lis[i] > lb_inc) {
	  lb_inc = lis[i];
	  lb_end = i;
#if 1
	  if(lb + lb_inc >= ub) {
	    break;
	  }
#endif
	}
      }
#endif
      if(lb_end >= 0) {
	lb += lb_inc;
#if 1
	if(lb >= ub) {
	  break;
	}
#endif
	for(i = lb_end; i != -1; stacked_block[i] = -1, i = lisp[i]);
      }
    }

    state->n_block -= n_stacked;
    csstate[0].n_tier -= n_stacked;
    csstate[0].n_stacked = 0;

    retrieve_all_blocks(problem, state, NULL);
  }

  state->n_block = n_block;
  state->sstate = sstate;

  return(lb);
}
#endif /* LOWER_BOUND == 7 */
