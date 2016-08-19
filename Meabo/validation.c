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

#include "validation.h"
#include "compute_kernels.h"

void full_validation(void (*compute_fct)(int, time_t, va_list),
    int phase, time_t rawtime, int nr_params, ...) {
  printf ("Starting full validation computation for phase %d at %s\n",
         phase, ctime(&rawtime));
  va_list args;
  va_start(args, nr_params);

  compute_fct(phase, rawtime, args);
}

/*
 * Wrappers for the compute functions for validation
 */


void phase1_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int array_size = va_arg(args, int);
  int block_size = va_arg(args, int);
  double temp1 = va_arg(args, double);
  double temp2 = va_arg(args, double);
  double temp3 = va_arg(args, double);
  int int_temp1 = va_arg(args, int);
  int int_temp2 = va_arg(args, int);
  int int_temp3 = va_arg(args, int);
  double *valid_vals = va_arg(args, double*);
  int *valid_int_vals = va_arg(args, int*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
  int num_cpus = va_arg(args, int);
  int phase1_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
  int *valid_red_int_vals = va_arg(args, int*);
#endif
  double *vals = va_arg(args, double*);
  int *int_vals = va_arg(args, int*);

  phase1_compute(num_iterations, array_size, block_size, temp1, temp2, temp3,
      int_temp1, int_temp2, int_temp3, valid_vals, valid_int_vals,
      validation_phase, num_threads
#if ENABLE_BINDING
      , num_cpus, phase1_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
#if RED_VALIDATION
      , valid_red_vals, valid_red_int_vals
#endif
    );

  validation_array("Full", array_size, vals, valid_vals, 1, rawtime);
  validation_array("Full", array_size, int_vals, valid_int_vals,
      1, rawtime);
}

void phase2_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int array_size = va_arg(args, int);
  double *valid_dest = va_arg(args, double*);
  double *valid_src1 = va_arg(args, double*);
  double *valid_src2 = va_arg(args, double*);
  int *valid_ind_src2 = va_arg(args, int*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
  int num_cpus = va_arg(args, int);
  int phase2_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
#endif
  double *dest = va_arg(args, double*);

  phase2_compute(num_iterations, array_size, valid_dest, valid_src1, valid_src2,
    valid_ind_src2, validation_phase, num_threads
#if ENABLE_BINDING
    , num_cpus, phase2_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
#if RED_VALIDATION
    , valid_red_vals
#endif
    );
  validation_array("Full", array_size, dest, valid_dest, 2, rawtime);
}


void phase3_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int array_size = va_arg(args, int);
  double *valid_vals = va_arg(args, double*);
  double reduction_var = va_arg(args, double);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
  int num_cpus = va_arg(args, int);
  int phase3_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#if RED_VALIDATION
  double valid_red_reduction_var = va_arg(args, double);
#endif
  double *vals = va_arg(args, double*);

  phase3_compute(num_iterations, array_size, valid_vals, &reduction_var,
    validation_phase, num_threads
#if ENABLE_BINDING
    , num_cpus, phase3_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
#if RED_VALIDATION
    , &valid_red_reduction_var
#endif
    );

  validation_array("Full", array_size, vals, valid_vals, phase, rawtime);
}


void phase4_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int array_size = va_arg(args, int);
  double *valid_dest = va_arg(args, double*);
  double *valid_src1 = va_arg(args, double*);
  double *valid_src2 = va_arg(args, double*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
  int num_cpus = va_arg(args, int);
  int phase4_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
#endif
  double *dest = va_arg(args, double*);

  phase4_compute(num_iterations, array_size, valid_dest, valid_src1,
      valid_src2, validation_phase, num_threads
#if ENABLE_BINDING
      , num_cpus, phase4_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );

  validation_array("Full", array_size, dest, valid_dest, 4, rawtime);
}

void phase5_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int array_size = va_arg(args, int);
  double *valid_dest = va_arg(args, double*);
  double *valid_src1 = va_arg(args, double*);
  double *valid_src2 = va_arg(args, double*);
  int *valid_ind_src1 = va_arg(args, int*);
  int *valid_ind_src2 = va_arg(args, int*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
  int num_cpus = va_arg(args, int);
  int phase5_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
#endif
  double *dest = va_arg(args, double*);

  phase5_compute(num_iterations, array_size, valid_dest, valid_src1,
      valid_src2, valid_ind_src1, valid_ind_src2, validation_phase,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase5_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );

  validation_array("Full", array_size, dest, valid_dest, 5, rawtime);
}

void phase6_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int nrow = va_arg(args, int);
  double **valid_sparse_matrix_values = va_arg(args, double**);
  double *valid_vect_in = va_arg(args, double*);
  int **valid_sparse_matrix_indeces = va_arg(args, int**);
  int *valid_sparse_matrix_nonzeros = va_arg(args, int*);
  double* valid_vect_out = va_arg(args, double*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
#if ENABLE_BINDING
  int num_cpus = va_arg(args, int);
  int phase6_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#endif
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
#endif
  double *vect_out = va_arg(args, double*);

  phase6_compute(num_iterations, nrow, valid_sparse_matrix_values,
      valid_vect_in, valid_sparse_matrix_indeces,
      valid_sparse_matrix_nonzeros, valid_vect_out, validation_phase,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase6_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , NULL
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );

  validation_array("Full", nrow, vect_out, valid_vect_out, 6, rawtime);
}

/* There is no full validation for phase 7. This is here to show that we
 * haven't forgotten about P7.
void phase7_compute_wrapper(int phase, time_t rawtime, va_list args);*/

void phase8_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int num_particles = va_arg(args, int);
  particle *particles = va_arg(args, particle*);
  double *valid_forces = va_arg(args, double*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
#if ENABLE_BINDING
  int num_cpus = va_arg(args, int);
  int phase8_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  PAPI_info *papi_info = va_arg(args, PAPI_info*);
#endif
#if RED_VALIDATION
  double *valid_red_vals = va_arg(args, double*);
#endif
  double *forces = va_arg(args, double*);

  phase8_compute(num_iterations, num_particles, particles, valid_forces,
      validation_phase, num_threads
#if ENABLE_BINDING
      , num_cpus, phase8_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , papi_info
#endif
#if RED_VALIDATION
      , valid_red_vals
#endif
      );

  validation_array("Full", num_particles, forces, valid_forces, 8, rawtime);
}

void phase9_compute_wrapper(int phase, time_t rawtime, va_list args) {
  int num_iterations = va_arg(args, int);
  int num_palindromes = va_arg(args, int);
  unsigned long* restrict valid_palindromes = va_arg(args, unsigned long*);
  int validation_phase = va_arg(args, int);
  int num_threads = va_arg(args, int);
#if ENABLE_BINDING
  int num_cpus = va_arg(args, int);
  int phase9_cpu_id = va_arg(args, int);
  int bind_to_cpu_set = va_arg(args, int);
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  PAPI_info *papi_info = va_arg(args, PAPI_info*);
#endif
#if RED_VALIDATION
  unsigned long* restrict valid_red_ulong_vals = va_arg(args, unsigned long*);
#endif
  unsigned long *palindromes = va_arg(args, unsigned long*);

  phase9_compute(num_iterations, num_palindromes, valid_palindromes,
      validation_phase, num_threads
#if ENABLE_BINDING
      , num_cpus, phase9_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , papi_info
#endif
#if RED_VALIDATION
      , valid_red_ulong_vals
#endif
      );

  validation_array("Full", num_palindromes, palindromes,
                   valid_palindromes, 9, rawtime);

/*  for (int i = 0; i < num_palindromes; ++i) {
    printf("FD %d valid_palindrome %lu palindrome %lu\n", i, valid_palindromes[i],
        palindromes[i]);
  }*/
}
