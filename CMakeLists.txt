cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(Readout VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_MACOSX_RPATH OFF)

include(GNUInstallDirs)
set(Readout_BINDIR "${CMAKE_INSTALL_BINDIR}")  # e.g. <prefix>/bin
set(Readout_LIBDIR "${CMAKE_INSTALL_LIBDIR}")  # e.g. <prefix>/lib
set(Readout_INCDIR "${CMAKE_INSTALL_INCLUDEDIR}")  # e.g. <prefix>/include
set(Readout_DATADIR "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}") # e.g. <prefix>/share/Readout>
if (NOT Readout_CMAKEDIR)
    set(Readout_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}") #e.g. <prefix>/lib/cmake/Readout>
endif()

#Get a few relative paths, mostly for expansion in various installed files (we
#use PROJECT_BINARY_DIR as prefix here, but it should not matter which as long
#as it is an absolute path):
file(RELATIVE_PATH Readout_BIN2LIB "${PROJECT_BINARY_DIR}/${Readout_BINDIR}" "${PROJECT_BINARY_DIR}/${Readout_LIBDIR}")
file(RELATIVE_PATH Readout_BIN2INC "${PROJECT_BINARY_DIR}/${Readout_BINDIR}" "${PROJECT_BINARY_DIR}/${Readout_INCDIR}")
file(RELATIVE_PATH Readout_BIN2DATA "${PROJECT_BINARY_DIR}/${Readout_BINDIR}" "${PROJECT_BINARY_DIR}/${Readout_DATADIR}")
file(RELATIVE_PATH Readout_CMAKE2ROOT "${PROJECT_BINARY_DIR}/${Readout_CMAKEDIR}" "${PROJECT_BINARY_DIR}/")
file(RELATIVE_PATH Readout_CMAKE2BIN "${PROJECT_BINARY_DIR}/${Readout_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Readout_BINDIR}")
file(RELATIVE_PATH Readout_CMAKE2LIB "${PROJECT_BINARY_DIR}/${Readout_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Readout_LIBDIR}")
file(RELATIVE_PATH Readout_CMAKE2INC "${PROJECT_BINARY_DIR}/${Readout_CMAKEDIR}" "${PROJECT_BINARY_DIR}/${Readout_INCDIR}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file( "${PROJECT_BINARY_DIR}/ReadoutConfigVersion.cmake"
        VERSION ${Readout_VERSION} COMPATIBILITY SameMajorVersion )

include_directories(.)

add_library(Readout SHARED Readout.cpp ReadoutClass.cpp)
set(Readout_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}Readout${CMAKE_SHARED_LIBRARY_SUFFIX}")
set_target_properties(Readout PROPERTIES PUBLIC_HEADER Readout.h)
set_target_properties(Readout PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

set_property(TARGET Readout PROPERTY CXX_STANDARD 11)

add_executable(tester tester.cpp)
target_link_libraries(tester Readout)

install(TARGETS Readout EXPORT ReadoutTargets
        RUNTIME DESTINATION ${Readout_BINDIR}
        LIBRARY DESTINATION ${Readout_LIBDIR}
        ARCHIVE DESTINATION ${Readout_LIBDIR}
        PUBLIC_HEADER DESTINATION ${Readout_INCDIR})
install(EXPORT ReadoutTargets FILE ReadoutTargets.cmake NAMESPACE READOUT:: DESTINATION ${Readout_CMAKEDIR})
add_library(READOUT::Readout ALIAS Readout) # always alias namespaces locally

configure_file("${PROJECT_SOURCE_DIR}/cmake/ReadoutConfig.cmake.in" "${PROJECT_BINARY_DIR}/ReadoutConfig.cmake" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/ReadoutConfigVersion.cmake" "${PROJECT_BINARY_DIR}/ReadoutConfig.cmake"
        DESTINATION ${Readout_CMAKEDIR} )

configure_file( "${PROJECT_SOURCE_DIR}/cmake/readout-config.in" "${PROJECT_BINARY_DIR}/readout-config" @ONLY )
install( PROGRAMS "${PROJECT_BINARY_DIR}/readout-config" DESTINATION ${Readout_BINDIR} )

# Copy the component file to the include directory too
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/Readout.comp" DESTINATION ${Readout_DATADIR})