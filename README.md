
# Introduction

## What it is

Meabo is a multi-phased multi-purpose micro-benchmark. It is a highly configurable tool which can be used for energy efficiency studies, ARM big.LITTLE Linux scheduler analysis and DVFS studies. It can be used for other benchmarking as well. 

The micro-benchmark is composed of 10 phases that perform various generic calculations (from memory to compute intensive). None of the compute kernels are optimized for a specific architecture. The micro-benchmark is easily extensible, provides performance counter readings (today) and energy readings (in the future), has multi-core and flexible pinning support, and allows the user to run each phase in different configurations.

One of the main benefits of this tool is that it is scalable beyond the aforementioned investigations, and is useful for anyone who would like to understand system behaviour, whilst running small, simple and well-understood computational kernels. The level of flexibility built into the application is an added benefit, as it gives the user full control of what is being run, where it is run, and whether it's single or multi-threaded, all within the same run of the application.

The micro-benchmark offers performance counters readings (available now) via PAPI and energy readings (available in the future) via a generic hwmon interface.


## Why/when use it

* energy efficiency, heterogeneous scheduler or DVFS studies and more!
* small, simple and well-understood computational kernels enable the user to focus on understanding system behaviour in a certain running scenario
* highly-configurable 
* easily-extensible

## Kernels

Currently, Meabo contains 10 computational kernels. This can be increased in the future. 

All floating-point calculations are double-precision.

Phase 1: Floating-point & integer computations with good data locality<br>
Phase 2: Vector multiplication & addition, 1 level of indirection in 1 source vector <br>
Phase 3: Vector scalar addition and reductions <br>
Phase 4: Vector addition <br>
Phase 5: Vector addition, 1 level of indirection in both source vectors <br>
Phase 6: Sparse matrix-vector multiplication <br>
Phase 7: Linked-list traversal <br>
Phase 8: Electrostatic force calculations <br>
Phase 9: Palindrome calculations <br>
Phase 10: Random memory accesses <br>

## How-Tos

### Changing the performance counters

In <code>util.c:init\_PAPI</code>, we list 7 counters which are common to both ARM Cortex-A57 and ARM Cortex-A53. You can get different counters by either running, for example, <code>papi\_native\_avail</code> or by looking at each core's Technical Reference Manual (TRM).

## Prerequisits

There are no pre-requisits for running Meabo. 

## 3rd party libraries

Meabo uses the following 3rd party libraries to extend its functionality:

* PAPI - used for performance counter readings
* hwloc (coming soon) - power counter readings

## Building

Meabo comes with a default set of Makefiles for a 3 target architectures: ARMv8, ARMv7 and x86\_64. 

**ARMv7**: Very limited testing has been done on ARMv7 platforms.  

You can also check the build status in [README-buildstatus.md](README-buildstatus.md).

If you wish to build it it with PAPI support, please set the PAPI\_INC and PAPI\_LIB variables appropriately in the Makefile. You will also need to set the #define ENABLE\_PAPI 1 in meabo.h. Also, please remember to uncomment the desired performance counters in util.c:init\_PAPI.

For now, this has been tested to compile with GCC. As is, you require a GCC that supports OpenMP 4.0 or above (e.g. gcc 5.1). If you have an older version of gcc, please comment out the "simd" construct from the OpenMP pragmas in compute_kernels.c.

This has been tested with gcc versions 5.1 and 6.1.1, and PAPI versions 5.4.0, 5.4.1 and 5.4.3.

To build/clean, just run: <code>make ARCH=<desired arch></code> or <code>make ARCH=<desired arch> clean</code>.

Current <code>ARCH</code> options: aarch64, aarch64\_debug, x86\_64, x86\_64\_debug, armv7, armv7\_debug. 

## Running

For detailed explanation of each running option, please run <code>./meabo.\$ARCH -h</code>.

For a default run, when you use the maximum number of threads available and each thread to a particular core, you can use the following command line: <code>./meabo.\$ARCH -B 1</code>.

If the benchmark is run on one core, using the default configuration variables for validation and sizes of structures, on an ARM Cortex-A53 clocked at 850MHz, the total runtime of the 10 phases and initialisation is expected to be around 30-40 minutes, whilst on an ARM Cortex-A57 clocked at 1100MHz, the total runtime is expected to be of around 10-20 minutes. 

If you are running this for the first time, I strongly recommend you enable both reduced and full validation to check there are no issues.

## Output


## More information

### Configuration variables

We wrote the micro-benchmark to be highly-configurable, but this translates into lots of configuration variables (and #defines). To aid the user, we include a README for the configuration variables (including in-source comments) that describes what they are used for, formats, the default value choices etc.
[README-configvars.md](README-configvars.md)

### Concepts

In this micro-benchmark, we use the concepts of *binding to core* versus *binding to core set*. *Binding to core* means binding to a specific core, within a core-set. This does not permit migration. *Binding to core set* means binding to all cores within that set. This permits migration within the cores of the core set. 

There is no association between core set and cluster (whether it is an ARM big.LITTLE cluster or any other sort). The definition of the core set is left to the user.

In the case of *binding to core*, thread binding happens from the lowest numbered core to the highest, in order. Example: if Phase 1 is bound to cores 4 and 5 and is run with 2 threads, thread 0 will be bound to core 4 and thread 1 to core 5.

# License 

This project is licensed under Apache-2.0.

This project includes some third-party code under other open source licenses. For more information, see Meabo/LICENSE.*

# Contributions / Pull Requests

Contributions are accepted under Apache-2.0. Only submit contributions where you have authored all of the code. If you do this on work time, make sure you have your employer's approval.


