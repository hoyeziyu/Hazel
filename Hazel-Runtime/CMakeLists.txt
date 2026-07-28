project(Hazel-Runtime)

set(HAZEL_RUNTIME_SOURCES
    src/RuntimeApp.cpp
    src/RuntimeLayer.cpp
)

add_executable(${PROJECT_NAME} ${HAZEL_RUNTIME_SOURCES})
hazel_set_output_dirs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} Hazel::Engine)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Hazelnut/assets/shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Hazelnut/SampleProject"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject"
    COMMENT "Copying runtime shaders and SampleProject"
)

if(HZ_SCRIPTING)
    hazel_deploy_scripting_runtime(${PROJECT_NAME})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject/assets/Scripts/Binaries"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Hazelnut/SampleProject/assets/Scripts/Binaries/Sample.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject/assets/Scripts/Binaries/Sample.dll"
        COMMENT "Deploy Sample.dll into bundled SampleProject"
        VERBATIM
    )
endif()
