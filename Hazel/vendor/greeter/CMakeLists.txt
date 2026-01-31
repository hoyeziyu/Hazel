cmake_minimum_required(VERSION 3.16)

project(GreeterLib VERSION 1.0.0)

add_library(GreeterLib src/Greeter.cpp)
add_library(Greeter::Lib ALIAS GreeterLib)

set_target_properties(GreeterLib PROPERTIES 
  OUTPUT_NAME Greeter
  EXPORT_NAME Lib
)

include(GNUInstallDirs)

target_include_directories(GreeterLib PUBLIC
    # This location is in the build tree
    # "${CMAKE_CURRENT_SOURCE_DIR}/include"
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  TARGETS GreeterLib
  EXPORT GreeterLibTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT GreeterLibTargets
  FILE GreeterLibTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GreeterLib
  NAMESPACE Greeter::
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "cmake/GreeterLibConfig.cmake.in"
  "GreeterLibConfig.cmake"
  INSTALL_DESTINATION
  "${CMAKE_INSTALL_LIBDIR}/cmake/GreeterLib"
)

write_basic_package_version_file(
  "GreeterLibConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/GreeterLibConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/GreeterLibConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/GreeterLib"
)