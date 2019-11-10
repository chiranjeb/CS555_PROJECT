cmake_minimum_required(VERSION 3.5)
project(raytracer)

include_directories(includes)
include_directories(src)

find_package (Threads)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")
#SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

include(src/transport/CMakeLists.txt)
include(src/renderer/CMakeLists.txt)
include(src/transport/CMakeLists.txt)
include(src/worker/CMakeLists.txt)

set(TARGET_NAME renderer)
add_executable(renderer ${RAY_TRACER_RENDERER_SOURCES} src/renderer/renderer_main.cpp)
target_link_libraries (renderer ${CMAKE_THREAD_LIBS_INIT})

set(TARGET_NAME worker)
add_executable(worker ${RAY_TRACER_WORKER_SOURCES} src/worker/worker_main.cpp)
target_link_libraries (worker ${CMAKE_THREAD_LIBS_INIT})
