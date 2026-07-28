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

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
include(HazelImGui)

# Hazel library
add_subdirectory(Hazel)


option(HAZEL_BUILD_SANDBOX "Build sandbox application" ON)
option(HAZEL_BUILD_HAZELNUT "Build Hazelnut editor application" ON)
option(HAZEL_BUILD_RUNTIME "Build Hazel-Runtime standalone player" ON)
option(HAZEL_BUILD_LAUNCHER "Build Hazel-Launcher project picker" ON)
option(HAZEL_BUILD_SAMPLE_PACK_TOOL "Build SampleProject AssetPack command-line tool" ON)
option(HAZEL_BUILD_TESTS "Build Google Test unit tests" ON)
option(HAZEL_BUILD_BENCHMARKS "Build Google Benchmark performance tests" ON)
option(HZ_SCRIPTING "Enable C# scripting via Coral" ON)
option(HZ_AUDIO "Enable miniaudio audio playback" ON)

if(HZ_SCRIPTING)
    include(HazelCoral)
    include(HazelScripting)
endif()

# Sandbox application
if(HAZEL_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
endif()

if(HAZEL_BUILD_HAZELNUT)
    add_subdirectory(Hazelnut)
endif()

if(HAZEL_BUILD_RUNTIME)
    add_subdirectory(Hazel-Runtime)
endif()

if(HAZEL_BUILD_LAUNCHER)
    add_subdirectory(Hazel-Launcher)
endif()

if(HAZEL_BUILD_SAMPLE_PACK_TOOL)
    add_subdirectory(tools)
endif()

if(HAZEL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(HAZEL_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()


