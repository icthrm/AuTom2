cmake_minimum_required(VERSION 3.10)
project(CTFMeasure VERSION 1.4.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 1. 查找 OpenMP
find_package(OpenMP REQUIRED)

# 2. 查找 FFTW (通过 PkgConfig 查找主库，直接查找线程库)
# FFTW 版本要求：>= 3.3.0 (推荐 3.3.7 及以上)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3F REQUIRED fftw3f>=3.3.0)   # 查找 libfftw3f，要求版本 >= 3.3.0

# 验证 FFTW 版本
if(FFTW3F_VERSION)
    message(STATUS "Found FFTW3F version: ${FFTW3F_VERSION}")
    if(FFTW3F_VERSION VERSION_LESS "3.3.0")
        message(FATAL_ERROR "FFTW3F version ${FFTW3F_VERSION} is too old. Required: >= 3.3.0")
    endif()
endif()

# FFTW3F 线程库通常没有 pkg-config 文件，直接查找
find_library(FFTW3F_THREADS_LIB fftw3f_threads REQUIRED)
if(NOT FFTW3F_THREADS_LIB)
    message(FATAL_ERROR "FFTW3F threads library not found")
endif()
message(STATUS "Found FFTW3F threads library: ${FFTW3F_THREADS_LIB}")

# 3. 查找 NLopt (通过 Config 模式)
# NLopt 版本要求：>= 2.4.0 (推荐 2.6.0 及以上)
find_package(NLopt 2.4.0 REQUIRED CONFIG)

# 验证 NLopt 版本
if(NLopt_VERSION)
    message(STATUS "Found NLopt version: ${NLopt_VERSION}")
endif()

# 4. 设置头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src/include
    ${PROJECT_SOURCE_DIR}/external/Eigen
    ${FFTW3F_INCLUDE_DIRS}
    # NLopt 会自动包含路径，不需要手动写
)

# 5. 添加子目录
add_subdirectory(src)
