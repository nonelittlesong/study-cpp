cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

set( OpenCV_DIR "/usr/local/opencv2/share/OpenCV" )

project( mtcnnAttribute )
add_executable( mtcnnAttribute mtcnnAttribute.cpp )

include_directories( /home/song/ncnn/build/install/include )
MESSAGE(${CMAKE_CURRENT_BINARY_DIR})

set(OpenCV_FOUND 1)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

target_link_libraries( mtcnnAttribute ${OpenCV_LIBS}  /home/song/ncnn/build/install/lib/libncnn.a)
