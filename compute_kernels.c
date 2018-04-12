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

#include "compute_kernels.h"
void phase1_compute(const int num_iterations, const int array_size,
    const int block_size, register double temp1, register double temp2,
    register double temp3, register int int_temp1, register int int_temp2,
    register int int_temp3, double *vals, int *int_vals, int validation_phase,
    int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase1_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals, int *valid_red_int_vals
#endif
    ) {
  #pragma omp parallel private (temp1, temp2, temp3, \
    int_temp1, int_temp2, int_temp3) shared(vals, int_vals) \
    if (!validation_phase) num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase1_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase1_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for simd
      for (int i = 0; i < array_size; i += block_size) {
        for (int j = i; j < i + block_size; ++j) {
          temp1 = vals[j];
          int_temp1 = int_vals[j];
          temp1 *= temp1;
          temp2 = temp1 + vals[j];
          temp3 = temp2 / (1024 + temp1);
          temp3 -= vals[j];
          int_temp1 *= int_temp1;
          int_temp2 = int_temp1 + int_vals[j];
          int_temp3 = int_temp2 / (1024 + int_temp1);
          int_temp3 -= int_vals[j];
          vals[j] = temp3;
          int_vals[j] += (int_temp1 + int_temp2 + int_temp3) % 1024;
#if RED_VALIDATION
          valid_red_vals[j] = vals[j];
          valid_red_int_vals[j] = int_vals[j];
#endif
        }
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }
#endif
  }
}

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
    ) {
  #pragma omp parallel shared(dest, src1, src2, ind_src2) \
    if (!validation_phase) num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase2_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase2_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for
      for (int i = 0; i < array_size; ++i) {
        dest[i] += src1[i] * src2[ind_src2[i]];
#if RED_VALIDATION
        valid_red_vals[i] = dest[i];
#endif
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }

#endif
  }
}

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
    ) {
  double tmp_reduction_var = 0;
  #pragma omp parallel shared(vals, tmp_reduction_var) if (!validation_phase) \
    num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase3_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase3_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for reduction(+:tmp_reduction_var)
      for (int i = 0; i < array_size; ++i) {
        vals[i] += 8;
        tmp_reduction_var += vals[i];
      }
      
      int tmp_rounding = tmp_reduction_var * 1000000;
      tmp_reduction_var = tmp_rounding  / 1000000;
      *reduction_var = fmod(tmp_reduction_var, 1024);
      #pragma omp for
      for (int i = 0; i < array_size; ++i) {
        vals[i] = *reduction_var;
      }
#if RED_VALIDATION
      *valid_red_reduction_var = *reduction_var;
#endif
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }
#endif
  }
}

void phase4_compute(const int num_iterations, const int array_size,
    double *dest, double *src1, double *src2, int validation_phase,
    int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase4_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    ) {
  #pragma omp parallel shared(dest, src1, src2) \
    if (!validation_phase) num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase4_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase4_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for
      for (int i = 0; i < array_size; ++i) {
        dest[i] += src1[i] + src2[i];
#if RED_VALIDATION
        valid_red_vals[i] = dest[i];
#endif
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }
#endif
  }
}

void phase5_compute(const int num_iterations, const int array_size,
    double *dest, double *src1, double *src2, int *ind_src1, int *ind_src2,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase5_cpu_id, int bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    ) {
  #pragma omp parallel shared(dest, src1, ind_src1, ind_src2, \
    src2) if (!validation_phase) num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase5_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase5_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for
      for (int i = 0; i < array_size; ++i) {
        dest[i] += src1[ind_src1[i]] + src2[ind_src2[i]];
#if RED_VALIDATION
        valid_red_vals[i] = dest[i];
#endif
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }
#endif
  }
}

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
    ) {
  #pragma omp parallel if (!validation_phase) num_threads(num_threads) \
    shared(sparse_matrix_values, sparse_matrix_indeces, sparse_matrix_nonzeros,\
           vect_in, vect_out)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase6_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase6_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    if ((retval = PAPI_start_counters(papi_info->event_code,
                                      papi_info->total_events)) != PAPI_OK) {
      printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
    }
  }
#endif
    for (int iter = 0; iter < num_iterations/5; ++iter) {
      double reduction_var;

      #pragma omp for private(reduction_var)
      for (int i = 0 ; i < nrow; ++i) {
        reduction_var = 0.0;
        double * restrict values = sparse_matrix_values[i];
        int *restrict cols = sparse_matrix_indeces[i];
        const int nonzeros = sparse_matrix_nonzeros[i];

        for (int j = 0; j < nonzeros; ++j) {
          reduction_var += values[j] * vect_in[cols[j]];
        }
        vect_out[i] = reduction_var;
#if RED_VALIDATION
        valid_red_vals[i] = reduction_var;
#endif
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
  #pragma omp critical
  {
    int retval;
    unsigned long long event_values[papi_info->total_events];
    memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
    if ((retval = PAPI_stop_counters(event_values,
                                     papi_info->total_events)) != PAPI_OK) {
      printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
    }
    for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
      printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
      printf("Thread %d %s value = %lld\n", 0,
#endif
                                            papi_info->event_code_str[i],
                                            event_values[i]);
    }
  }
#endif
  }
}

void phase7_compute(const int num_iterations, const int array_size,
    linked_list **llist,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase7_cpu_id, int bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_reduction_var
#endif
) {
#pragma omp parallel firstprivate(llist) \
    if (!validation_phase) num_threads(num_threads)
  {
#ifdef _OPENMP
    linked_list *start_node = llist[omp_get_thread_num()];
#else
    linked_list *start_node = llist[0];
#endif
    linked_list *orig_cur_node = malloc(sizeof(linked_list*));
    linked_list *cur_node;
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase7_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase7_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      if ((retval = PAPI_start_counters(papi_info->event_code,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
      }
    }
#endif
    for (int iter = 0; iter < num_iterations; ++iter) {
      cur_node = orig_cur_node;
      cur_node->value = start_node->value;
      cur_node->next = start_node->next;
      while (cur_node != NULL) {
#if RED_VALIDATION
        if ((iter >= num_iterations - 1) && (cur_node->next == NULL)) {
          *valid_red_reduction_var = cur_node->value;
        }
#endif
        cur_node = cur_node->next;
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      unsigned long long event_values[papi_info->total_events];
      memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
      if ((retval = PAPI_stop_counters(event_values,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
      }
      for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
        printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
        printf("Thread %d %s value = %lld\n", 0,
#endif
            papi_info->event_code_str[i],
            event_values[i]);
      }
    }
#endif
  }
}

void phase8_compute(const int num_iterations, const int num_particles,
    particle* restrict particles, double* restrict forces,
    int validation_phase, int num_threads
#if ENABLE_BINDING
    , int num_cpus, int phase8_cpu_id, int bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
    , PAPI_info *papi_info
#endif
#if RED_VALIDATION
    , double *valid_red_vals
#endif
    ) {
  /*
   * This phase does a 3D distance calculation between particles and calculates
   * the electrostatic force between pairs of points.
   */
#pragma omp parallel shared(forces) firstprivate(particles) \
  if(!validation_phase) num_threads(num_threads)
  {
    double k = 8.987551 * 1000000000;

#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase8_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase8_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
    }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      if ((retval = PAPI_start_counters(papi_info->event_code,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
      }
    }
#endif

    for (int iter = 0; iter < num_iterations; ++iter) {
      #pragma omp for simd
      for (int i = 0; i < num_particles-1; ++i) {
        double r = (particles[i+1].x - particles[i].x) *
                   (particles[i+1].x - particles[i].x) +
                   (particles[i+1].y - particles[i].y) *
                   (particles[i+1].y - particles[i].y) +
                   (particles[i+1].z - particles[i].z) *
                   (particles[i+1].z - particles[i].z);

        forces[i] = (k * particles[i].charge * particles[i+1].charge) / r;

#if RED_VALIDATION
        valid_red_vals[i] = forces[i];
#endif
      }
    }
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      unsigned long long event_values[papi_info->total_events];
      memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
      if ((retval = PAPI_stop_counters(event_values,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
      }
      for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
        printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
        printf("Thread %d %s value = %lld\n", 0,
#endif
            papi_info->event_code_str[i],
            event_values[i]);
      }
    }
#endif
  }
}

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
    ) {
  /*
   * Computes the first N palindromes
   */

  unsigned int is_palindrome = 0;
  unsigned long num = 0;
  int found = 0;
  unsigned long latest_pal = 0;
  unsigned int latest_i = 0;
  int total_palindromes = 0;
  int from_zero = 0;

#pragma omp parallel firstprivate(palindromes, total_palindromes, from_zero) private(found, num, \
  is_palindrome) \
  if(!validation_phase) num_threads(num_threads) shared(latest_pal, latest_i)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase9_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase9_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
  }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      if ((retval = PAPI_start_counters(papi_info->event_code,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to start counters %d: %s\n", retval,
               handle_error(retval));
      }
    }
#endif

    for (int iter = 0; iter < num_iterations/10; ++iter) {
      num = 0;
      latest_pal = 0;
      latest_i = 0;
      #pragma omp for
      for (int i = 0; i < num_entries; ++i) {
        if (i == 0) {
          palindromes[i] = num;
#if RED_VALIDATION
          valid_red_ulong_vals[i] = palindromes[i];
#endif
        } else {
          if (i > latest_i) {
            found = latest_i;
            num = palindromes[found] + 1;
          } else {
            ++from_zero;
            found = -1;
            num = 0;
          }

          while (found < i) {
            if (num < 10) {
              palindromes[i] = num;
              is_palindrome = 1;
            } else {
              is_palindrome = 1;
              unsigned long tmp_num = num;
              unsigned int length = 0;
               while (tmp_num) {
                length += 1;
                tmp_num /= 10;
              }
              tmp_num = num;
              while (tmp_num) {
                if ((tmp_num % 10) !=
                    (tmp_num / (unsigned long) pow(10, (length-1)))) {
                  is_palindrome = 0;
                  break;
                }
                tmp_num %= (unsigned long) pow(10, (length-1));
                tmp_num /= 10;
                length -= 2;
              }
            }
            if (is_palindrome) {
              ++found;
            }
            ++num;
          }
          palindromes[i] = num-1;
          ++total_palindromes;
          if ((i > latest_i) && (palindromes[i] > latest_pal)) {
            latest_pal = palindromes[i];
            latest_i = i;
          }
#if RED_VALIDATION
          valid_red_ulong_vals[i] = palindromes[i];
#endif
        }
      }
    }

#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      unsigned long long event_values[papi_info->total_events];
      memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
      if ((retval = PAPI_stop_counters(event_values,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
      }
      for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
        printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
        printf("Thread %d %s value = %lld\n", 0,
#endif
            papi_info->event_code_str[i],
            event_values[i]);
      }
      printf("Thread %d total_palindromes value = %d\n", omp_get_thread_num(), total_palindromes);
      printf("Thread %d from_zero value = %d\n", omp_get_thread_num(), from_zero);
    }
#endif
  }
}

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
    ) {
  /*
   * GUPS-like kernel
   */
  int index;
  unsigned int seed;
#pragma omp parallel firstprivate(randomloc, num_randomloc) \
  private(index, seed)\
  if(!validation_phase) num_threads(num_threads)
  {
#if ENABLE_BINDING
    if (bind_to_cpu_set) {
      bind_to_cpu_w_reset(phase10_cpu_id, num_cpus, 0);
    } else {
#ifdef _OPENMP
      bind_to_available_cpu_w_reset(phase10_cpu_id, num_cpus, 0,
                                    omp_get_thread_num());
#endif
  }
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      if ((retval = PAPI_start_counters(papi_info->event_code,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to start counters %d: %s\n", retval,
               handle_error(retval));
      }
    }
#endif

#ifdef _OPENMP
    seed = omp_get_thread_num();
#else
    seed = rand();
#endif
    #pragma omp for
    for (unsigned long iter = 0; iter < num_iterations * num_randomloc; ++iter) {
      index = rand_r(&seed) % num_randomloc;
      randomloc[index] = index;
    }

#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
#pragma omp critical
    {
      int retval;
      unsigned long long event_values[papi_info->total_events];
      memset(event_values, 0, papi_info->total_events * sizeof(unsigned long long));
      if ((retval = PAPI_stop_counters(event_values,
          papi_info->total_events)) != PAPI_OK) {
        printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
      }
      for (int i = 0; i < papi_info->total_events; ++i) {
#ifdef _OPENMP
        printf("Thread %d %s value = %lld\n", omp_get_thread_num(),
#else
        printf("Thread %d %s value = %lld\n", 0,
#endif
            papi_info->event_code_str[i],
            event_values[i]);
      }
    }
#endif

  }

}
