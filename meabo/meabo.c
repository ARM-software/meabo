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

#include "meabo.h"
#include "compute_kernels.h"
#include "validation.h"
#include "util.h"

int main(int argc, char** argv) {
  printf("Meabo v1.0\n");
  time_t rawtime;
  time(&rawtime);
  printf("Program started at %s\n", ctime(&rawtime));

  // Print what flags and compiler it was compiled with -- works only with GCC
  print_compiler_information(argv[0]);

  // Default values
  int num_iterations = 1000;
  long array_size = 1 << 20;
  long llist_size = 1 << 24;
  int nrow = 1 << 14;
  int ncol = 1 << 14;
  int num_particles = 1 << 20;
  int num_palindromes = 1 << 10;
  int num_randomloc = 1 << 21;

  // Data structure initialisation
  int print = 1;
  double reduction_var = 0;
  int block_size = 8;
  int i = 0, j = 0, k = 0, col = 0;
  register double temp1 = 1.0, temp2 = 1.0, temp3 = 1.0;
  register int int_temp1 = 1, int_temp2 = 1, int_temp3 = 1;
  struct timespec t1, t2;
  unsigned long long total_exec_time = 0;
  /*
   * The CPU IDs have the following format:  
   * - the bits corresponding to the CPUs on which the phase is to be run 
   * should be set to 1. The input is the decimal representation.
   */
  int num_cpus = 6, phase1_cpu_id = -1, phase2_cpu_id = -1,
      phase3_cpu_id = -1, phase4_cpu_id = -1, phase5_cpu_id = -1,
      phase6_cpu_id = -1, phase7_cpu_id = -1, phase8_cpu_id = -1,
      phase9_cpu_id = -1, phase10_cpu_id = -1;
  int bind_to_cpu_set = 1;
  /*
   * For now, we only support 32 phases. Will extend in the future, 
   * if necessary. 
   * Format:
   * - 0: reset value (assumption: you always want to run at least 1 phase)
   * - set bits corresponding to the phase you want to run to 1 (input 
   * is the decimal representation)
   */
  unsigned int run_phases = 0;
  /* 
   * This variable controls how many threads the application will be using. 
   * Format:
   * - 0: if phaseX_cpu_id = -1 then num_threads = num_cpus
   *      else num_threads = number of cpus set in phaseX_cpu_id 
   * - non 0: user inputted num_threads
   * This variable is set before every kernel function call by calling
   * get_num_threads.
   */
  int num_threads = 0; 
  /*
   * Data structures for the compute kernels.
   */
  double * restrict vals = NULL, * restrict src1 = NULL, 
         * restrict src2 = NULL, * restrict dest = NULL, 
         * restrict forces = NULL;
  int * restrict int_vals = NULL, * restrict ind_src1 = NULL, 
      * restrict ind_src2 = NULL, * restrict randomloc = NULL;
  int ** restrict sparse_matrix_indeces = NULL;
  double ** restrict sparse_matrix_values = NULL;
  int * restrict sparse_matrix_nonzeros = NULL;
  double * restrict vect_in = NULL, * restrict vect_out = NULL;
  linked_list ** llist = NULL;
  particle * restrict particles = NULL;
  unsigned long * restrict palindromes = NULL;

#if ENABLE_PAPI
  /*
   * We are using 7 counters by default, as that is the number of counters
   * of the ARM Cortex-A57 and ARM Cortex-A53, on which we developed this 
   * micro-benchmark. Each of these cores has 6 general purpose counters and
   * one cycle counter.
   */
  unsigned int num_hwcntrs = 7;
  PAPI_info *papi_info = NULL;
#endif


  // Parse arguments
  while ((k = getopt(argc, argv, 
		     "s:r:c:i:b:C:1:2:3:4:5:6:7:8:9:0:H:P:T:B:l:p:x:R:h")) != -1) {
    switch(k) {
      case 's':
        array_size = atoll(optarg);
        break;
      case 'r':
        nrow = atoi(optarg);
        break;
      case 'c':
        ncol = atoi(optarg);
        break;
      case 'i':
        num_iterations = atoi(optarg);
        break;
      case 'b':
        block_size = atoi(optarg);
        break;
      case 'C':
        num_cpus = atoi(optarg);
        break;
      case '1':
        phase1_cpu_id = atoi(optarg);
        break;
      case '2':
        phase2_cpu_id = atoi(optarg);
        break;
      case '3':
        phase3_cpu_id = atoi(optarg);
        break;
      case '4':
        phase4_cpu_id = atoi(optarg);
        break;
      case '5':
        phase5_cpu_id = atoi(optarg);
        break;
      case '6':
        phase6_cpu_id = atoi(optarg);
        break;
      case '7':
        phase7_cpu_id = atoi(optarg);
        break;
      case '8':
        phase8_cpu_id = atoi(optarg);
        break;
      case '9':
        phase9_cpu_id = atoi(optarg);
        break;
      case '0':
        phase10_cpu_id = atoi(optarg);
        break;
#if ENABLE_PAPI
      case 'H':
        num_hwcntrs = atoi(optarg);
        break;
#endif
      case 'P':
        run_phases = atoi(optarg);
        break;
      case 'T':
        num_threads = atoi(optarg);
        break;
      case 'B':
        bind_to_cpu_set = atoi(optarg);
        break;
      case 'l':
        llist_size = atol(optarg);
        break;
      case 'p':
        num_particles = atoi(optarg);
        break;
      case 'x':
        num_palindromes = atoi(optarg);
        break;
      case 'R':
        num_randomloc = atoi(optarg);
        break;
      case 'h':
        usage(argv);
        exit(0);
      default:
        printf("Unknown option -%c\n", k);
        break;
    }
  }

  if (bind_to_cpu_set) {
    printf("Binding to cpu set...\n");
  } else {
    printf("Binding to particular core within cpu set...\n");
  }

  /* Printing useful information */
  printf("-------- Input data --------\n");
  printf("Number of cpus %d\n", num_cpus);
  printf("Array size %ld\n", array_size);
  printf("Linked list size %ld\n", llist_size);
  printf("Number of rows %d\n", nrow);
  printf("Number of columns %d\n", ncol);
  printf("Number of particles %d\n", num_particles);
  printf("Number of palindromes %d\n", num_palindromes);
  printf("Number of random locations %d\n", num_randomloc);
  printf("Number of iterations %d\n", num_iterations);
  printf("----------------------------\n");

  int orig_num_threads = num_threads;

  if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
    vals = malloc(sizeof(double) * array_size);  
    int_vals = malloc(sizeof(int) * array_size);
  }

  if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
      (!run_phases)) {
    src1 = malloc(sizeof(double) * array_size);
    src2 = malloc(sizeof(double) * array_size);
    dest = malloc(sizeof(double) * array_size);
    ind_src1 = malloc(sizeof(double) * array_size);
    ind_src2 = malloc(sizeof(double) * array_size);  
  }

  if ((run_phases & 32) || (!run_phases)) {
    sparse_matrix_indeces = malloc(sizeof(int*) * nrow);
    sparse_matrix_values = malloc(sizeof(double*) * nrow);
    sparse_matrix_nonzeros = malloc(sizeof(int) * nrow);
    vect_in = malloc(sizeof(double) * ncol);
    vect_out = malloc(sizeof(double) * nrow);   
  }

  if ((run_phases & 64) || (!run_phases)) {
    llist = malloc(num_cpus * sizeof(linked_list*));  
    for (i = 0; i < num_cpus; ++i) {
      llist[i] = malloc(sizeof(linked_list*));
    }
  }

  if ((run_phases & 128) || (!run_phases)) {
    particles = malloc(sizeof(particle) * num_particles);
    forces = calloc(num_particles-1, sizeof(double));
  }
  
  if ((run_phases & 256) || (!run_phases)) {
    palindromes = calloc(num_palindromes, sizeof(unsigned long));  
  }
  
  if ((run_phases & 512) || (!run_phases)) {
    randomloc = calloc(num_randomloc, sizeof(int));  
  }

/*
 * Validation
 * 
 * This is done in 2 ways: 
 * - reduced: as we produce the final values, store them in separate variables 
 *            and them compare those with the main arrays.
 *            Features: fast, but it only catches if threads write to other 
 *            threads' memory locations. 
 * - full: copy all the input data into validation arrays, and re-compute              
 *         serially. Validate by comparing main arrays with validation arrays.
 *         Features: slow, validates for serial consistency, as the full 
 *         validation is always run serially. 
 * 
 * We expect validation to run initially, when a new kernel is added or tested 
 * on a new platform, and not after that.
 */

#if FULL_VALIDATION
  double * restrict valid_vals = NULL, * restrict valid_src1 = NULL, 
         * restrict valid_src2 = NULL, * restrict valid_dest = NULL, 
         * restrict valid_vect_in = NULL, * restrict valid_vect_out = NULL,
         * restrict valid_forces = NULL;
  int * restrict valid_ind_src1 = NULL, * restrict valid_ind_src2 = NULL,
      * restrict valid_int_vals = NULL;
  int ** restrict valid_sparse_matrix_indeces = NULL;
  double ** restrict valid_sparse_matrix_values = NULL;
  int * restrict valid_sparse_matrix_nonzeros = NULL;
  unsigned long* restrict valid_palindromes = NULL;
#endif

#if RED_VALIDATION
  double* restrict valid_red_vals = malloc(sizeof(double) * array_size);
  int* restrict valid_red_int_vals = malloc(sizeof(int) * array_size);
  unsigned long* restrict valid_red_ulong_vals = malloc(sizeof(unsigned long) *
                                                        num_palindromes);
  double valid_red_reduction_var = 0;
#endif

#if FULL_VALIDATION
  if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
    valid_vals = malloc(sizeof(double) * array_size);
    valid_int_vals = malloc(sizeof(int) * array_size);
  }
  
  if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
      (!run_phases)) {
    valid_src1 = malloc(sizeof(double) * array_size);
    valid_src2 = malloc(sizeof(double) * array_size);
    valid_dest = malloc(sizeof(double) * array_size);
    valid_ind_src1 = malloc(sizeof(double) * array_size);
    valid_ind_src2 = malloc(sizeof(double) * array_size);
  }

  if ((run_phases & 32) || (!run_phases)) {
    valid_sparse_matrix_indeces = malloc(sizeof(int*) * nrow);
    valid_sparse_matrix_values = malloc(sizeof(double*) * nrow);
    valid_sparse_matrix_nonzeros = malloc(sizeof(int) * nrow);
    valid_vect_in = malloc(sizeof(double) * ncol);
    valid_vect_out = malloc(sizeof(double) * nrow);
  }

  if ((run_phases & 128) || (!run_phases)) {
    valid_forces = calloc(num_particles-1, sizeof(double));
  }

  if ((run_phases & 256) || (!run_phases)) {
    valid_palindromes = calloc(num_palindromes, sizeof(unsigned long));
  }
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  int retval = 0;
  unsigned long long event_values[num_hwcntrs];
  printf("Meabo.c num_hwcntrs %d\n", num_hwcntrs);
#endif

  /*
   * As the number of phases grew, we split the initialisation per phase
   * as much as possible, to try to minimise the initialisation time.
   *
   * For all initialisation, we use randomly-generated data. 
   */

  printf("Starting array initialisation...\n");
  if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
    for (i = 0; i < array_size; ++i) {
      vals[i] = (rand() % 1024) * cos(rand()%1024);
      int_vals[i] = ceil(vals[i]);
    }
  }

  if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
      (!run_phases)) {
    for (i = 0; i < array_size; ++i) {
      src1[i] = (rand() % 1024) * sin(rand()%1024);
      src2[i] = (rand() % 1024) * sin(rand()%1024);
      dest[i] = (rand() % 1024) * cos(rand()%1024);
      ind_src1[i] = rand() % array_size;
      ind_src2[i] = rand() % array_size;
    }
  }

  if ((run_phases & 32) || !run_phases) {
    printf("Starting sparse matrix initialisation...\n");
    for (i = 0; i < ncol; ++i) {
       vect_in[i] = (rand() % 1024) * sin(rand()%1024);
    }

    for (i = 0; i < nrow; ++i) {
      sparse_matrix_nonzeros[i] = rand() % ncol;
      const int nonzeros = sparse_matrix_nonzeros[i];
      sparse_matrix_indeces[i] = malloc(sizeof(int) * nonzeros);
      int * restrict cols = sparse_matrix_indeces[i];
      sparse_matrix_values[i] = malloc(sizeof(double) * nonzeros);
      double * restrict values = sparse_matrix_values[i];

      int* restrict columns = calloc((int)ceil(ncol/(sizeof(int)*8)),sizeof(int));
      for (j = 0; j < nonzeros; ++j) {
        col = rand() % ncol;
        while ((1 << (col % 32)) & *(columns + col/32)) {
          col = rand() % ncol;
        }
        cols[j] = col;
        *(columns + col/32) |= 1<<col;
        values[j] = (rand() % 1024) * cos(rand() % 1024);
      }
    }
  }

  if ((run_phases & 64) || !run_phases) {
    printf("Starting linked list initialisation...\n");
    for (i = 0; i < num_cpus; ++i) {
      linked_list *node = llist[i];
      for (j = 0; j < llist_size; ++j) {
        node->value = j;
        if (j == llist_size - 1) {
          node->next = NULL;
        } else {
          node->next = (linked_list*) malloc(sizeof(linked_list));
        }
        node = node->next;
      }
    }
  }

  if ((run_phases & 128) || !run_phases) {
    printf("Starting particles initialisation...\n");
    for (i = 0; i < num_particles; ++i) {
      particles[i].x = rand() % 1024;
      particles[i].y = rand() % 1024;
      particles[i].z = rand() % 1024;
      particles[i].charge = sin(rand()%1024);
    }
  }

#if FULL_VALIDATION
  if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
    for (i = 0; i < array_size; ++i) {
      valid_vals[i] = vals[i];
      valid_int_vals[i] = int_vals[i];
    }
  }

  if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
      (!run_phases)) {
    for (i = 0; i < array_size; ++i) {
      valid_src1[i] = src1[i];
      valid_src2[i] = src2[i];
      valid_ind_src1[i] = ind_src1[i];
      valid_ind_src2[i] = ind_src2[i];
      valid_dest[i] = dest[i];
    }
  }

 if ((run_phases & 32) || !run_phases) {
  for (i = 0; i < ncol; ++i) {
     valid_vect_in[i] = vect_in[i];
  }

  for (i = 0; i < nrow; ++i) {
    valid_sparse_matrix_nonzeros[i] = sparse_matrix_nonzeros[i];
    const int nonzeros = valid_sparse_matrix_nonzeros[i];
    valid_sparse_matrix_indeces[i] = malloc(sizeof(int) * nonzeros);
    int * restrict cols = valid_sparse_matrix_indeces[i];
    valid_sparse_matrix_values[i] = malloc(sizeof(double) * nonzeros);
    double * restrict values = valid_sparse_matrix_values[i];

    for (j = 0; j < nonzeros; ++j) {
      cols[j] = sparse_matrix_indeces[i][j];
      values[j] = sparse_matrix_values[i][j];
    }
  }
 }
#endif

#if ENABLE_BINDING
  printf("Per-phase core-binding enabled...\n");
#endif
#if ENABLE_PAPI
  printf("Enabling PAPI...\n");
  papi_info =  init_PAPI(num_hwcntrs);
#endif
#if ENABLE_ENERGY
  printf("Initializing energy readings...\n");
  init_read_energy();
#endif

  time(&rawtime);
  printf("All initialization completed at %s\n", ctime(&rawtime));

  // Phase 1: Floating-point and integer operations with good data re-use

if ((run_phases & 1) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 1... %s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase1_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase1_cpu_id, num_cpus, print);
#endif
#if ENABLE_ENERGY
   read_energy("#PHASE1_START");
#endif
#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    papi_info->num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase1_compute(num_iterations, array_size, block_size, temp1, temp2, temp3,
      int_temp1, int_temp2, int_temp3, vals, int_vals, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase1_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , valid_red_vals, valid_red_int_vals
#endif
    );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

#if ENABLE_ENERGY
   read_energy("#PHASE1_STOP");
#endif
#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif
  printf("Phase 1 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 1 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", array_size, vals, valid_red_vals, 1, rawtime);
  validation_array("Reduced", array_size, int_vals, valid_red_int_vals,
      1, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase1_compute_wrapper, 1, rawtime,
 #if RED_VALIDATION
       20,
 #else
       18,
 #endif
       num_iterations, array_size, block_size, temp1, temp2, temp3,
       int_temp1, int_temp2, int_temp3, valid_vals, valid_int_vals, 1,
       num_threads
#if ENABLE_BINDING
       , num_cpus, phase1_cpu_id, bind_to_cpu_set
#endif
 #if RED_VALIDATION
     , valid_red_vals, valid_red_int_vals
 #endif
     , vals, int_vals);
#endif
} else{
  printf("Skipping phase 1...\n");
}
  // Phase 2: Vector multiplication & addition, 1 level of indirection in 
  // 1 source vector
if ((run_phases & 2) || !run_phases) {

  num_threads = get_num_threads(phase2_cpu_id, num_cpus, orig_num_threads);

  time(&rawtime);
  printf("Starting phase 2...%s\n", ctime(&rawtime));
#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase2_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE2_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase2_compute(num_iterations, array_size, dest, src1, src2, ind_src2, 0,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase2_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
  );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE2_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif


  printf("Phase 2 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 2 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", array_size, dest, valid_red_vals, 2, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase2_compute_wrapper, 2, rawtime,
  #if RED_VALIDATION
        13,
  #else
        12,
  #endif
      num_iterations, array_size, valid_dest, valid_src1, valid_src2,
      valid_ind_src2, 1, num_threads
  #if ENABLE_BINDING
      , num_cpus, phase2_cpu_id, bind_to_cpu_set
  #endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      , dest
      );
#endif
} else {
  printf("Skipping phase 2...\n");
}

  // Phase 3: Vector scalar addition and reductions

if ((run_phases & 4) || !run_phases) {

  num_threads = get_num_threads(phase3_cpu_id, num_cpus, orig_num_threads);

  time(&rawtime);
  printf("Starting phase 3...%s\n", ctime(&rawtime));
#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase3_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE3_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase3_compute(num_iterations, array_size, vals, &reduction_var, 0,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase3_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
  #if RED_VALIDATION
      , &valid_red_reduction_var
  #endif
      );

  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE3_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif


  printf("Phase 3 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
    printf("Phase 3 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  reduced_validation_var(array_size, vals, valid_red_reduction_var, 3, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase3_compute_wrapper, 3, rawtime,
#if RED_VALIDATION
      11,
#else
      10,
#endif
      num_iterations, array_size, valid_vals, reduction_var, 1, num_threads
#if ENABLE_BINDING
      , num_cpus, phase3_cpu_id, bind_to_cpu_set
#endif
#if RED_VALIDATION
      , valid_red_reduction_var
#endif
      , vals);
#endif
} else {
  printf("Skipping phase 3...\n");
}

// Phase 4: Vector addition
if ((run_phases & 8) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 4...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase4_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase4_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE4_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase4_compute(num_iterations, array_size, dest, src1, src2, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase4_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE4_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values,
                                   num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif


  printf("Phase 4 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 4 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", array_size, dest, valid_red_vals, 4, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase4_compute_wrapper, 4, rawtime,
 #if RED_VALIDATION
       12,
 #else
       11,
 #endif
     num_iterations, array_size, valid_dest, valid_src1,
     valid_src2, 1, num_threads
#if ENABLE_BINDING
     , num_cpus, phase4_cpu_id, bind_to_cpu_set
#endif
 #if RED_VALIDATION
     , valid_red_vals
 #endif
     , dest
     );
#endif
} else {
  printf("Skipping phase 4...\n");
}
  // Phase 5: Vector addition, 1 level of indirection in both source vectors

if ((run_phases & 16) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 5...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase5_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase5_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
  read_energy("#PHASE5_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

  phase5_compute(num_iterations, array_size, dest, src1, src2, ind_src1,
      ind_src2, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase5_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );

  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE5_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif


  printf("Phase 5 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 5 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", array_size, dest, valid_red_vals, 5, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase5_compute_wrapper, 5, rawtime,
  #if RED_VALIDATION
        14,
  #else
        13,
  #endif
      num_iterations, array_size, valid_dest, valid_src1,
      valid_src2, valid_ind_src1, valid_ind_src2, 1, num_threads
  #if ENABLE_BINDING
      , num_cpus, phase5_cpu_id, bind_to_cpu_set
  #endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      , dest
      );
#endif
} else {
  printf("Skipping phase 5...\n");
}

// Phase 6: Sparse matrix-vector multiplication 

if ((run_phases & 32) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 6...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase6_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase6_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE6_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase6_compute(num_iterations, nrow, sparse_matrix_values, vect_in,
      sparse_matrix_indeces, sparse_matrix_nonzeros, vect_out, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase6_cpu_id, bind_to_cpu_set
#endif
#if ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE6_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif


  printf("Phase 6 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 6 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  int reallocation = 0;
  if (nrow > array_size) {
    printf("Reallocating valid_red_vals for reduced validation\n");
    realloc(valid_red_vals, sizeof(double) * nrow);
    reallocation = 1;
  }
  validation_array("Reduced", nrow, vect_out, valid_red_vals, 6, rawtime);
  if (reallocation) {
    printf("Reallocating back valid_red_vals after reduced validation\n");
    realloc(valid_red_vals, sizeof(double) * array_size);
  }

  printf ("Reduced validation for phase 6 ended at %s\n", ctime(&rawtime));
#endif

#if FULL_VALIDATION
  full_validation(phase6_compute_wrapper, 6, rawtime,
  #if RED_VALIDATION
        14,
  #else
        13,
  #endif
      num_iterations, nrow, valid_sparse_matrix_values,
      valid_vect_in, valid_sparse_matrix_indeces,
      valid_sparse_matrix_nonzeros, valid_vect_out, 1, num_threads
  #if ENABLE_BINDING
      , num_cpus, phase6_cpu_id, bind_to_cpu_set
  #endif
  #if RED_VALIDATION
      , valid_red_vals
  #endif
      , vect_out
      );
#endif
} else {
  printf("Skipping phase 6...\n");
}

// Phase 7: Linked-list traversal
if ((run_phases & 64) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 7...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase7_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase7_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE7_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase7_compute(num_iterations, llist_size, llist, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase7_cpu_id, bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , &valid_red_reduction_var
#endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE7_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif

  printf("Phase 7 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 7 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  reduced_validation_sg_var(valid_red_reduction_var, (double) (llist_size - 1), 7, rawtime);
#endif

#if FULL_VALIDATION
  printf("There is no full validation for phase 7, as we do not modify the "
      "linked list data.\n");
#endif
} else {
  printf("Skipping phase 7...\n");
}

// Phase 8: Electrostatic force calculations
if ((run_phases & 128) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 8...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase8_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase8_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE8_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase8_compute(num_iterations, num_particles, particles, forces, 0,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase8_cpu_id, bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , valid_red_vals
#endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE8_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif

  printf("Phase 8 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 8 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", num_particles - 1, forces, valid_red_vals, 8,
                    rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase8_compute_wrapper, 8, rawtime,
#if RED_VALIDATION
      11,
#else
      10,
#endif
      num_iterations, num_particles, particles, valid_forces, 1, num_threads
#if ENABLE_BINDING
      , num_cpus, phase8_cpu_id, bind_to_cpu_set
#endif
#if RED_VALIDATION
      , valid_red_vals
#endif
      , forces
      );
#endif
} else {
  printf("Skipping phase 8...\n");
}

// Phase 9: Palindrome caluclations
if ((run_phases & 256) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 9...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase9_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase9_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE9_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase9_compute(num_iterations, num_palindromes, palindromes, 0,
      num_threads
#if ENABLE_BINDING
      , num_cpus, phase9_cpu_id, bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , valid_red_ulong_vals
#endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE9_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif

  printf("Phase 9 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 9 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  validation_array("Reduced", num_palindromes, palindromes,
                    valid_red_ulong_vals, 9, rawtime);
#endif

#if FULL_VALIDATION
  full_validation(phase9_compute_wrapper, 9, rawtime,
#if RED_VALIDATION
      11,
#else
      10,
#endif
      num_iterations, num_palindromes, valid_palindromes, 1, num_threads
#if ENABLE_BINDING
      , num_cpus, phase9_cpu_id, bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , valid_red_ulong_vals
#endif
      , palindromes
      );
#endif
} else {
  printf("Skipping phase 9...\n");
}

// Phase 10: Random memory accesses
if ((run_phases & 512) || !run_phases) {

  time(&rawtime);
  printf("Starting phase 10...%s\n", ctime(&rawtime));

  num_threads = get_num_threads(phase10_cpu_id, num_cpus, orig_num_threads);

#if ENABLE_BINDING
  bind_to_cpu_w_reset(phase10_cpu_id, num_cpus, print);
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_start_counters(papi_info->event_code,
                                    num_hwcntrs)) != PAPI_OK) {
    printf("Failed to start counters %d: %s\n", retval, handle_error(retval));
  }
#endif

#if ENABLE_ENERGY
   read_energy("#PHASE10_START");
#endif
  clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
  phase10_compute(num_iterations, num_randomloc, randomloc, 0, num_threads
#if ENABLE_BINDING
      , num_cpus, phase10_cpu_id, bind_to_cpu_set
#endif
#if  ENABLE_PAPI && !(RED_VALIDATION || FULL_VALIDATION)
      , papi_info
#endif
#if RED_VALIDATION
      , valid_red_int_vals
#endif
      );
  clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
#if ENABLE_ENERGY
   read_energy("#PHASE10_STOP");
#endif

#if ENABLE_PAPI && (RED_VALIDATION || FULL_VALIDATION)
  if ((retval = PAPI_stop_counters(event_values, num_hwcntrs)) != PAPI_OK) {
    printf("Failed to stop counters %d: %s\n", retval, handle_error(retval));
  }
  for (i = 0; i < num_hwcntrs; ++i) {
    printf("%s value = %lld\n", papi_info->event_code_str[i], event_values[i]);
  }
#endif

  printf("Phase 10 duration (ns): %llu\n", duration(t1,t2));
  total_exec_time += duration(t1, t2);
  time(&rawtime);
  printf("Phase 10 completed at %s\n", ctime(&rawtime));

#if RED_VALIDATION
  /* No need for validation */
  printf("There is no reduced validation for phase 10, as we only perform random "
      "memory reads.\n");
#endif

#if FULL_VALIDATION
  /* No need for validation */
  printf("There is no full validation for phase 10, as we only perform random "
      "memory reads.\n");
#endif
} else {
  printf("Skipping phase 10...\n");
}

printf("Total execution time (ns): %llu\n", total_exec_time);

// Wrap-up & cleaning up

#if ENABLE_PAPI
  dinit_PAPI(papi_info);
#endif
#if ENABLE_ENERGY
  exit_read_energy();
#endif
  
if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
  free(vals);  
  free(int_vals);
}

if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
    (!run_phases)) {
  free(src1);
  free(src2);
  free(dest);
  free(ind_src1);
  free(ind_src2);
}

if ((run_phases & 32) || (!run_phases)) {
  for (i = 0; i < nrow; ++i) {
    free(sparse_matrix_indeces[i]);
    free(sparse_matrix_values[i]);
  }
  free(sparse_matrix_indeces);
  free(sparse_matrix_values);
  free(sparse_matrix_nonzeros);
  free(vect_in);
  free(vect_out);
}

if ((run_phases & 64) || (!run_phases)) {
  for (i = 0; i < num_cpus; ++i) {
    free(llist[i]);
  }
  free(llist);  
}

if ((run_phases & 128) || (!run_phases)) {
  free(particles);
  free(forces);
}

if ((run_phases & 256) || (!run_phases)) {
  free(palindromes);
}

if ((run_phases & 512) || (!run_phases)) {
  free(randomloc);  
}

#if RED_VALIDATION
  free(valid_red_vals);
  free(valid_red_int_vals);
  free(valid_red_ulong_vals);  
#endif

#if FULL_VALIDATION
if ((run_phases & 1) || (run_phases & 4) || (!run_phases)) {
  free(valid_vals);  
  free(valid_int_vals);
}

if ((run_phases & 2) || (run_phases & 8) || (run_phases & 16) || 
    (!run_phases)) {
  free(valid_src1);
  free(valid_src2);
  free(valid_dest);
  free(valid_ind_src1);
  free(valid_ind_src2);
}

if ((run_phases & 32) || (!run_phases)) {
  free(valid_sparse_matrix_indeces);
  free(valid_sparse_matrix_values);
  free(valid_sparse_matrix_nonzeros);
  free(valid_vect_in);
  free(valid_vect_out);
}

if ((run_phases & 128) || (!run_phases)) {
  free(valid_forces);
}

if ((run_phases & 256) || (!run_phases)) {
  free(valid_palindromes);
}
#endif

  time(&rawtime);
  printf("Program execution completed at %s\n", ctime(&rawtime));
}
