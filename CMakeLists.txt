cmake_minimum_required(VERSION 3.25)

# vcpkg manifest 模式：依赖在 vcpkg.json 中声明，由 toolchain 自动安装
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
endif()

project(Hazel)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(HazelOutputDirs)

# Hazel library
add_subdirectory(Hazel)


option(HAZEL_BUILD_SANDBOX "Build sandbox application" ON)
# Sandbox application
if(HAZEL_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()
