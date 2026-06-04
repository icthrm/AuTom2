
MESSAGE(STATUS "building main")

add_executable(markererase main.cpp)

target_link_libraries(markererase mrcimg dataf inpaint)
