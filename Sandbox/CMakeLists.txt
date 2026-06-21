project(Sandbox)

# Add source files (.cpp only)
set (SANDBOX_SOURCES
    "src/SandboxApp.cpp"
)

# Define executable
add_executable(${PROJECT_NAME} ${SANDBOX_SOURCES})
hazel_set_output_dirs(${PROJECT_NAME})

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
# Compiler standard
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
# Link libraries
target_link_libraries(${PROJECT_NAME} Hazel::Engine)

# Copy assets next to Sandbox.exe so paths like assets/shaders/... resolve at runtime
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying Sandbox assets"
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin" CACHE PATH "..." FORCE)
endif()

# Install the final executable to a 'bin' directory
install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_SOURCE_DIR}/bin")

# 2. 核心：安装 assets 文件夹到 /bin
# 注意：路径末尾不加斜杠，表示拷贝整个文件夹
#install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets" 
#        DESTINATION "${CMAKE_SOURCE_DIR}/bin")