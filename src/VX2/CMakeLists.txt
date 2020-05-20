cmake_minimum_required(VERSION 3.5.0)
project(Voxelyze VERSION 0.1.0)

set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
file( GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.c* )

add_executable(Voxelyze main.cpp ${SRC})

target_include_directories(Voxelyze PUBLIC ${INCLUDE_DIR})

add_definitions(-DUSE_OMP)
