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

#ifndef UTIL_H_
#define UTIL_H_

#include "meabo.h"

#define NANOS 1000000000LL

void bind_to_available_cpu(int cpu_id, int num_cpus, int print, int skip);

void bind_to_cpu(int cpu_id, int num_cpus, int print);

void bind_to_1_cpu(int cpu_id);

void reset_affinity(int num_cpus, int print);

void bind_to_available_cpu_w_reset(int cpu_id, int num_cpus,
                                   int print, int skip);

void bind_to_cpu_w_reset(int cpu_id, int num_cpus, int print);

unsigned long long duration(struct timespec t1, struct timespec t2);

#if ENABLE_PAPI
char* handle_error(int retval);
PAPI_info* init_PAPI(int num_hwcntrs);
void dinit_PAPI(PAPI_info *papi_info);
#endif

void usage(char *argv[]);

void print_compiler_information(char * exec_name);

int get_num_threads(int phase_cpu_id, int num_cpus, int orig_num_threads);

#endif /* UTIL_H_ */
