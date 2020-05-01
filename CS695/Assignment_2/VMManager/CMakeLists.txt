cmake_minimum_required(VERSION 3.15)
project(Assignment_2)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(LIBVIRT REQUIRED IMPORTED_TARGET libvirt)

add_executable(Manager main.cpp Manager.cpp Manager.h VM.cpp VM.h VmManager.cpp VmManager.h Graph.cpp Graph.h)
target_link_libraries(VM_Manager PkgConfig::GTKMM)
target_link_libraries(VM_Manager PkgConfig::LIBVIRT)
