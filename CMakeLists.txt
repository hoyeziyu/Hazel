# library的配置文件（cmakelists.txt）使用target_include_directories命令添加头文件的搜索路径
# application的配置文件（cmakelists.txt）使用target_include_directories命令添加library的搜索路径

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
option(HAZEL_BUILD_TESTS "Build Google Test unit tests" ON)
option(HAZEL_BUILD_BENCHMARKS "Build Google Benchmark performance tests" ON)

# Sandbox application
if(HAZEL_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()

if(HAZEL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(HAZEL_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()


