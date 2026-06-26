# CMakeLists.txt 硬编码路径改造计划

## Context

本项目（autom2）是一个多模块的科学计算/Qt GUI应用，包含大量子模块（Markerauto2、mrctools、TiltRec、Markerfree、resample、markererase_opencv4、ctfmeasure、traj、GeoParGen、geom、cryo_EM等）。当前CMakeLists.txt中大量硬编码了依赖库的安装路径（OpenCV、CUDA、Eigen3等），导致项目只能在特定开发环境中构建，无法方便地打包为Docker镜像或在其他机器上构建。

本次修改的目标：将所有硬编码的依赖路径改为CMake标准`find_package`查找方式，使构建系统具备通用性，为后续Docker镜像打包奠定基础。

**约束**：只修改硬编码路径相关逻辑，不改动其他任何构建逻辑。

---

## 硬编码路径汇总与修改方案

### 一、OpenCV_DIR 硬编码（13个文件）

当前大量CMakeLists.txt中硬编码了`set(OpenCV_DIR ...)`，指向特定机器上的OpenCV安装位置。修改方案：**删除所有`set(OpenCV_DIR ...)`行**，保留`find_package(OpenCV ...)`。用户构建时可通过`-DOpenCV_DIR=...`传入，或在Docker中将OpenCV安装到标准位置让`find_package`自动查找。

| 文件 | 当前硬编码内容 | 修改方式 |
|------|--------------|----------|
| `src/mrcimg/CMakeLists.txt:27` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/Markerauto2/CMakeLists.txt:17` | `set(OpenCV_DIR "usr/local/opencv45/include/opencv4")` | 删除该行 |
| `src/Markerauto2/src/mrcimg/CMakeLists.txt:9` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/Markerauto2/src/ransac/CMakeLists.txt:8` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/traj/CMakeLists.txt:17` | `set(OpenCV_DIR "usr/local/opencv45/include/opencv4")` | 删除该行 |
| `src/traj/src/mrcimg/CMakeLists.txt:27` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/trajplot_opencv4/CMakeLists.txt:17` | `set(OpenCV_DIR "usr/local/opencv45/include/opencv4")` | 删除该行 |
| `src/trajplot_opencv4/src/mrcimg/CMakeLists.txt:27` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/traj.bak/CMakeLists.txt:17` | `set(OpenCV_DIR "usr/local/opencv45/include/opencv4")` | 删除该行 |
| `src/traj.bak/src/mrcimg/CMakeLists.txt:27` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/toolsx/traj/CMakeLists.txt:17` | `set(OpenCV_DIR "usr/local/opencv45/include/opencv4")` | 删除该行 |
| `src/toolsx/traj/src/mrcimg/CMakeLists.txt:27` | `set(OpenCV_DIR /home/xzh-vm/opencv-4.5.5/build)` | 删除该行 |
| `src/TiltRec/src/filter/CMakeLists.txt:8` | `set(OpenCV_DIR "/usr/local/lib64/cmake/opencv4")` | 删除该行，同时删除第13-14行的`set(OpenCV_INCLUDE_PATH ...)`和对应的`include_directories(${OpenCV_INCLUDE_PATH})`，保留`include_directories(${PROJECT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})` |

> **注意**：`src/resample/CMakeLists.txt:26`有一行注释掉的`set(OpenCV_DIR /home/yucy_gpu/software/opencv/lib64/cmake/opencv4)`，一并删除。

---

### 二、CUDA 硬编码 include 路径（5个文件）

当前多处直接硬编码了CUDA头文件路径（如`/usr/local/cuda-11.1/targets/x86_64-linux/include`）。这些文件已经使用了`find_package(CUDA)`或`find_package(CUDAToolkit)`，查找模块已经提供了对应的变量，硬编码路径是多余的。

| 文件 | 当前硬编码内容 | 修改方式 |
|------|--------------|----------|
| `src/geom/CMakeLists.txt:32` | `include_directories(... /usr/local/cuda-11.1/targets/x86_64-linux/include)` | 替换为 `${CUDAToolkit_INCLUDE_DIRS}`（该文件已使用`find_package(CUDAToolkit REQUIRED)`） |
| `src/geom/src/method/CMakeLists.txt:21` | `include_directories(/usr/local/cuda-11.1/targets/x86_64-linux/include)` | 替换为 `include_directories(${CUDA_INCLUDE_DIRS})`（该文件使用`find_package(CUDA)`） |
| `src/Markerfree/src/method/CMakeLists.txt:21` | `include_directories(/usr/local/cuda-11.1/targets/x86_64-linux/include)` | 同上 |
| `src/TiltRec/src/TiltRec_cuda_y/CMakeLists.txt:8` | `include_directories(/usr/local/cuda-11.1/targets/x86_64-linux/include)` | 同上 |
| `src/TiltRec/src/TiltRec_cuda_z/CMakeLists.txt:6` | `include_directories(/usr/local/cuda-11.7/targets/x86_64-linux/include)` | 同上 |

---

### 三、Eigen3 硬编码路径（1个文件）

| 文件 | 当前内容 | 修改方式 |
|------|----------|----------|
| `src/resample/CMakeLists.txt:30` | `set(EIGEN3_INCLUDE_DIR "$ENV{HOME}/software/eigen-3.4.0")` | 删除该行，添加`find_package(Eigen3 3.4 REQUIRED)` |
| `src/resample/CMakeLists.txt:37` | `include_directories(... ${EIGEN3_INCLUDE_DIR})` | 将 `${EIGEN3_INCLUDE_DIR}` 改为 `${EIGEN3_INCLUDE_DIRS}` |

---

### 四、旧版 CUDA 查找改为 CUDAToolkit（1个文件）

`src/resample/CMakeLists.txt`使用了`find_package(CUDA REQUIRED)`，但该模块及其子目录**没有使用**`cuda_add_library`/`cuda_add_executable`等FindCUDA特有的宏，仅使用了`${CUDA_LIBRARIES}`和`${CUDA_CUFFT_LIBRARIES}`变量。可以安全地迁移到现代的`CUDAToolkit`模块。

| 文件 | 当前内容 | 修改方式 |
|------|----------|----------|
| `src/resample/CMakeLists.txt:22` | `find_package(CUDA REQUIRED)` | 改为 `find_package(CUDAToolkit REQUIRED)` |
| `src/resample/CMakeLists.txt:23` | `include_directories(${CUDA_INCLUDE_DIRS})` | 删除该行（CUDAToolkit不需要显式include） |
| `src/resample/src/CMakeLists.txt:20` | `${CUDA_LIBRARIES}` | 改为 `CUDA::cudart` |
| `src/resample/src/CMakeLists.txt:21` | `${CUDA_CUFFT_LIBRARIES}` | 改为 `CUDA::cufft` |

---

### 五、硬编码库链接路径（1个文件）

| 文件 | 当前内容 | 修改方式 |
|------|----------|----------|
| `src/TiltRec/src/filter/CMakeLists.txt:26` | `target_link_libraries(filter ${OpenCV_LIBS} ${CUDA_LIBRARIES} -L/usr/local/cuda/lib64 -lcufftw)` | 删除`-L/usr/local/cuda/lib64`（`find_package(CUDA)`已通过`${CUDA_LIBRARIES}`提供库搜索路径），保留`-lcufftw` |

---

### 六、注释掉的硬编码库路径（清理，1个文件）

| 文件 | 当前内容 | 修改方式 |
|------|----------|----------|
| `src/mrcimg/CMakeLists.txt:34` | 注释掉的`target_link_libraries(mrcimg /usr/local/opencv45/lib/libopencv_*.so ...)` | 删除该注释行 |

---

## 验证方案

1. **本地验证**：在修改完成后，运行`cmake -B build`检查是否能成功配置（不报错）。
2. **Docker验证**：编写一个基础的Dockerfile，安装所有依赖（Qt6、OpenCV 4.5+、CUDA toolkit、Eigen3、MPI、FFTW3、NLopt、Ceres等），然后运行`cmake -B build && cmake --build build`，验证项目能在干净环境中构建。
3. **关键检查点**：
   - OpenCV是否正确找到（检查`${OpenCV_LIBS}`输出）
   - CUDA是否正确找到（检查`CUDA::cudart`、`CUDA::cufft`目标可用）
   - Eigen3是否正确找到（检查`${EIGEN3_INCLUDE_DIRS}`）
   - MPI是否正确找到（检查`${MPI_LIBRARIES}`）

---

## Docker依赖清单（供后续Dockerfile编写参考）

根据CMakeLists中的`find_package`调用，本项目需要的系统依赖包括：
- Qt6 (Core, Gui, Qml, Quick, Concurrent, Svg, Widgets, OpenGL, OpenGLWidgets)
- OpenCV 4.5.5+
- CUDA Toolkit
- Eigen3 3.4+
- MPI (OpenMPI或MPICH)
- FFTW3 (含fftw3f_threads)
- NLopt 2.4+
- Ceres Solver
- OpenMP
- reproc ( vendored in `lib/reproc/` )

这些依赖在Ubuntu上可通过`apt`安装大部分，OpenCV和CUDA可能需要单独安装或从源码编译。
