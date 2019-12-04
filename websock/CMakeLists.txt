cmake_minimum_required(VERSION 3.4.1)

project(websock)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)

add_executable(websock main.cpp websock.cpp mainwindow.cpp mainwindow.ui videodata.cpp videoframe.cpp videoframe.ui DataStream.cpp)

include_directories(./
                        ./include/
	                ./ffmpeg/include)

find_library(AVCODEC avcodec HINTS ffmpeg/bin)
find_library(AVFORMAT avformat HINTS ffmpeg/bin)
find_library(AVUTIL avutil HINTS ffmpeg/bin)

set(LIBS Qt5::Core Qt5::Widgets Qt5::Network Qt5::OpenGL
    ${AVCODEC} ${AVFORMAT} ${AVUTIL})

if(WIN32)
    set(LIBS ${LIBS} WS2_32)
else()
    set(LIBS ${LIBS} pthread)
endif()

target_link_libraries(websock ${LIBS})
