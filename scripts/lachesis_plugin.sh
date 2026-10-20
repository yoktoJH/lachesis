#!/usr/bin/env bash

rm -f instrumented.o instrumented

clang -fpass-plugin="Lachesis.so" -g -O0 -c $1 -o instrumented.o && \
clang++ instrumented.o -llachesis -o "instrumented" || exit $?

for i in {0..5}
do
  ./instrumented
done