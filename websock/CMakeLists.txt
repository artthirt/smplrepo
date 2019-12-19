cmake_minimum_required(VERSION 3.10)

message(STATUS "cmake version " ${CMAKE_VERSION})

project(websock LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(HAVE_MMX "use mmx instructions" ON)
option(HAVE_OPENCL "use opencl" OFF)
option(HAVE_CUDA "use cuda" OFF)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(INCLUDES ./
	./include/)

if(MSVC)
	set(HAVE_MMX OFF)
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

if(HAVE_MMX)
    add_definitions(-DUSE_MMX)
endif()

set(SOURCES)

if(HAVE_OPENCL)

	set(OpenCL_FOUND ON)
	set(OpenCL_INCLUDE_DIRS "3rdparty/include/opencl/1.2")
	set(OpenCL_LIBRARIES)

	#find_package(OpenCL)
    if(OpenCL_FOUND)
        set(LIBS ${LIBS} ${OpenCL_LIBRARIES})
        add_definitions(-DUSE_OPENCL)
        set(SOURCES ${SOURCES} cl_main_object.cpp)
    endif()
endif()

if(HAVE_CUDA)
    find_package(CUDA)
    if(CUDA_FOUND)
        enable_language(CUDA)
        add_definitions(-DUSE_CUDA)
        set(INCLUDES ${INCLUDES} cuda/)

        add_library(convert STATIC cuda/convert.cu)

        set(LIBS ${LIBS} convert ${CUDA_LIBRARIES})

        message(STATUS "cuda library " ${CUDA_LIBRARIES})

        #add_library(convert cuda/cuda_)

        #target_link_libraries(convert PRIVATE )

        set(SOURCES ${SOURCES} cuda/cudaobj.cpp)

        set(CMAKE_CUDA_FLAGS "-arch=sm_30 -Xcompiler=-fpic")
     endif()
endif()

add_executable(websock main.cpp
        ${SOURCES}
	websock.cpp
	mainwindow.cpp
	mainwindow.ui
	videodata.cpp
	videoframe.cpp
	videoframe.ui
	DataStream.cpp
	testsender.cpp
	tcpserver.cpp
	tcpsocket.cpp
	dialogsetfilename.cpp
	dialogsetfilename.ui
	resource.qrc)

if(HAVE_CUDA)
	set_property(TARGET websock
		         PROPERTY CUDA_SEPARABLE_COMPILATION ON)
endif()

set(DEFAULT_FFMPEGDIR ./)

if(WIN32)
    set(INCLUDES ${INCLUDES}
        ./ffmpeg/include)
    set(DEFAULT_FFMPEGDIR ./ffmepg/bin/)
endif()

if(HAVE_OPENCL)
    message(STATUS "opencl include dir " ${OpenCL_INCLUDE_DIRS})
    set(INCLUDES ${INCLUDES} ${OpenCL_INCLUDE_DIRS})
endif()

include_directories(${INCLUDES})

find_library(AVCODEC avcodec HINTS      ${DEFAULT_FFMPEGDIR})
find_library(AVFORMAT avformat HINTS    ${DEFAULT_FFMPEGDIR})
find_library(AVUTIL avutil HINTS        ${DEFAULT_FFMPEGDIR})

#get_property(core_options TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_OPTIONS)
#string(REPLACE "-fPIC" "" new_core_options ${core_options})
#set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_OPTIONS ${new_core_options})
#set_property(TARGET Qt5::Core PROPERTY INTERFACE_POSITION_INDEPENDENT_CODE "ON")

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC)

set(LIBS ${LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::OpenGL
    ${AVCODEC} ${AVFORMAT} ${AVUTIL})

if(WIN32)
    set(LIBS ${LIBS} WS2_32 opengl32)
else()
    set(LIBS ${LIBS} pthread GL dl)
endif()

#message(STATUS "cuda flags " ${CMAKE_CUDA_FLAGS})

target_link_libraries(websock ${LIBS} OpenMP::OpenMP_CXX)
