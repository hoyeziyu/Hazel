project(Hazelnut)

set(HAZELNUT_SOURCES
    src/HazelnutApp.cpp
    src/EditorLayer.cpp
    src/Panels/SceneHierarchyPanel.cpp
    src/Panels/ScriptFieldDrawer.cpp
    src/Panels/ContentBrowserPanel.cpp
)

add_executable(${PROJECT_NAME} ${HAZELNUT_SOURCES})
hazel_set_output_dirs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_link_libraries(${PROJECT_NAME} Hazel::Engine)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/SampleProject"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/LD51Project"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/LD51Project"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/imgui.ini"
    COMMENT "Copying Hazelnut assets, SampleProject, and default imgui.ini"
)

if(HZ_SCRIPTING)
    hazel_deploy_scripting_runtime(${PROJECT_NAME})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject/assets/Scripts/Binaries"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/SampleProject/Assets/Scripts/Binaries/Sample.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SampleProject/assets/Scripts/Binaries/Sample.dll"
        COMMENT "Deploy Sample.dll into bundled SampleProject"
        VERBATIM
    )
endif()
