#!/bin/bash
set -e

IMAGE="${1:-autom2-dist}"
PROJ_DIR="$(pwd)"

docker run --rm \
    -v "${PROJ_DIR}:/workspace/autom2" \
    -w /workspace/autom2 \
    "${IMAGE}" \
    bash -c "
      rm -rf build
      cmake -B build -S . 2>&1
      cmake --build build -j\$(nproc) 2>&1
      echo 'done'
    "

