cmake_minimum_required(VERSION 3.5)
project(FindOffset LANGUAGES CXX CUDA)

cmake_policy(SET CMP0146 NEW)

# ----------------------------------------------------------------
# 编译选项
# ----------------------------------------------------------------
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3")

# 目标 GPU 架构（与原项目保持一致）
set(CMAKE_CUDA_ARCHITECTURES 52 60 61 62 80 86)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)

# ----------------------------------------------------------------
# 依赖
# ----------------------------------------------------------------
find_package(CUDAToolkit REQUIRED)
find_package(MPI REQUIRED)

# 源码目录
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(
    ${SRC_DIR}
    ${MPI_INCLUDE_PATH}
    ${CUDAToolkit_INCLUDE_DIRS}
)

# ----------------------------------------------------------------
# mrc 静态库
# ----------------------------------------------------------------
file(GLOB MRC_SOURCES "${SRC_DIR}/mrc/*.cpp")
add_library(mrc STATIC ${MRC_SOURCES})
target_include_directories(mrc PRIVATE ${MPI_INCLUDE_PATH})
target_link_libraries(mrc ${MPI_LIBRARIES})

# ----------------------------------------------------------------
# method 静态库（含 CUDA 核函数）
# ----------------------------------------------------------------
file(GLOB METHOD_SOURCES
    "${SRC_DIR}/method/*.cpp"
    "${SRC_DIR}/method/*.cu"
    "${SRC_DIR}/method/*.h"
    "${SRC_DIR}/method/*.cuh"
)
add_library(method_library STATIC ${METHOD_SOURCES})
target_include_directories(method_library PRIVATE ${MPI_INCLUDE_PATH})
target_link_libraries(method_library CUDA::cufft ${MPI_LIBRARIES} mrc)

# ----------------------------------------------------------------
# FindOffset 可执行文件
# ----------------------------------------------------------------
add_executable(FindOffset ${SRC_DIR}/main/main_offset.cpp)
target_link_libraries(FindOffset
    CUDA::cufft
    CUDA::cudart
    ${MPI_LIBRARIES}
    method_library
    mrc
)
target_include_directories(FindOffset PRIVATE ${MPI_INCLUDE_PATH})
