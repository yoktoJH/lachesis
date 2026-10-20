#!/usr/bin/env bash

cmake -S . -B build -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build build
