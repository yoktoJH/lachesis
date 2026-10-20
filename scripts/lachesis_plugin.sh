#!/usr/bin/env bash

CODE="$(realpath "$1")"
REPORT="$(realpath "$2")"

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"
rm -f instrumented.o instrumented

clang -fpass-plugin="Lachesis.so" -g -O0 -c $CODE -o instrumented.o && \
clang++ instrumented.o -llachesis -o "instrumented" || exit $?
cp "$REPORT" .

for i in {0..5}
do
  ./instrumented
done