cmake_minimum_required(VERSION 2.8)
project(Cliente)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
add_executable(cliente client_main.cpp ../Logger.cpp Client.cpp ../Lock/LockFile.cpp)
