# 创建main库
add_library(main_lib
    main.cpp
)

# 设置包含目录
target_include_directories(main_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 链接依赖库
target_link_libraries(main_lib
    mrcimg
    util
    ${MPI_LIBRARIES}
)
