cmake_minimum_required(VERSION 2.8)
project(DbManager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
add_executable(gestor manager_main.cpp DbManager.cpp Database.cpp ../Logger.cpp ../Lock/LockFile.cpp ../Signals/SignalHandler.cpp)
