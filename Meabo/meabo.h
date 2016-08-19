/*
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (C) 2016, ARM Limited and contributors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 *
 */

#ifndef MEABO_H_
#define MEABO_H_

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/signal.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "sched.h"
#include "assert.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef SYS_gettid
#define gettid() syscall(SYS_gettid)
#else
#error "SYS_gettid unavailable on this system"
#endif

// Control variables
#define ENABLE_PAPI 0
#define ENABLE_BINDING 1
#define ENABLE_ENERGY 0
#define FULL_VALIDATION 0 
#define RED_VALIDATION 0

typedef struct linked_list {
    struct linked_list *next;
    double value;
} linked_list;

typedef struct particle {
    int x;
    int y;
    int z;
    double charge;
} particle;

#if ENABLE_PAPI
#include "papi.h"

typedef struct PAPI_info {
  char **event_code_str;
  int total_events;
  int *event_code;
  int num_hwcntrs;
} PAPI_info;

#endif

#if ENABLE_ENERGY
#include "readenergy.h"
#endif

#endif 
