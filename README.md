# Blandwidth r1
Blandwidth is a compact memory bandwidth tester for x64 CPUs running Windows or Linux.  It was developed during creation of the [Star Code Galaxy](https://starcodegalaxy.com) programming course because publicly available per-core bandwidth measurements seemed scarce.  It is difficult to tell students to learn about their processor's per-core memory bandwidth numbers when nobody seems to publish them!

Blandwidth is designed to determine the sustainable real bandwidth from the processor to L1, L2, L3, and main memory during single thread and multiple thread workloads.  It is meant to produce numbers programmers can use to create realistic estimates of how much read, write, or read-write bandwidth their algorithms can expect for each specific processor and number of concurrent threads.

# Building

## Windows

To build Blandwidth, install either Visual Studio or CLANG, cd to the blandwidth directory and run:

```
build.bat
```

Note that Blandwidth on Windows has no prerequisites and no dependencies other than the user32.lib and kernel32.lib import libraries.  It does not require a C runtime library of any kind, or any other library.

If your system supports llvm-mca, the build.bat file will run it and write into the build directory a cycle analysis of the memory routines.  This can be useful for ensuring that your version of the C compiler is generating efficient memory test routines.

## Linux

To build Blandwidth, install CLANG, cd to the blandwidth directory, and run:

``` bash
$ ./linux_build.sh
```

This produces debug and release builds as well as LLVM MCA data (if you installation includes `llvm-mca`) in the `build/` directory.

# Usage

To run Blandwidth, run a release executable and pipe the output to the file where measurements should be stored, eg.: 

```
build\blandwidth_release_msvc.exe > intel_core_i9_test.csv
```

On Linux, this would be:

``` bash
$ build/blandwidth_release > intel_core_i9_test.csv
```

During testing, it will write summary messages to stderr which you can use to track its progress.  Once complete, it will write a CSV to stdout with the statistics gathered for each test.

# Limitations

As of r1, Blandwidth does not test CPU core patterns, so it does not properly discover all interesting memory bandwidth differences.  As time permits, in future versions it would be nice to use core-locked threads and test different distribution patterns to see which patterns produce the best and worst bandwidth on NUMA architectures.
