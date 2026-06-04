# ============================================================
# autom2 构建+分发镜像
# 基础: CUDA 12.6 + Ubuntu 22.04
# 输出: /workspace/autom2/dist/ 目录，内含可执行文件+全部动态库
#       直接拷 dist/ 到任意同架构 Linux 即可运行，无需额外依赖
# ============================================================

FROM nvidia/cuda:12.6.0-devel-ubuntu22.04

LABEL maintainer="autom2-dev"
LABEL description="autom2 build environment with self-contained binary packaging"

# --------------------------------------------------------------
# 1. 基础设置
# --------------------------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

ENV CUDA_HOME=/usr/local/cuda
ENV PATH=${CUDA_HOME}/bin:${PATH}
ENV LD_LIBRARY_PATH=${CUDA_HOME}/lib64:${LD_LIBRARY_PATH}
ENV CPATH=/usr/lib/x86_64-linux-gnu/openmpi/include:${CPATH}

# --------------------------------------------------------------
# 2. 安装依赖
# --------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git wget ca-certificates pkg-config \
    qt6-base-dev qt6-declarative-dev qt6-tools-dev \
    libqt6svg6-dev libqt6opengl6-dev \
    libqt6shadertools6-dev qt6-shader-baker \
    libopenmpi-dev openmpi-bin \
    libeigen3-dev libfftw3-dev \
    libopencv-dev libceres-dev libnlopt-dev \
    libgl1-mesa-dev libfontconfig1-dev libfreetype6-dev \
    libpng-dev libjpeg-dev libtiff-dev libwebp-dev \
    libharfbuzz-dev libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev libvulkan-dev libxkbcommon-dev \
    python3 python3-pip patchelf \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install --no-cache-dir cmake==3.27.9

# --------------------------------------------------------------
# 3. 拷贝源码并编译
# --------------------------------------------------------------
COPY . /workspace/autom2
WORKDIR /workspace/autom2

RUN rm -rf build build_docker \
    && cmake -B build -S . \
    && cmake --build build -j$(nproc)

# --------------------------------------------------------------
# 4. 打包：收集动态库 + 修改 RPATH
# --------------------------------------------------------------
RUN mkdir -p dist \
    && cp -r build/bin/* dist/ \
    && for f in dist/*; do \
           [ -x "$f" ] || continue; \
           ldd "$f" 2>/dev/null | grep '=>' | awk '{print $3}' | while read libpath; do \
               [ -f "$libpath" ] || continue; \
               case "$(basename "$libpath")" in \
                   ld-linux*|libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*) continue ;; \
               esac; \
               cp -n "$libpath" dist/ 2>/dev/null || true; \
           done; \
       done \
    && for f in dist/*; do \
           [ -x "$f" ] || continue; \
           patchelf --set-rpath '$ORIGIN' "$f" 2>/dev/null || true; \
       done \
    && echo "=== 处理间接依赖 ===" \
    && for f in dist/*; do \
           [ -x "$f" ] || continue; \
           ldd "$f" 2>/dev/null | grep 'not found' | awk '{print $1}' | while read lib; do \
               libpath=$(find /lib /usr/lib -name "$lib" 2>/dev/null | head -1); \
               if [ -f "$libpath" ]; then cp -n "$libpath" dist/ 2>/dev/null; fi; \
           done; \
       done

# --------------------------------------------------------------
# 5. 验证
# --------------------------------------------------------------
RUN echo "=== 验证 autom2 是否能裸运行 ===" \
    && ldd dist/autom2 | grep -E 'not found' && echo "有缺失库！" || echo "全部找到，可以裸运行" \
    && echo "=== 输出目录 ===" \
    && ls -lh dist/ | head -20

# 默认入口：显示打包好的目录内容
CMD ["ls", "-lh", "/workspace/autom2/dist"]
