#!/bin/bash
TMPDIR=$(basename $1)_build
mkdir $TMPDIR
#cd $TMPDIR
cp config.yaml config.yaml
clang -fpass-plugin="$(realpath build/llvm-pass/Lachesis.so)" -g -O0 -c $1 -o instrumented.o && \
clang++ instrumented.o -Lbuild/src/ -llachesis -o "$TMPDIR/instrumented"
for i in {0..5}
do
  ./$TMPDIR/instrumented
done

cd ..
rm -rf $TMPDIR