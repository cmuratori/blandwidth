# Blandwidth
Blandwidth is a compact memory bandwidth tester for x64 CPUs running Windows.  It was developed during creation of the [Star Code Galaxy](https://starcodegalaxy.com) programming course because publicly available per-core bandwidth measurements seemed scarce.  It is difficult to tell students to learn about their processor's per-core memory bandwidth numbers when nobody seems to publish them!

Blandwidth is designed to determine the sustainable real bandwidth from the processor to L1, L2, L3, and main memory both during single thread and multiple thread workloads.  It is meant to produce numbers programmers can use to create realistic estimates of how much read, write, or read-write bandwidth their algorithms can expect for each specific processor and number of concurrent threads.

To build Blandwidth, install either Visual Studio or CLANG and run the included build.bat.

To run Blandwidth, run a release executable and pipe the output to the file where measurements should be stored, eg.: 

```
blandwidth_msvc_release.exe > intel_core_i9_test.csv
```
