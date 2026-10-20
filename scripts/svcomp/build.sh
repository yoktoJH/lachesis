#!/usr/bin/env bash

set -euo pipefail

docker build -t lachesis_builder:latest .
docker rm -f lachesis_builder 2>/dev/null || true
docker run --name lachesis_builder lachesis_builder:latest
rm -rf build
mkdir build
docker cp lachesis_builder:/lachesis/build/llvm-pass/Lachesis.so build/
docker cp lachesis_builder:/lachesis/build/src/liblachesis.a build/
