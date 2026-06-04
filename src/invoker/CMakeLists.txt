add_library(invoker STATIC
    procinvoker.cpp
    procinvoker.h
    mrcimageprovider.cpp
    mrcimageprovider.h
    svgimageprovider.cpp
    svgimageprovider.h
)

target_include_directories(invoker
    PUBLIC .
    PUBLIC ${CMAKE_SOURCE_DIR}/src/mrcimg)


    # 1. 明确告诉 invoker，reproc 的头文件在哪里
#    我们使用 CMAKE_SOURCE_DIR 这个全局变量来定位
# target_include_directories(invoker 
#     PUBLIC
#         .
#         "${CMAKE_SOURCE_DIR}/src/3rdparty/reproc/reproc/include/"
#         "${CMAKE_SOURCE_DIR}/src/3rdparty/reproc/reproc++/include/"
# )

# message(STATUS "${CMAKE_SOURCE_DIR}/src/3rdparty/reproc")


message(STATUS "${CMAKE_BINARY_DIR}/lib/3rdparty/reproc")

target_link_libraries(invoker PUBLIC
    Qt6::Core
    Qt6::Concurrent
    Qt6::Gui
    Qt6::Quick
    Qt6::Svg
    Qt6::Widgets
    reproc
    reproc++
    mrcimg
)


