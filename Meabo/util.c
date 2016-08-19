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

#include "util.h"

/*
 * This function binds a thread to the first available CPU from a set of CPUs. 
 * The print flag is used a control flag for all printf statements.
 */
void bind_to_available_cpu(int cpu_id, int num_cpus, int print, int skip) {
  if (!cpu_id && print) {
    printf("CPU_ID has no CPUs set. Skipping...\n");
    return;
  }
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);
  for (int i = 0; i < num_cpus; ++i) {
    if ((cpu_id & 0x00000001)) {
      if (!skip) {
        if (print) {
          printf("Setting cpu %d\n", i);
        }
        CPU_SET(i, &cpu_mask);
        break;
      } else {
        --skip;
      }
    }
    cpu_id >>= 1;
  }

  int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
  if (err == -1) {
    perror("sched_setaffinity");
  }
  assert(!err);
}

/*
 * This function binds a thread to a set of CPUs. The print flag is used as a
 * control flag for all printf statements.
 */
void bind_to_cpu(int cpu_id, int num_cpus, int print) {
  if (!cpu_id && print) {
    printf("CPU_ID has no CPUs set. Skipping...\n");
    return;
  }
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);
  for (int i = 0; i < num_cpus; ++i) {
    if (cpu_id & 0x00000001) {
      if (print) {
        printf("Setting cpu %d\n", i);
      }
      CPU_SET(i, &cpu_mask);
    }
    cpu_id >>= 1;
  }
  int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
  if (err == -1) {
    perror("sched_setaffinity");
  }
  assert(!err);
}

/*
 * This function binds a thread to a particular CPU.
 */
void bind_to_1_cpu(int cpu_id) {
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);
  CPU_SET(cpu_id, &cpu_mask);
  int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
  assert(!err);
}

/*
 * This function resets affinity (aka binds a thread to all available CPUs).
 * The print flag is used as a control flag for all printf statements.
 */
void reset_affinity(int num_cpus, int print) {
  if (print) {
    printf("Reseting affinity...\n");
  }
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);
  for (int i = 0; i < num_cpus; ++i) {
    CPU_SET(i, &cpu_mask);
  }
  int err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
    if (err == -1) {
      perror("sched_setaffinity");
    }
    assert(!err);
}

/*
 * This functions binds the thread to the first available CPU or all CPUs
 * (reset). The print flag controls whether printf statements are outputted.
 */
void bind_to_available_cpu_w_reset(int cpu_id, int num_cpus,
                                   int print, int skip) {
  if (cpu_id == -1) {
    reset_affinity(num_cpus, print);
  } else {
    bind_to_available_cpu(cpu_id, num_cpus, print, skip);
  }
}

/*
 * This function binds the thread to a set CPU or to all CPUs (reset). 
 * The print flag is used to as a control flag for all printf statements.
 */
void bind_to_cpu_w_reset(int cpu_id, int num_cpus, int print) {
  if (cpu_id == -1) {
    reset_affinity(num_cpus, print);
  } else {
    bind_to_cpu(cpu_id, num_cpus, print);
  }
}

/*
 * Duration, in nanoseconds.
 */
unsigned long long duration(struct timespec t1, struct timespec t2) {
  return ((t2.tv_sec * NANOS + t2.tv_nsec) -
         (t1.tv_sec * NANOS + t1.tv_nsec));
}

#if ENABLE_PAPI
/*
 * PAPI error-handling wrapper.
 */
char* handle_error (int retval) {
  return (PAPI_strerror(retval));
}

/*
 * PAPI initialisation function.
 * For now, we only support as many events as there are hardware counters.
 */
PAPI_info* init_PAPI(int num_hwcntrs) {
  printf("Init_PAPI start\n");

  unsigned int domain = PAPI_DOM_USER;
  int retval = 0, total_events = 0;
  PAPI_info *papi_info;
  papi_info = malloc(sizeof(PAPI_info));
  papi_info->event_code = calloc(0, sizeof(int) * num_hwcntrs);
  papi_info->event_code_str = malloc(sizeof(char*) * num_hwcntrs);
  int *event_code = papi_info->event_code;
  char **event_code_str = papi_info->event_code_str;

  printf("Enabling PAPI...\n");

   //Initialising PAPI
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
     printf("PAPI library init error %d: %s\n", retval, handle_error(retval));
     exit(1);
   }

  /* Suggested events for aarch64 */
/*  PAPI_event_name_to_code("CPU_CYCLES", &event_code[total_events++]);
  PAPI_event_name_to_code("INST_RETIRED", &event_code[total_events++]);
  PAPI_event_name_to_code("L1D_CACHE_REFILL", &event_code[total_events++]);
  PAPI_event_name_to_code("L1D_CACHE_ACCESS", &event_code[total_events++]);
  PAPI_event_name_to_code("L2D_CACHE_REFILL", &event_code[total_events++]);
  PAPI_event_name_to_code("L2D_CACHE_ACCESS", &event_code[total_events++]);
  PAPI_event_name_to_code("DATA_MEM_ACCESS", &event_code[total_events++]);*/

  /* Suggested events for x86_64 */
/*  PAPI_event_name_to_code("perf::CPU-CYCLES", &event_code[total_events++]);
  PAPI_event_name_to_code("INST_RETIRED", &event_code[total_events++]);
  PAPI_event_name_to_code("perf::L1-DCACHE-LOAD-MISSES", &event_code[total_events++]);
  PAPI_event_name_to_code("perf::L1-DCACHE-LOADS", &event_code[total_events++]);
  PAPI_event_name_to_code("perf::LLC-LOAD-MISSES", &event_code[total_events++]);
  PAPI_event_name_to_code("perf::LLC-LOADS", &event_code[total_events++]);
  PAPI_event_name_to_code("OFFCORE_REQUESTS:ALL_DATA_RD", &event_code[total_events]);*/

  if (total_events > num_hwcntrs) {
    total_events = num_hwcntrs;
  }
 
  papi_info->total_events = total_events;
  papi_info->num_hwcntrs = num_hwcntrs;

  for (int i = 0; i < total_events; ++i) {
    papi_info->event_code_str[i] = malloc(sizeof(char) * PAPI_MAX_STR_LEN);
  }

  PAPI_set_domain(domain);
  for (int i = 0; i < total_events; ++i) {
    PAPI_event_code_to_name(event_code[i], event_code_str[i]);
  }

  return papi_info;
}

/*
 * Free up PAPI-related memory variables.
 */
void dinit_PAPI(PAPI_info *papi_info) {
  for (int  i = 0; i < papi_info->total_events; ++i) {
    free(papi_info->event_code_str[i]);
  }
  //free(papi_info->event_code);
  //free(papi_info->event_code_str);
  free(papi_info);
}
#endif

/*
 * This function prints the usage information.
 */
void usage(char *argv[]) {
  printf("%s Meabo v1.0 [options]\n", argv[0]);
  printf("-s Array size\n");
  printf("-l Linked list size\n");
  printf("-r Number of rows\n");
  printf("-c Number of columns\n");
  printf("-i Number of iterations\n");  
  printf("-p Number of particles\n");
  printf("-x Number of palindromes\n");
  printf("-R Number of random locations\n");
  printf("-b Block size\n");
  printf("-C Number of CPUs\n");
  printf("-1 Phase1 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-2 Phase2 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-3 Phase3 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-4 Phase4 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-5 Phase5 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-6 Phase6 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-7 Phase7 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-8 Phase8 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-9 Phase9 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-0 Phase10 CPU affinity. -1 is reset. 0 has no effect.\n");
  printf("-H Number of hardware counters (including the cycle counter)\n");
  printf("-P Phases to run\n");
  printf("-T Total number of threads to run\n");
  printf("-B Bind to  cpu set versus specific cpu within cpu set.\n");
  printf("-h This menu\n");
}

/*
 * This function prints compiler information (compiler with which the code 
 * was built and compiler flags.
 */
void print_compiler_information(char * exec_name) {
    char cmd[256] = "";
    snprintf(cmd, sizeof(cmd), "objdump -s --section .comment %s", exec_name);

    FILE *cmd_pipe = popen(cmd, "r");
    if (cmd_pipe == NULL) {
      printf("Unable to open pipe\n");
    }

    char line[64], output[1024*16];
    char *tmp;
    int start_string = 0;
    while (fgets(line, 64, cmd_pipe) != NULL) {
      if (!start_string) {
          if (strstr(line, "0000") != NULL) {
          start_string = 1;
        } else {
          continue;
        }
      }
      tmp = (char*) (&(line[0]))+43;
      tmp[strlen(tmp)-1] = 0;
      strcat(output, tmp);
    }
    fclose(cmd_pipe);
    printf("Binary built with %s\n", output);

    cmd[0] = 0;
    output[0] = 0;

    snprintf(cmd, sizeof(cmd), "objdump -s --section .GCC.command.line %s", exec_name);
    cmd_pipe = popen(cmd, "r");
    if (cmd_pipe == NULL) {
      printf("Unable to open pipe\n");
    }

    start_string = 0;
     while (fgets(line, 64, cmd_pipe) != NULL) {
       if (!start_string) {
           if (strstr(line, "0000") != NULL) {
           start_string = 1;
         } else {
           continue;
         }
       }
       tmp = (char*) (&(line[0]))+43;
       tmp[strlen(tmp)-1] = 0;
       strcat(output, tmp);
     }
     fclose(cmd_pipe);
     printf("Binary built with %s\n", output);
}

/*
 * This function calculates how many threads the phase will be using.
 * For more information, see the explanation from meabo.c on num_threads.
 */
int get_num_threads(int phase_cpu_id, int num_cpus, int orig_num_threads) {
  if (orig_num_threads) {
    return orig_num_threads;
  }
  if (phase_cpu_id == -1) {
    return num_cpus;
  } else {
    int num_threads = 0;
    while (phase_cpu_id != 0) {
      if (phase_cpu_id & 1) {
        ++num_threads;
      }
      phase_cpu_id >>= 1;
    }
    return num_threads;
  }
}
