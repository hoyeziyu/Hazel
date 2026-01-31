project(hazel-inputs)

# Add source files (.cpp only)
set (HazelInputs_SOURCES
    "src/EditorLayer.h" 
    "src/EditorLayer.cpp"
    "src/HazelnutApp.cpp" 
 "src/Panels/SceneHierarchyPanel.h" "src/Panels/SceneHierarchyPanel.cpp")

# Define executable
add_executable(${PROJECT_NAME} ${HazelInputs_SOURCES})

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
# Compiler standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
# Link libraries
target_link_libraries(${PROJECT_NAME} Hazel::Engine)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin" CACHE PATH "..." FORCE)
endif()

# Install the final executable to a 'bin' directory
install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_SOURCE_DIR}/bin")

# 2. 核心：安装 assets 文件夹到 /bin
# 注意：路径末尾不加斜杠，表示拷贝整个文件夹
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" 
        DESTINATION "${CMAKE_SOURCE_DIR}/bin")