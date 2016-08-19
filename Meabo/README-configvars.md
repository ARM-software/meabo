# Introduction

This document contains descriptions for the configuration variables used in Meabo.

# Configuration variables (#defines)

* ENABLE_PAPI
	* default: 1
 	* description: This variable controls the use of PAPI.
* ENABLE_ENERGY
	* default: 0 (as this is still a sub)
 	* description: This variable controls the use of energy reading mechanisms.
* ENABLE_BINDING
	* default: 1
 	* description: This variable controls whether any binding to cores or core sets will occur.
* FULL_VALIDATION
	* default: 0
	* description: This variable controls whether we perform the validation for serial consistency.
* RED_VALIDATION
	* default: 0 
	* description: This variable controls whether we perform the validation that checks if any threads write to other threads' results locations.

# Configuration variables (compile-time	 flags)

* OpenMP:
	* default: on
	* description: The micro-benchmark uses OpenMP for parallelism. This is enabled by passing the <code>-fopenmp</code> and <code>-lgomp</code> compilation and linking, respectively.

# Configuration variables (via command line)

* num_iterations
	* command line option: <code>-i X</code>
	* type: int
	* default: 1000
	* description: Number of iterations the core loop is executed for. This number can vary per phase, but the actual number of iterations is based on this. We vary this, as the core loop length can vary and thus some phases can take far too long assuming we keep the same number of iterations. The phases where it varies are: Phase 6 (actual number of iterations = num\_iterations/5) and Phase 9 (actual number of iterations = num\_iterations/10). 
* array_size
 	* command line option: <code>-s X</code>
	* type: long
	* default: 1048576
	* description: Size of arrays used in Phases 1, 2, 3, 4 and 5.
* llist_size
 	* command line option: <code>-l X</code>
	* type: long
	* default: 16777216
	* description: Size of the linked list available for each thread.
* nrow
	* command line option: <code>-r X</code>
	* type: int
	* default: 16384
	* description: Number of rows for the sparse matrix used in Phase 6.
* ncol
	* command line option: <code>-c X</code>
	* type: int
	* default: 16384
	* description: Number of columns for the spare matrix used in Phase 6.
* num_particles
 	* command line option: <code>-p X</code>
	* type: int
	* default: 1048576
 	* description: Number of particles used in Phase 8.
* num_palindromes
 	* command line option: <code>-x X</code>
	* type: int
	* default: 1024
	* description: Number of palindromes used in Phase 9.
* num_randomloc
 	* command line option: <code>-R X</code>
	* type: int
	* default: 2097152
	* description: Number of random memory locations accessed in Phase 10. 
* block_size
	* command line option: <code>-b X</code>
	* type: int
	* default: 8
	* description: Block size used in Phase 1.
* num_cpus
	* command line option: <code>-C X</code>
	* type: int 
	* default: 6
	* description: Number of total CPUs that the application can bind threads to.
* phaseX\_cpu\_id
	* command line option: <code>-1 X -2 Y -3 Z (up to) -0 A(for phase 10)</code>
	* type: int
	* default: -1
	* description: These variables control which cores each phase is run on.

			Format:	  
		   	- the bits corresponding to the CPUs on which the phase is to be run should be set to 1. 
		   	  The input is the decimal representation.
* bind\_to\_cpu\_set
	* command line option: <code>-B X</code>
	* type: int
	* default: 1
	* description: This variable controls whether threads will be bound to a core set, or each individual thread will be bound to a specific core within the core set. 
* run_phases
	* command line option: <code>-P X</code>
	* type: unsigned int
	* default: 0
	* description: This variable controls which phases to run.
	   	
	   		Format:
   			- 0: reset value, runs all phases (assumption: you always want to run at least 1 phase)
   			- set bits corresponding to the phase you want to run to 1 (input is the decimal representation)
* num_threads
	* command line option: <code>-T X</code>
	* type: int
	* default: 0
	* description: This variable controls how many threads the application will be using. 
	
			Format:
			- 0: if phaseX_cpu_id = -1 then num_threads = num_cpus
  			     else num_threads = number of cpus set in phaseX_cpu_id 
			- non 0: user inputted num_threads
			         This variable is set before every kernel function call by calling get_num_threads.
* num_hwcntrs
	* command line option: <code>-H X</code>
	* type: unsigned int
	* default: 7
	* description: Only available when using PAPI. This variable controls how many hardware counters PAPI will get access to. We are using 7 counters by default, as that is the number of counters of the ARM Cortex-A57 and ARM Cortex-A53, on which we developed this micro-benchmark. Each of these cores has 6 general purpose counters and one cycle counter.
