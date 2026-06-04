
aux_source_directory(. FILTER_SRCS)

MESSAGE(STATUS "building filter")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

find_package(OpenCV REQUIRED)
find_package(CUDA QUIET REQUIRED)
include_directories(${PROJECT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-lcufftw -lcufft -lculibos)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS}; -lcufftw -lcufft -lculibos)
set(CUDA_ARCH_LIST "Auto" CACHE STRING "List of CUDA architectures (e.g. 52, 61, 75)")
include(FindCUDA/select_compute_arch)

CUDA_SELECT_NVCC_ARCH_FLAGS(NVCC_FLAGS_EXTRA ${CUDA_ARCH_LIST})
list(APPEND CUDA_NVCC_FLAGS ${NVCC_FLAGS_EXTRA})
cuda_add_library (filter ${FILTER_SRCS})

# 查找 cufftw 库（CUDA FFTW3 wrapper），避免硬编码 -L 路径
find_library(CUFFTW_LIBRARY NAMES cufftw HINTS ${CUDA_TOOLKIT_ROOT_DIR}/lib64 ${CUDA_TOOLKIT_ROOT_DIR}/lib)
if(NOT CUFFTW_LIBRARY)
    message(FATAL_ERROR "cufftw library not found. Please ensure CUDA toolkit is installed.")
endif()

target_link_libraries(filter ${OpenCV_LIBS} ${CUDA_LIBRARIES} ${CUFFTW_LIBRARY}) #需要系统能够找到opencv才可打开

set_target_properties(filter PROPERTIES OUTPUT_NAME "filter")
set_target_properties(filter PROPERTIES CLEAN_DIRECT_OUTPUT 1)
