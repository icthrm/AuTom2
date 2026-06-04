#!/bin/bash
set -e

IMAGE="${1:-autom2-dist}"
PROJ_DIR="$(pwd)"

docker run --rm \
    -v "${PROJ_DIR}:/workspace/autom2" \
    -w /workspace/autom2 \
    "${IMAGE}" \
    bash -c "
      rm -rf build dist
      cmake -B build -S . 2>&1
      cmake --build build -j\$(nproc) 2>&1
      mkdir -p dist && cp build/bin/* dist/
      echo 'done'
    "

