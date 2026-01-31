#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Greeter::Lib" for configuration "Debug"
set_property(TARGET Greeter::Lib APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Greeter::Lib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/Greeter.lib"
  )

list(APPEND _cmake_import_check_targets Greeter::Lib )
list(APPEND _cmake_import_check_files_for_Greeter::Lib "${_IMPORT_PREFIX}/lib/Greeter.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
