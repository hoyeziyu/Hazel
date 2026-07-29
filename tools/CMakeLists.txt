# tools/CMakeLists.txt — invoked from root when HAZEL_BUILD_SAMPLE_PACK_TOOL is ON

if(HAZEL_BUILD_SAMPLE_PACK_TOOL)
    add_executable(BuildSamplePack BuildSamplePack.cpp)
    target_link_libraries(BuildSamplePack PRIVATE Hazel::Engine)
    target_compile_features(BuildSamplePack PRIVATE cxx_std_17)
    hazel_set_output_dirs(BuildSamplePack NAME Tools)

    add_custom_command(TARGET BuildSamplePack POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Sandbox/assets"
            "$<TARGET_FILE_DIR:BuildSamplePack>/assets"
        COMMENT "Copying Sandbox assets for BuildSamplePack"
    )
endif()
