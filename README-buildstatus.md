# Introduction

This document contains Meabo's build status and a list of known bugs.

# Build status


Test description | Phase |Binary | x86_64  | x86_64-debug | AArch64 | AArch64-debug | ARMv7 | ARMv7-debug |
-----------------|-------|-------|---------|--------------|---------|---------------|-------|-------------|
Default run		|  all	  |default| **PASS**| **PASS**     | **PASS**| **PASS**| Not yet tested| Not yet tested |
Default run		|  all   |reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS**| Not yet tested | Not yet tested |
Default run		| 	1     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	2     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	3     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	4     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	5     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	6     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	7     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	8     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	9     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Default run		| 	10     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	1     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	2     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		|  3     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	4     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	5     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	6     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	7     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	8     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	9     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Small run		| 	10     | reduced & full validation| **PASS** | **PASS** | **PASS** | **PASS** | Not yet tested | Not yet tested |
Large run		| 	all     | reduced & full validation| **PASS** | **PASS** | **Not run*** | **Not run*** | Not yet tested | Not yet tested |
Large run		| 	1     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	2     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	3     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	4     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	5     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	6     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		tested| 	7     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	8     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	9     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |
Large run		| 	10     | reduced & full validation| Not run | Not run | Not run | Not run | Not yet tested | Not yet tested |

* The large all-phases tests were not run on the AArch64 test board we have available due to insufficient RAM.

 

# Known bugs/quirks

* Some PAPI x86\_64 native counters don't work when using PAPI\_start\_counters.
	* Workaround: tests show that PAPI\_start calls work with those counters. 	
	* This has been tested using API version 5.4.0.

# Bug reports

In case you would like to submit bug reports, please remember to first have reproducible test cases. It's unlikely that I will be able to fix bugs which I cannot reproduce.