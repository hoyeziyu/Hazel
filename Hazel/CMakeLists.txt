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
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_library(${PROJECT_NAME} ${HAZEL_SOURCES}
    # src/Platform/Windows/WindowsInput.h
    )
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
)

target_include_directories(${PROJECT_NAME} PUBLIC ${Stb_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC opengl32)
endif()
target_precompile_headers(${PROJECT_NAME} PRIVATE "src/hzpch.h")

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION bin
    LIBRARY DESTINATION bin
    RUNTIME DESTINATION bin
)
