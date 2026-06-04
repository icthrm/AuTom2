#!/bin/bash
set -e

IMAGE="${1:-autom2-dist}"
PROJ_DIR="$(pwd)"

docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
-v $(pwd)/daa:/workspace/data \
autom2-dist \
/workspace/autom2/dist/autom2                                     

 


