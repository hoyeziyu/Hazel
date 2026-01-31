cmake_minimum_required(VERSION 3.16)


project(entt)

# Create an INTERFACE target to hold our warning flags
add_library(${PROJECT_NAME} INTERFACE) 


# 3. 配置包含路径
target_include_directories(${PROJECT_NAME} INTERFACE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)