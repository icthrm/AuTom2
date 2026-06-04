# 添加util库
add_library(util STATIC
    util/util.cpp
)

target_include_directories(util PUBLIC
    ${CMAKE_SOURCE_DIR}/src/include
)

target_compile_options(util PRIVATE -O2)
target_link_libraries(util PUBLIC OpenMP::OpenMP_CXX)

# 添加core库
add_library(core STATIC
    core/CTF.cpp
    core/CTFBase.cpp
    core/CTFAlgo_v3.cpp
    core/mrc.cpp
)

target_include_directories(core PUBLIC
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/external/Eigen
)

target_compile_options(core PRIVATE -O2)
target_link_libraries(core PUBLIC
    OpenMP::OpenMP_CXX
    ${FFTW3F_LIBRARIES}
    ${FFTW3F_THREADS_LIB}
)

# 通用库链接设置
set(COMMON_LIBS
    core
    util
    pthread
    m
    ${FFTW3F_LIBRARIES}         # 链接 fftw3f
    ${FFTW3F_THREADS_LIB}       # 链接 fftw3f_threads
    NLopt::nlopt                # 链接 nlopt
    OpenMP::OpenMP_CXX
)

# 添加主程序 CTFMeasure
add_executable(CTFMeasure
    main/CTFMeasure.cpp
)

target_link_libraries(CTFMeasure
    ${COMMON_LIBS}
)

# 添加工具程序
add_executable(stack_to_list
    tool/stack_to_list.cpp
)

target_link_libraries(stack_to_list
    ${COMMON_LIBS}
)

add_executable(ctf_deconvolve
    tool/ctf_deconvolve.cpp
)

target_link_libraries(ctf_deconvolve
    ${COMMON_LIBS}
)

add_executable(ctfmeasure_to_ctffind4
    tool/ctfmeasure_to_ctffind4.cpp
)

target_link_libraries(ctfmeasure_to_ctffind4
    ${COMMON_LIBS}
)

add_executable(tomo_tilt_offset
    tool/tomo_tilt_offset.cpp
)

target_link_libraries(tomo_tilt_offset
    ${COMMON_LIBS}
)
