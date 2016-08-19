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

#ifndef VALIDATION_H_
#define VALIDATION_H_

#include "util.h"
#include <stdarg.h>

#define THRESHOLD 0.0000000001

/*void reduced_validation_array(const int array_size, void *array,
    void *valid_array, int phase);*/

typedef union udouble {
  double d;
  unsigned long u;
} udouble;

#define validation_array(type, array_size, array, valid_array, \
        phase, rawtime) \
    ({ \
    printf ("Starting %s validation for phase %d at %s\n", \
            type, phase, ctime(&rawtime));\
    int valid = 1;\
    for (int i = 0; i < array_size; ++i) {\
      if (array[i] != valid_array[i]) {\
        if (abs(array[i] - valid_array[i]) > THRESHOLD) { \
          valid = 0; \
          printf("%s validation failed during phase %d at elem %d\n", \
                  type, phase, i);\
          printf("%s validation expected (FP) %24.24f and found %24.24f\n", \
                  type, (double) valid_array[i], (double) array[i]);\
          printf("%s validation expected (INT) %d and found %d\n", \
                  type, (int) valid_array[i], (int) array[i]);\
          printf("%s validation expected (ULINT) %lu and found %lu\n", \
                  type, (unsigned long) valid_array[i], \
                  (unsigned long) array[i]);\
         break; \
        }\
      }\
    }\
    if (!valid) {\
      printf("%s validation for phase %d failed\n", type, phase);\
    } else {\
     printf("%s validation for phase %d succeeded\n", type, phase);\
    }\
    printf ("%s validation for phase %d ended at %s\n", type, phase,\
            ctime(&rawtime));\
    })


#define reduced_validation_var(array_size, array, valid_var, phase, rawtime) \
    ({ \
    printf ("Starting reduced validation for phase %d at %s\n", \
            phase, ctime(&rawtime));\
    int valid = 1;\
    for (int i = 0; i < array_size; ++i) {\
      if (array[i] != valid_var) {\
        valid = 0;\
        printf("Reduced validation failed during phase %d at elem %d\n", \
                phase, i);\
        printf("Reduced validation expected %f and found %f\n", \
                valid_var, array[i]);\
        break;\
      }\
    }\
    if (!valid) {\
      printf("Reduced validation for phase %d failed\n", phase);\
    } else {\
     printf("Reduced validation for phase %d succeeded\n", phase);\
    }\
    printf ("Reduced validation for phase %d ended at %s\n", phase,\
            ctime(&rawtime));\
    })

#define reduced_validation_sg_var(valid_var, expected_res, phase, rawtime) \
    ({ \
    printf("Starting reduced validation for phase %d at %s\n", \
          phase, ctime(&rawtime));\
    if (valid_var == expected_res) {\
      printf("Reduced validation for phase %d succeded\n", phase); \
    } else { \
      printf("Reduced validation failed during phase %d\n", phase);\
      printf("Reduced validation expected %f and found %f\n", \
          expected_res, valid_var); \
    }\
    printf("Reduced validation for phase %d ended at %s\n", \
        phase, ctime(&rawtime)); \
    })

void (*compute_fct)(int time_t, va_list);

void full_validation(void (*compute_fct)(int, time_t, va_list), 
    int phase, time_t rawtime, int nr_params, ...);

void phase1_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase2_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase3_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase4_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase5_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase6_compute_wrapper(int phase, time_t rawtime, va_list args);
/*void phase7_compute_wrapper(int phase, time_t rawtime, va_list args);*/
void phase8_compute_wrapper(int phase, time_t rawtime, va_list args);
void phase9_compute_wrapper(int phase, time_t rawtime, va_list args);
/*void phase10_compute_wrapper(int phase, time_t rawtime, va_list args);*/
#endif /* VALIDATION_H_ */
