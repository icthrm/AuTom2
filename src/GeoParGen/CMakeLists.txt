cmake_minimum_required(VERSION 3.2)

project(GeoParGen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# 添加编译调试选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(OpenCV REQUIRED)
# 查找MPICH库

message(STATUS "finding MPI ... ")

find_package(MPI REQUIRED)

# 如果找到MPI库,区分OPENMPI和MPICH
if(MPI_FOUND)
    if(MPI_C_COMPILER MATCHES "mpicc" AND MPI_CXX_COMPILER MATCHES "mpicxx")
        message(STATUS "找到MPICH库")
        include_directories(${MPI_INCLUDE_PATH})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
    elseif(MPI_C_COMPILER MATCHES "mpicc" AND MPI_CXX_COMPILER MATCHES "mpic\\+\\+")
        message(STATUS "找到OPENMPI库")
        include_directories(${MPI_INCLUDE_PATH})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
    elseif(MPI_C_COMPILER MATCHES "mpigcc" AND MPI_CXX_COMPILER MATCHES "mpigxx")
        message(STATUS "找到MPICH库")
        include_directories(${MPI_INCLUDE_PATH})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
    else()
        message(FATAL_ERROR "未找到支持的MPI库,请确保已正确安装MPICH或OPENMPI")
    endif()
else()
    message(FATAL_ERROR "未找到MPI库,请确保已正确安装")
endif()

add_subdirectory(lib)
add_subdirectory(src)



