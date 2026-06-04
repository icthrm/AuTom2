
aux_source_directory(. GPG_SRCS)

add_executable(GeoParGen ${GPG_SRCS})

include_directories(${OpenCV_INCLUDE_DIRS}) # Not needed for CMake >= 2.8.11
target_include_directories(GeoParGen PUBLIC .) # Not needed for CMake >= 2.8.11
target_link_libraries(GeoParGen ${OpenCV_LIBS})
target_link_libraries(GeoParGen mrcimg)
target_link_libraries(GeoParGen ${MPI_CXX_LIBRARIES})
