cmake_minimum_required(VERSION 3.0)

project(DB4AI-UDF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SET_VERBOSE_MAKEFILE ON)
set(Torch_DIR "$ENV{Torch_DIR}/share/cmake/Torch")
set(OpenCV_DIR "$ENV{OpenCV_DIR}/share/OpenCV/")

# libtorch
find_package(Torch REQUIRED)
if(Torch_FOUND)
    message(STATUS "Torch library found!")
else()
    message(FATAL_ERROR "Could not locate Torch" \n)
endif()

# opencv
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV library found!")
else()
    message(FATAL_ERROR "Could not locate OpenCV" \n)
endif()

# libtorch include
set(Torch_INCLUDE_DIR "$ENV{Torch_DIR}/include")
include_directories(${Torch_INCLUDE_DIR})
include_directories(${Torch_INCLUDE_DIR}/torch/csrc/api/include)

# OpenCV include
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIR}/opencv2) 
include_directories(${OpenCV_INCLUDE_DIR}/opencv)

# postgresql include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../interfaces/libpq)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../contrib)


include_directories(/home/lhh/sentencepiece/include)

FILE(GLOB ALL_SOURCES "./*/*.cpp" "./*.cpp" "./*.c" "./*/*.c")
add_library(aiudf SHARED ${ALL_SOURCES})

target_link_libraries(aiudf ${OpenCV_LIBS})
target_link_libraries(aiudf /home/lhh/postgres-kernel-new/pg_compile/lib/libpq.so.5.12)
# target_link_libraries(aiudf /home/lhh/sentencepiece/lib64/libsentencepiece.so.0.0.0)