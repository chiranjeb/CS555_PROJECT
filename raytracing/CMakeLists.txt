cmake_minimum_required(VERSION 3.5)
project(raytracer)

include_directories(src)

find_package (Threads)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

include(src/framework/CMakeLists.txt)
include(src/transport/CMakeLists.txt)
include(src/master/CMakeLists.txt)
include(src/transport/CMakeLists.txt)
include(src/worker/CMakeLists.txt)
include(src/wiremsg/CMakeLists.txt)
include(src/ray_tracer/CMakeLists.txt)



set(TARGET_NAME master)
add_executable(master ${RAY_TRACER_MASTER_SOURCES} src/master/master_main.cpp)
SET_TARGET_PROPERTIES(master PROPERTIES COMPILE_FLAGS "-D__Make_master")
target_link_libraries (master ${CMAKE_THREAD_LIBS_INIT})

set(TARGET_NAME worker)
add_executable(worker ${RAY_TRACER_WORKER_SOURCES} src/worker/worker_main.cpp)
SET_TARGET_PROPERTIES(worker PROPERTIES COMPILE_FLAGS "-D__Make_worker")
target_link_libraries (worker ${CMAKE_THREAD_LIBS_INIT})
