# ============================================================
# autom2 构建镜像
# 基础: CUDA 12.2 + Ubuntu 22.04
# 用途: 在容器内编译并直接运行 autom2
# ============================================================

FROM nvidia/cuda:12.2.2-devel-ubuntu22.04

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
    qml6-module-qtquick-controls qml6-module-qtquick-layouts \
    qml6-module-qtqml-workerscript qml6-module-qt5compat-graphicaleffects \
    qml6-module-qtquick-dialogs qml6-module-qtquick-shapes \
    qml6-module-qt-labs-qmlmodels qml6-module-qtquick-templates \
    qml6-module-qtcore qml6-module-qtqml qml6-module-qtquick \
    qml6-module-qtquick-window \
    libopenmpi-dev openmpi-bin \
    libeigen3-dev libfftw3-dev \
    libopencv-dev libceres-dev libnlopt-dev \
    libgl1-mesa-dev libfontconfig1-dev libfreetype6-dev \
    libpng-dev libjpeg-dev libtiff-dev libwebp-dev \
    libharfbuzz-dev libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev libvulkan-dev libxkbcommon-dev \
    fonts-noto-cjk fonts-noto-color-emoji fonts-wqy-zenhei \
    python3 python3-pip python-is-python3 gnuplot patchelf \
    && rm -rf /var/lib/apt/lists/*

# cryo_EM/Autom3D 打开 MRC 文件时调用 python 运行 ncempy + numpy
RUN pip3 install --no-cache-dir cmake==3.27.9 ncempy numpy

# --------------------------------------------------------------
# 3. 拷贝源码并编译
# --------------------------------------------------------------
COPY . /workspace/autom2
WORKDIR /workspace/autom2

RUN rm -rf build build_docker \
    && cmake -B build -S . \
    && cmake --build build -j$(nproc)

# 默认入口
CMD ["bash"]
