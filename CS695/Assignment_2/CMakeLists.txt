cmake_minimum_required(VERSION 3.15)
project(Assignment_2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(LIBVIRT REQUIRED IMPORTED_TARGET libvirt)

add_executable(Assignment_2 main.cpp manager/Manager.cpp manager/Manager.h manager/VM.cpp manager/VM.h VMManager/VmManager.cpp VMManager/VmManager.h VMManager/Graph.cpp VMManager/Graph.h)
target_link_libraries(Assignment_2 PkgConfig::GTKMM)
target_link_libraries(Assignment_2 PkgConfig::LIBVIRT)
