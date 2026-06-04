cmake_minimum_required(VERSION 3.5)

project(Autom3D VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE LEGACY)# Attempt to fix linuxdeployqt not adding libOpenGL.so.0 to appimage

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core Gui)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core Gui OpenGL OpenGLWidgets)
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo.rc")
set(PROJECT_SOURCES
    main.cpp
    lib/load.hpp lib/matrix.hpp
    lib/mix.h
    lib/mix.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    lib/graphicsView.h lib/graphicsView.cpp
    pic.qrc
    lib/light_contrast.h lib/light_contrast.cpp
    lib/Camera.h lib/Camera.cpp
    lib/Model.h lib/Model.cpp
    lib/OpenGLWidget1.h lib/OpenGLWidget1.cpp
    lib/OpenGLWidget2.h lib/OpenGLWidget2.cpp
    lib/Rectangle.h lib/Rectangle.cpp
    lib/Line.h lib/Line.cpp
    lib/Object.h lib/Object.cpp
    lib/Points.h lib/Points.cpp
    lib/drawSymbol.h lib/drawSymbol.cpp
    lib/showModel.h lib/showModel.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Autom3D

        MANUAL_FINALIZATION

        ${PROJECT_SOURCES}
        ${app_icon_resource_windows}
    )

# Define target properties for Android with Qt 6 as:

# set_property(TARGET Autom3D APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR

# ${CMAKE_CURRENT_SOURCE_DIR}/android)

# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Autom3D SHARED

            ${PROJECT_SOURCES}
        )

    # Define properties for Android with Qt 5 after find_package() calls as:

    # set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Autom3D

            ${PROJECT_SOURCES}
        )
    endif()
endif()

include_directories(${PROJECT_SOURCE})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


target_link_libraries(Autom3D PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets)

set_target_properties(Autom3D PROPERTIES

    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com

    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}

    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}

    MACOSX_BUNDLE TRUE

    WIN32_EXECUTABLE TRUE# Only commented out for debugging, remember to uncomment
)

install(TARGETS Autom3D

    BUNDLE DESTINATION .

    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Autom3D)
endif()

