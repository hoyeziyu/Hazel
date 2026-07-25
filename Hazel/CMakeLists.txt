cmake_minimum_required(VERSION 3.25)

project(hazel-engine)

file(GLOB_RECURSE HAZEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

foreach(FILE ${HAZEL_SOURCES})
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "/" "\\" GROUP_NAME "${PARENT_DIR}")
    source_group("${GROUP_NAME}" FILES "${FILE}")
endforeach()

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_library(${PROJECT_NAME} ${HAZEL_SOURCES})
add_library(Hazel::Engine ALIAS ${PROJECT_NAME})
hazel_set_output_dirs(${PROJECT_NAME} NAME Hazel)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC HZ_PLATFORM_WINDOWS HZ_ENABLE_ASSERTS)
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HZ_BUILD_DLL)
    endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} PUBLIC
    spdlog::spdlog
    glfw
    glad::glad
    EnTT::EnTT
    glm::glm
    imgui::imgui
    yaml-cpp::yaml-cpp
    imguizmo::imguizmo
    assimp::assimp
    box2d::box2d
)

target_include_directories(${PROJECT_NAME} PUBLIC ${Stb_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC opengl32)
endif()
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/hzpch.h")

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable architecture-specific optimizations for Release builds
# -march=native: use instructions available on the build CPU (e.g. AVX)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-O3 -march=native>
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/O2 /arch:AVX2>
    )
endif()

# Enable link-time optimization (LTO) if supported
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)

if(result)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION $<CONFIG:Release>
    )
else()
    message(WARNING "IPO is not supported: ${output}")
endif()

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION bin
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)
