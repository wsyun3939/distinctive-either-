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
 *  $Id: timer.c,v 1.9 2021/03/31 01:47:47 tanaka Exp tanaka $
 *  $Revision: 1.9 $
 *  $Date: 2021/03/31 01:47:47 $
 *  $Author: tanaka $
 *
 */
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif /* _OPENMP */
#ifndef _MSC_VER
#include <unistd.h>
#include <string.h>
#endif /* !_MSC_VER */

#include "define.h"
#include "timer.h"
#include "problem.h"

#ifdef USE_CLOCK
#include <time.h>
#else  /* !USE_CLOCK */
#include <sys/time.h>
#endif /* !USE_CLOCK */

#define INCLUDE_SYSTEM_TIME

void timer_start(problem_t *problem)
{
#ifdef _OPENMP
  problem->stime = (double) omp_get_wtime();
#ifndef _MSC_VER
#ifdef USE_CLOCK
  problem->stime_total = (double) clock()/(double) CLOCKS_PER_SEC;
#else  /* !USE_CLOCK */
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);
  problem->stime_total = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
#ifdef INCLUDE_SYSTEM_TIME
  problem->stime_total += ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
#endif /* INCLUDE_SYSTEM_TIME */
#endif /* USE_CLOCK */
#endif /* _MSC_VER */
#else /* !_OPENMP */
#ifdef USE_CLOCK
  problem->stime = (double) clock()/(double) CLOCKS_PER_SEC;
#else  /* !USE_CLOCK */

#endif /* USE_CLOCK */
#endif /* !_OPENMP */
}

void set_time(problem_t *problem)
{
#ifdef _OPENMP
#ifndef _MSC_VER
#ifdef USE_CLOCK
  problem->time_total = (double) clock()/(double) CLOCKS_PER_SEC;
#else  /* !USE_CLOCK */
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);
  problem->time_total = ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1000000.0;
#ifdef INCLUDE_SYSTEM_TIME
  problem->time_total += ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1000000.0;
#endif /* INCLUDE_SYSTEM_TIME */
#endif /* !USE_CLOCK */
  problem->time_total -= problem->stime_total;

  if(problem->time_total < 0.0) {
    problem->time_total = 0.0;
  }
#endif /* !_MSC_VER */
  problem->time = (double) omp_get_wtime();
#else /* !_OPENMP */
#ifdef USE_CLOCK
  problem->time = (double) clock()/(double) CLOCKS_PER_SEC;
#else  /* !USE_CLOCK */

#endif /* !USE_CLOCK */
#endif /* !_OPENMP */
  problem->time -= problem->stime;

  if(problem->time < 0.0) {
    problem->time = 0.0;
  }
}

double get_time(problem_t *problem)
{
  set_time(problem);
  return(problem->time);
}
