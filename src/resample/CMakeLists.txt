cmake_minimum_required(VERSION 3.10)
project(resample LANGUAGES CXX CUDA)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置 CUDA 标准
set(CMAKE_CUDA_STANDARD 11)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# 查找 MPI 包
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

# 查找 CUDA 包
find_package(CUDAToolkit REQUIRED)

# 尝试查找OpenCV
find_package(OpenCV REQUIRED)

# 查找 Eigen3
find_package(Eigen3 3.4 REQUIRED)

# 包含头文件目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${MPI_INCLUDE_PATH}
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
)

# 添加子目录
add_subdirectory(src)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()

# 输出信息
message(STATUS "MPI_LIBRARIES: ${MPI_LIBRARIES}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")
message(STATUS "EIGEN3_INCLUDE_DIRS: ${EIGEN3_INCLUDE_DIRS}")
