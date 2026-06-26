cmake_minimum_required(VERSION 3.16)
project(autom2 VERSION 1.0 LANGUAGES CXX)

include(ExternalProject)

# --- 基本设置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON) # 虽然用不上，但留着无害

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(BUILD_SHARED_LIBS OFF)
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# --- 查找 Qt ---
# 需要 Qt6 的 Core, Gui, Qml, Quick 模块
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Concurrent Svg Widgets)

message(STATUS ${Qt6_DIR})

# --- 资源文件 ---
# qt_add_resources(RESOURCES "qmlapp/Autom2qml.qrc")

# --- 国际化 (可选，但推荐) ---
# qt_create_translation(TRANSLATION_FILES src/MyFirstQmlApp.ts)

# --- 定义可执行文件 ---
qt_add_executable(autom2 
    main/main.cpp
    ${RESOURCES}
    #    ${TRANSLATION_FILES}
)

# --- 设置目标属性和链接库 ---
set_target_properties(autom2 PROPERTIES
    WIN32_EXECUTABLE TRUE # 在 Windows 上创建无控制台窗口的应用
    MACOSX_BUNDLE TRUE     # 在 macOS 上创建 .app 包
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 链接 Qt 库
target_link_libraries(autom2 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Widgets
)

option(BUILD_QDS_COMPONENTS "Build QDS Components" ON)

add_subdirectory(qmlapp/Autom2qml)
add_subdirectory(qmlapp/Autom2qmlContent)
add_subdirectory(qmlapp/Dependencies)

if(BUILD_QDS_COMPONENTS)
    target_compile_definitions(autom2 PRIVATE BUILD_QDS_COMPONENTS=1)
endif()

target_link_libraries(autom2 PRIVATE
    -Wl,--whole-archive
    Autom2qml
    Autom2qmlContent
    Autom2qmlplugin
    Autom2qmlContentplugin
    QuickStudioApplicationplugin
    QuickStudioComponentsplugin
    QuickStudioEffectsplugin
    QuickStudioLogicHelperplugin
    QuickStudioEventSimulatorplugin
    QuickStudioEventSystemplugin
    -Wl,--no-whole-archive
    )




# --- 链接子模块invoker

add_subdirectory(mrcimg)
add_subdirectory(invoker)
target_link_libraries(autom2 PRIVATE invoker)

# --- 生成模块程序Markerauto2 

ExternalProject_Add(
    Markerauto2 # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Markerauto2

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/MARKERAUTO2

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)



# --- 生成模块程序cryo 

add_subdirectory(cryo_EM )

# --- 生成模块程序mrctools

ExternalProject_Add(
    mrctools # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mrctools

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/MRCTOOLS

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

# --- 生成模块程序TiltRec

ExternalProject_Add(
    TiltRec # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TiltRec

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/TILTREC

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

ExternalProject_Add(
    Markerfree # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Markerfree

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/MARKERFREE

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

ExternalProject_Add(
    Resample # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resample

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/RESAMPLE

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)


ExternalProject_Add(
    markererase_opencv4 # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/markererase_opencv4

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/MARKERERASE

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)


ExternalProject_Add(
    ctfmeasure # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ctfmeasure

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/CTFMEASURE

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)


ExternalProject_Add(
    traj # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/traj

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/TRAJ

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

ExternalProject_Add(
    GeoParGen # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/GeoParGen

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/GEOPARGEN

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

ExternalProject_Add(
    Geom # 为这个外部项目构建过程起一个唯一的名字

    # 指定B的源代码目录
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/geom

    # 指定B的构建目录，这会是 build/B_build/
    BINARY_DIR ${CMAKE_BINARY_DIR}/GEOM

    CMAKE_ARGS 
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/bin

    # 关键：告诉 ExternalProject 我们不需要下载、更新或打补丁
    # 因为源代码就在本地
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    INSTALL_COMMAND "" 
)

