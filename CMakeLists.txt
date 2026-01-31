cmake_minimum_required(VERSION 3.16)

message(STATUS "Using ${CMAKE_GENERATOR}")
message("Debug: ${CMAKE_CXX_FLAGS_DEBUG}") 
message("Release: ${CMAKE_CXX_FLAGS_RELEASE}") 
message("RelWithDebInfo: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}") 
message("MinSizeRel: ${CMAKE_CXX_FLAGS_MINSIZEREL}") 


# Project
project(HazelProject)

# 设置全局输出目录，方便调试时找到资源
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# Options
option(HAZEL_BUILD_SANDBOX "Build sandbox application" ON)
# Provide a user-facing option to control the library type
#option(BUILD_SHARED_LIBS "Build libraries as shared" OFF)

# Hazel
add_subdirectory(Hazel)

# Sandbox
if(HAZEL_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()
add_subdirectory(Hazelnut)