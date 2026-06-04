# Autom2 外部依赖列表

本文档列出了 Autom2 项目的所有外部依赖组件及其版本信息。

## 当前系统环境总结

| 依赖项 | 项目要求 | 当前系统版本 | 状态 |
|--------|----------|--------------|------|
| CMake | ≥3.12 (推荐) | 3.27.0 | ✓ 满足 |
| GCC/G++ | C++11 支持 | 11.4.0 | ✓ 满足 |
| OpenCV | 4.5.5 / 4.x | 4.5.5 | ✓ 满足 |
| MPI | 无要求 | Open MPI 4.1.5 | ✓ 满足 |
| CUDA | 11.7 | 12.6.77 | ⚠️ 版本不匹配 |
| Ceres | 无要求 | 未安装 | ✗ 缺失 |
| Eigen3 | 3.4.0 | 3.4.0 | ✓ 满足 |
| FFTW3 | 无要求 | 3.3.7 | ✓ 满足 |
| OpenMP | 无要求 | 4.5 (201511) | ✓ 满足 |
| pthread | NPTL | 2.35 | ✓ 满足 |

**系统信息**: Ubuntu 22.04, Linux 5.15.0-160-generic

## 目录
- [当前系统环境总结](#当前系统环境总结)
- [核心外部库](#核心外部库)
- [硬件和平台依赖](#硬件和平台依赖)
- [内部集成库](#内部集成库)
- [模块特定依赖](#模块特定依赖)

---

## 核心外部库

### 1. OpenCV
- **版本要求**: 4.5.5 (Markerauto2), 4.x (其他模块)
- **当前系统版本**: 4.5.5 ✓
- **用途**: 图像处理、计算机视觉算法
- **使用模块**:
  - Markerauto2 (要求精确版本 4.5.5)
  - markererase_opencv4
  - mrctools
  - resample
- **查找方式**: `find_package(OpenCV REQUIRED)`
- **安装路径参考**: `/usr/local/opencv45/`

### 2. Ceres Solver
- **版本要求**: 无具体版本要求
- **当前系统版本**: 未检测到 (需要安装)
- **用途**: 非线性优化
- **使用模块**: Markerauto2
- **查找方式**: `find_package(Ceres REQUIRED)`

### 3. MPI (Message Passing Interface)
- **版本要求**: 无具体版本要求
- **当前系统版本**: Open MPI 4.1.5 ✓
- **用途**: 并行计算、进程间通信
- **使用模块**:
  - TiltRec
  - resample
- **查找方式**: `find_package(MPI REQUIRED)`
- **编译选项**: 包含 C 和 C++ 接口

### 4. Eigen3
- **版本要求**:
  - 3.3.7 (ctfmeasure 内部集成)
  - 3.4.0 (resample 要求)
- **当前系统版本**: 3.4.0 ✓
- **用途**: 线性代数运算
- **使用模块**:
  - ctfmeasure (内部集成)
  - resample (外部依赖)
- **安装路径**: `$HOME/software/eigen-3.4.0` (需手动配置)

### 5. FFTW3
- **版本要求**: 无具体版本要求
- **当前系统版本**: 3.3.7 ✓
- **库组件**:
  - libfftw3f (单精度浮点版本)
  - libfftw3f_threads (多线程支持)
- **用途**: 快速傅里叶变换
- **使用模块**: ctfmeasure
- **查找方式**: 自定义路径查找 `3rdlib/lib` 或 `3rdlib/lib64`

### 6. NLopt
- **版本要求**: 2.6.2
- **用途**: 非线性优化库
- **使用模块**: ctfmeasure
- **集成方式**: 内部集成在 `external/nlopt-2.6.2`
- **可选特性**:
  - C++ 支持 (默认启用)
  - Python/Octave/Matlab 绑定 (可选)
  - SWIG 绑定支持

### 7. reproc/reproc++
- **版本要求**: 14.2.4
- **用途**: 跨平台子进程管理
- **使用模块**: 根项目 lib 目录
- **集成方式**: 内部集成
- **CMake 最低版本**: 3.12

---

## 硬件和平台依赖

### 1. CUDA Toolkit
- **版本要求**: 11.7
- **当前系统版本**: 12.6.77 ⚠️ (版本不匹配，但可能向后兼容)
- **项目要求路径**: `/usr/local/cuda-11.7/`
- **当前安装路径**: `/usr/local/cuda-12.6/`
- **用途**: GPU 加速计算
- **使用模块**:
  - TiltRec (TiltRec_cuda_z, TiltRec_cuda_y)
  - resample
- **架构支持**: 自动检测 GPU 架构 (CUDA_ARCH_LIST: "Auto")
- **编译选项**:
  - 优化级别: `-O3`
  - 扩展 Lambda: `--extended-lambda`
- **查找方式**: `find_package(CUDA REQUIRED)`

### 2. OpenMP
- **版本要求**: 无具体版本要求
- **当前系统版本**: OpenMP 4.5 (201511) ✓
- **编译器**: GCC 11.4.0
- **用途**: CPU 多线程并行计算
- **使用模块**: ctfmeasure
- **编译选项**: `-fopenmp`

---

## 内部集成库

以下库已集成在项目源码树中，无需外部安装：

### 1. ANN (Approximate Nearest Neighbor)
- **版本**: 1.2 (char 版本)
- **用途**: 近似最近邻搜索
- **路径**: `lib/ann_1.2_char`
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 2. CLAPACK
- **版本**: 3.1
- **用途**: C 语言线性代数库
- **路径**: `lib/clapack_3.1`
- **组件**: BLAS/WRAP, F2C
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 3. CBLAS
- **版本**: 自定义实现
- **用途**: C 接口的 BLAS
- **路径**: `lib/cblas`
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 4. SBA (Sparse Bundle Adjustment)
- **版本**: 1.6
- **用途**: 稀疏光束法平差
- **路径**: `lib/sba_1.6`
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 5. Levmar (Levenberg-Marquardt)
- **版本**: 2.5
- **用途**: 非线性最小二乘优化
- **路径**: `lib/levmar_2.5`
- **包含**: Matlab 接口
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 6. cminpack
- **版本**: 未指定
- **用途**: 非线性方程和最小二乘问题
- **路径**: `lib/cminpack`
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

### 7. matrix
- **版本**: 自定义实现
- **用途**: 矩阵运算
- **路径**: `lib/matrix`
- **使用模块**: Markerauto2, mrctools, markererase_opencv4

---

## 模块特定依赖

### Markerauto2
```cmake
- OpenCV 4.5.5 (精确版本)
- Ceres Solver
- pthread (-lpthread)
- 内部库: ANN, CLAPACK, CBLAS, SBA, Matrix, Levmar
```

### TiltRec
```cmake
- MPI (C/C++ 接口)
- CUDA 11.7
- 内部: filter, mrcmx, opts, util 模块
```

### resample
```cmake
- CUDA (CUDA 11 标准)
- MPI
- OpenCV
- Eigen3 3.4.0
- C++11 标准
```

### markererase_opencv4
```cmake
- OpenCV 4.x
- 内部库: ANN, CLAPACK, CBLAS, SBA, Matrix, Levmar
- C++11 标准
```

### mrctools
```cmake
- OpenCV (无版本限制)
- 内部库: ANN, CLAPACK, CBLAS, SBA, Matrix, Levmar
- C++11 标准
```

### ctfmeasure
```cmake
- FFTW3 (fftw3f, fftw3f_threads)
- NLopt 2.6.2 (内部集成)
- Eigen 3.3.7 (内部集成)
- OpenMP
- C++11 标准
```

### Markerfree
```cmake
- MPI (可选)
- 最低 CMake 版本: 3.1
```

---

## CMake 版本要求

不同模块对 CMake 的最低版本要求：
- 大部分模块: `2.6`
- Markerfree: `3.1`
- markererase_opencv4: `3.5`
- mrctools: `3.5`
- resample: `3.10`
- ctfmeasure: `3.10`
- reproc: `3.12`

**推荐 CMake 版本**: 3.12 或更高
**当前系统版本**: CMake 3.27.0 ✓

---

## 编译标准

- **C++ 标准**: C++11 (大部分模块)
- **CUDA 标准**: CUDA 11
- **当前编译器**: GCC/G++ 11.4.0 ✓
- **pthread**: NPTL 2.35 ✓

---

## 安装建议

### 系统包管理器安装
```bash
# Ubuntu/Debian
sudo apt-get install cmake libopencv-dev libceres-dev \
    libopenmpi-dev libfftw3-dev libeigen3-dev

# CUDA (需从 NVIDIA 官网下载)
# 安装 CUDA Toolkit 11.7
```

### 手动配置路径
某些依赖可能需要手动指定路径：
```bash
# Eigen3
export EIGEN3_INCLUDE_DIR=$HOME/software/eigen-3.4.0

# OpenCV (如果使用自定义安装)
export OpenCV_DIR=/usr/local/opencv45/include/opencv4
```

### CUDA 环境
确保 CUDA 安装路径正确：
```bash
export CUDA_HOME=/usr/local/cuda-11.7
export PATH=$CUDA_HOME/bin:$PATH
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
```

---

## 注意事项

1. **OpenCV 版本**: Markerauto2 严格要求 OpenCV 4.5.5，其他模块可以使用 OpenCV 4.x 系列
2. **⚠️ CUDA 版本不匹配**:
   - **项目要求**: CUDA 11.7
   - **当前系统**: CUDA 12.6.77
   - **影响模块**: TiltRec 和 resample
   - **建议**: CUDA 12.x 通常向后兼容 CUDA 11 代码，但建议测试。如遇编译或运行问题，需要安装 CUDA 11.7 或修改 CMakeLists.txt 中的 CUDA 路径配置
   - **解决方案**:
     - 选项1: 安装 CUDA 11.7 并配置环境变量指向正确版本
     - 选项2: 修改项目的 CUDA 配置适配 CUDA 12.x
     - 选项3: 使用 CUDA 多版本共存，编译时指定 CUDA 11.7 路径
3. **MPI 实现**: 当前系统使用 Open MPI 4.1.5，确保 C 和 C++ 接口都可用
4. **Ceres Solver**: 未检测到系统安装，Markerauto2 模块需要手动安装 Ceres
5. **内部库**: 项目包含多个内部库，通常不需要外部安装
6. **Eigen 版本**: ctfmeasure 使用内部集成的 3.3.7，resample 需要外部 3.4.0 (当前系统已安装)
7. **线程库**: 多个模块需要 pthread 支持，当前系统 NPTL 2.35 已满足要求

---

## 快速环境检查

使用以下命令检查当前系统环境：

```bash
# 检查编译工具链
cmake --version
gcc --version
g++ --version

# 检查外部库版本
pkg-config --modversion opencv4
pkg-config --modversion ceres
pkg-config --modversion eigen3
pkg-config --modversion fftw3f

# 检查并行库
mpirun --version
nvcc --version  # CUDA
echo | gcc -fopenmp -dM -E - | grep -i openmp  # OpenMP

# 检查 pthread
ldd --version
```

---

**文档生成时间**: 2026-01-21
**CMake 配置文件来源**: /home/xzh/autom2 根目录及所有子模块
**系统环境检测时间**: 2026-01-21
