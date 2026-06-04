
MESSAGE(STATUS "building main")

add_executable(traj main.cpp)

target_link_libraries(traj util dataf trajectory mrcimg)
