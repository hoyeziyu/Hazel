# Build Coral.Managed + Hazel-ScriptCore + active game script project via dotnet CLI

set(HAZEL_DOTNET_OUTPUT "${CMAKE_BINARY_DIR}/dotnet")
set(HAZEL_CORAL_MANAGED_PROJ "${CMAKE_SOURCE_DIR}/vendor/Coral/Coral.Managed/Coral.Managed-Static.csproj")
set(HAZEL_SCRIPTCORE_PROJ "${CMAKE_SOURCE_DIR}/Hazel-ScriptCore/Hazel-ScriptCore.csproj")
set(HAZEL_SAMPLE_SCRIPT_PROJ "${CMAKE_SOURCE_DIR}/Hazelnut/SampleProject/Assets/Scripts/Sample.csproj")
set(HAZEL_SAMPLE_SCRIPT_OUTPUT "${CMAKE_SOURCE_DIR}/Hazelnut/SampleProject/Assets/Scripts/Binaries")

set(DOTNET_CONFIG "$<IF:$<CONFIG:Debug>,Debug,Release>")

add_custom_target(HazelDotNetBuild ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HAZEL_DOTNET_OUTPUT}/DotNet"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HAZEL_DOTNET_OUTPUT}/Resources/Scripts"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HAZEL_SAMPLE_SCRIPT_OUTPUT}"
    COMMAND dotnet build "${HAZEL_CORAL_MANAGED_PROJ}" -c ${DOTNET_CONFIG} --nologo
        -p:AppendTargetFrameworkToOutputPath=false
        -p:OutputPath="${HAZEL_DOTNET_OUTPUT}/DotNet/"
        -p:IntermediateOutputPath="${HAZEL_DOTNET_OUTPUT}/intermediate/Coral.Managed/"
    COMMAND dotnet build "${HAZEL_SCRIPTCORE_PROJ}" -c ${DOTNET_CONFIG} --nologo
        -p:OutputPath="${HAZEL_DOTNET_OUTPUT}/Resources/Scripts/"
        -p:IntermediateOutputPath="${HAZEL_DOTNET_OUTPUT}/intermediate/Hazel-ScriptCore/"
    COMMAND dotnet build "${HAZEL_SAMPLE_SCRIPT_PROJ}" -c ${DOTNET_CONFIG} --nologo
        -p:OutputPath="${HAZEL_SAMPLE_SCRIPT_OUTPUT}/"
        -p:IntermediateOutputPath="${HAZEL_DOTNET_OUTPUT}/intermediate/Sample/"
    COMMENT "Building Coral.Managed, Hazel-ScriptCore, and Sample game scripts"
    VERBATIM
)

function(hazel_deploy_scripting_runtime target_name)
    add_dependencies(${target_name} HazelDotNetBuild)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target_name}>/DotNet"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target_name}>/Resources/Scripts"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${HAZEL_DOTNET_OUTPUT}/DotNet"
            "$<TARGET_FILE_DIR:${target_name}>/DotNet"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${HAZEL_DOTNET_OUTPUT}/Resources/Scripts"
            "$<TARGET_FILE_DIR:${target_name}>/Resources/Scripts"
        COMMENT "Deploy DotNet + Hazel-ScriptCore next to ${target_name}"
        VERBATIM
    )
endfunction()
