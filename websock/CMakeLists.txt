cmake_minimum_required(VERSION 3.4.1)

project(websock)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(OpenMP)

if(NOT TARGET OpenMP::OpenMP_CXX)
    find_package(Threads REQUIRED)
    add_library(OpenMP::OpenMP_CXX IMPORTED INTERFACE)
    set_property(TARGET OpenMP::OpenMP_CXX
                 PROPERTY INTERFACE_COMPILE_OPTIONS ${OpenMP_CXX_FLAGS})
    # Only works if the same flag is passed to the linker; use CMake 3.9+ otherwise (Intel, AppleClang)
    set_property(TARGET OpenMP::OpenMP_CXX
                 PROPERTY INTERFACE_LINK_LIBRARIES ${OpenMP_CXX_FLAGS} Threads::Threads)

endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)

add_executable(websock main.cpp websock.cpp mainwindow.cpp mainwindow.ui videodata.cpp videoframe.cpp videoframe.ui DataStream.cpp testsender.cpp)

set(INCLUDES ./
    ./include/)

set(DEFAULT_FFMPEGDIR ./)

if(WIN32)
    set(INCLUDES ${INCLUDES}
        ./ffmpeg/include)
    set(DEFAULT_FFMPEGDIR ${DEFAULT_FFMPEGDIR})
endif()

include_directories(${INCLUDES})

find_library(AVCODEC avcodec HINTS      ${DEFAULT_FFMPEGDIR})
find_library(AVFORMAT avformat HINTS    ${DEFAULT_FFMPEGDIR})
find_library(AVUTIL avutil HINTS        ${DEFAULT_FFMPEGDIR})

set(LIBS Qt5::Core Qt5::Widgets Qt5::Network Qt5::OpenGL
    ${AVCODEC} ${AVFORMAT} ${AVUTIL})

if(WIN32)
	set(LIBS ${LIBS} WS2_32 opengl32)
else()
    set(LIBS ${LIBS} pthread GL)
endif()

target_link_libraries(websock ${LIBS} OpenMP::OpenMP_CXX)
