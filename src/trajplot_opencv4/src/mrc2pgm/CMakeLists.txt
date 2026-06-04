include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(mrc2pgm main.cpp)

#find_package(OpenCV REQUIRED)

target_link_libraries(mrc2pgm mrcimg opencv_core opencv_imgproc opencv_calib3d opencv_video opencv_features2d 
	opencv_ml opencv_highgui opencv_objdetect opencv_contrib opencv_legacy) #${OpenCV_LIBS})
