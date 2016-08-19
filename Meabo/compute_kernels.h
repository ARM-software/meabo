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

#ifndef COMPUTE_KERNELS_H_
#define COMPUTE_KERNELS_H_

#include "meabo.h"
#include "util.h"

void phase1_compute(const int num_iterations, const int array_size,
    const int block_size, register double temp1, register double temp2,
    register double temp3, register int int_temp1, register int int_temp2,
    register int int_temp3, double *vals, int *int_vals,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase1_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals, int *valid_red_int_vals
#endif
    );

void phase2_compute(const int num_iterations, const int array_size,
    double *dest, double *src1, double *src2, int *ind_src2,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase2_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase3_compute(const int num_iterations, const int array_size,
    double *vals, double *reduction_var, int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase3_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_reduction_var
#endif
    );

void phase4_compute(const int num_iterations, const int array_size,
    double *dest, double *src1, double *src2, int validation_phase,
    int num_threads
#if ENABLE_BINDING
    ,int num_cpus, int phase4_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase5_compute(const int num_iterations, const int array_size,
    double *dest, double *src1, double *src2, int *ind_src1,
    int *ind_src2, int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase5_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase6_compute(const int num_iterations, const int nrow,
    double **sparse_matrix_values, double *vect_in,
    int **sparse_matrix_indeces, int *sparse_matrix_nonzeros, double *vect_out,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase6_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase7_compute(const int num_iterations, const int array_size,
    linked_list **llist,
    int validation, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase7_cpu_id, int bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase8_compute(const int num_iterations, const int num_particles,
    particle* restrict particles, double* restrict forces,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase8_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    );

void phase9_compute(const int num_iterations, const int num_entries,
    unsigned long* restrict palindromes, int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase9_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , unsigned long* restrict valid_red_ulong_vals
#endif
    );

void phase10_compute(const int num_iterations, int num_randomloc,
    int *randomloc, int validation_phase,
    int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase10_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , unsigned int* restrict valid_red_int_vals
#endif
    );
#endif /* COMPUTE_KERNELS_H_ */
