# Premake equivalent:
#   targetdir("bin/" .. outputdir .. "/%{prj.name}")
#   objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
#
# CMake output:
#   bin/<Config>-windows-x86_64/<name>/
#   bin-int/<Config>-windows-x86_64/<name>/

function(hazel_set_output_dirs target)
    set(options)
    set(oneValueArgs NAME)
    cmake_parse_arguments(HAZEL_OUT "${options}" "${oneValueArgs}" "" ${ARGN})

    if(HAZEL_OUT_NAME)
        set(output_name "${HAZEL_OUT_NAME}")
    else()
        set(output_name "${target}")
    endif()

    if(CMAKE_CONFIGURATION_TYPES)
        set(output_configs ${CMAKE_CONFIGURATION_TYPES})
    else()
        set(output_configs ${CMAKE_BUILD_TYPE})
    endif()

    foreach(config ${output_configs})
        string(TOUPPER "${config}" config_upper)
        set(bin_dir "${CMAKE_SOURCE_DIR}/bin/${config}-windows-x86_64/${output_name}")
        set(int_dir "${CMAKE_SOURCE_DIR}/bin-int/${config}-windows-x86_64/${output_name}")

        file(MAKE_DIRECTORY "${bin_dir}")
        file(MAKE_DIRECTORY "${int_dir}")

        set_target_properties(${target} PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${bin_dir}"
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${bin_dir}"
            "ARCHIVE_OUTPUT_DIRECTORY_${config_upper}" "${bin_dir}"
            "PDB_OUTPUT_DIRECTORY_${config_upper}" "${int_dir}"
        )

        if(MSVC)
            # Keep .obj files in bin-int (Premake objdir), but leave compiler PDBs in the
            # build tree (IntDir). Redirecting COMPILE_PDB_OUTPUT_DIRECTORY forces every
            # translation unit to share one .pdb and triggers MSVC C1041 under /MP.
            target_compile_options(${target} PRIVATE
                "$<$<CONFIG:${config}>:/Fo${int_dir}/>"
            )
        endif()
    endforeach()
endfunction()
