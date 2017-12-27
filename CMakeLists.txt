cmake_minimum_required(VERSION 3.0)
project(CG-RayTracing)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)
include_directories(src/headers)

# set(CMAKE_BUILD_TYPE Release)

add_executable(server
	src/main.cpp
	src/Chat.cpp
	src/EventLoop.cpp
	src/EventSource.cpp
	src/TcpServer.cpp
	src/TcpSocket.cpp
	)