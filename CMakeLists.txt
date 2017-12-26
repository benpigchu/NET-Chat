cmake_minimum_required(VERSION 3.0)
project(CG-RayTracing)

include_directories(src)
include_directories(src/headers)

# set(CMAKE_BUILD_TYPE Release)

add_executable(server
	src/main.cpp
	)