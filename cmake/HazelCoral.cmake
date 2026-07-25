# Coral.Native — C++ HostFXR interop (C++20)
set(HAZEL_CORAL_ROOT "${CMAKE_SOURCE_DIR}/vendor/Coral")

# MSVC: wchar_t stream fix for HostInstance error logging
set(HAZEL_CORAL_HOSTINSTANCE "${HAZEL_CORAL_ROOT}/Coral.Native/Source/Coral/HostInstance.cpp")
if(WIN32 AND EXISTS "${HAZEL_CORAL_HOSTINSTANCE}")
    file(READ "${HAZEL_CORAL_HOSTINSTANCE}" HAZEL_CORAL_HOSTINSTANCE_CONTENT)
    if(NOT HAZEL_CORAL_HOSTINSTANCE_CONTENT MATCHES "std::wcerr")
        execute_process(
            COMMAND git apply --ignore-whitespace "${CMAKE_SOURCE_DIR}/cmake/patches/Coral-HostInstance-msvc.patch"
            WORKING_DIRECTORY "${HAZEL_CORAL_ROOT}"
            RESULT_VARIABLE HAZEL_CORAL_PATCH_RESULT
        )
        if(NOT HAZEL_CORAL_PATCH_RESULT EQUAL 0)
            message(WARNING "Failed to apply Coral MSVC patch (result=${HAZEL_CORAL_PATCH_RESULT})")
        endif()
    endif()
endif()

file(GLOB_RECURSE CORAL_NATIVE_SOURCES CONFIGURE_DEPENDS
    "${HAZEL_CORAL_ROOT}/Coral.Native/Source/Coral/*.cpp"
)

add_library(Coral.Native STATIC ${CORAL_NATIVE_SOURCES})

target_include_directories(Coral.Native PUBLIC
    "${HAZEL_CORAL_ROOT}/Coral.Native/Include"
    "${HAZEL_CORAL_ROOT}/Coral.Native/Source"
    "${HAZEL_CORAL_ROOT}/NetCore"
)

target_compile_features(Coral.Native PUBLIC cxx_std_20)

if(WIN32)
    target_compile_definitions(Coral.Native PUBLIC CORAL_WINDOWS NOMINMAX)
    target_compile_options(Coral.Native PRIVATE
        /FI"${HAZEL_CORAL_ROOT}/Coral.Native/Source/CoralPCH.hpp"
        /utf-8
    )
endif()

if(MSVC)
    target_precompile_headers(Coral.Native PRIVATE "${HAZEL_CORAL_ROOT}/Coral.Native/Source/CoralPCH.hpp")
endif()

add_library(Coral::Native ALIAS Coral.Native)
