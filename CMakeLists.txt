cmake_minimum_required(VERSION 3.16)
project(flir_hadron_640 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CMAKE_PREFIX_PATH OR CMAKE_PREFIX_PATH STREQUAL "")
    set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.11.1/gcc_64")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort)

qt_standard_project_setup()

qt_add_executable(flir_hadron_640
    # core
    src/core/RawFrame.h
    src/core/SerialPort.cpp
    src/core/SerialPort.h
    src/core/VideoDevice.cpp
    src/core/VideoDevice.h

    # gui
    src/gui/main.cpp
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/ConnectDialog.cpp
    src/gui/ConnectDialog.h
    src/gui/RawFrameView.cpp
    src/gui/RawFrameView.h
)

target_link_libraries(flir_hadron_640 PRIVATE
    Qt6::Core Qt6::Widgets Qt6::SerialPort)

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_MESSAGELOGCONTEXT)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_DEBUG_OUTPUT)
endif()
