cmake_minimum_required(VERSION 3.15)
project(Assignment_2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(Server Server.cpp KVCache.cpp KVClientHandler.cpp KVServerLibrary.cpp KVStore.cpp ThreadPool.cpp)
