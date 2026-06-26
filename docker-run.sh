#!/bin/bash
set -e

IMAGE="${1:-autom2-dist}"
PROJ_DIR="$(pwd)"

xhost +local:0

docker run --rm \
    --gpus all \
    -e DISPLAY="$DISPLAY" \
    -e QT_QPA_PLATFORM=xcb \
    -e QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt6/plugins/platforms \
    -e OMPI_ALLOW_RUN_AS_ROOT=1 \
    -e OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1 \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v "$(pwd)/data:/workspace/data" \
    -v "${PROJ_DIR}:/workspace/autom2" \
    -v "$HOME:/host" \
    -e HOST_HOME="/host" \
    "$IMAGE" \
    /workspace/autom2/build/bin/autom2

