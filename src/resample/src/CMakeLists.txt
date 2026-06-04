add_subdirectory(mrcimg)
add_subdirectory(util)
add_subdirectory(main)




# 创建主可执行文件
add_executable(resample 
    main/main.cpp
)



# 链接库，确保链接顺序正确
target_link_libraries(resample
    main_lib
    mrcimg
    util
    CUDA::cudart
    CUDA::cufft
    ${MPI_LIBRARIES}
    ${OpenCV_LIBS}
)