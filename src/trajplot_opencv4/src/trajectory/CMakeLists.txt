
aux_source_directory(. TRAJECTORY_SRCS)
include_directories(${PROJECT_SOURCE_DIR})
add_library(trajectory STATIC ${TRAJECTORY_SRCS})

find_package(OpenCV REQUIRED)
target_link_libraries(trajectory ${OpenCV_LIBS} dataf)

set_target_properties(trajectory PROPERTIES OUTPUT_NAME "trajectory")
set_target_properties(trajectory PROPERTIES CLEAN_DIRECT_OUTPUT 1)
