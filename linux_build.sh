#!/bin/bash

CFLAGS_C="-Wall -Werror -Wno-unused-function -Wno-unused-variable -march=native -pthread"

mkdir -p build
pushd build

echo "-----------------"
echo "Building debug..."
clang $CFLAGS_C -O0 -g ../linux_blandwidth.c -o blandwidth_debug

echo "-----------------"
echo "Building release..."
clang $CFLAGS_C -O3 -g ../linux_blandwidth.c -o blandwidth_release

echo "-----------------"
echo "Generating analysis..."
clang -O3 -DLLVM_MCA=1 $CFLAGS_C ../linux_blandwidth.c -mllvm -x86-asm-syntax=intel -S -o blandwidth_release.asm
llvm-mca blandwidth_release.asm > blandwidth_release.mca

popd
