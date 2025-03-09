cmake_minimum_required(VERSION 3.20...3.30)

project(ClientServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Source/Client)
add_subdirectory(Source/Server)
add_subdirectory(Dependencies/Boost)
add_subdirectory(Dependencies/SpdLog)
