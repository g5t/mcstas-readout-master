set(CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})

find_package(Python3 QUIET COMPONENTS Interpreter REQUIRED)

if (NOT DEFINED gen_dir)
  set(gen_dir ${CMAKE_BINARY_DIR}/gen)
endif()
if (NOT DEFINED src_dir)
  # issuing, e.g., `cmake -S {source} -B {build} && cmake --build {directory}` gives
  # CMAKE_SOURCE_DIR={source} for the first call and CMAKE_SOURCE_DIR={directory} for the second
  set(src_dir ${CMAKE_SOURCE_DIR})
endif()

set(post_configure_file ${gen_dir}/version.hpp)

function(checkVersion version)
  execute_process(
          COMMAND ${Python3_EXECUTABLE} ${src_dir}/version.py ${src_dir} ${gen_dir}
          OUTPUT_VARIABLE returned_version
          OUTPUT_STRIP_TRAILING_WHITESPACE
          ECHO_ERROR_VARIABLE
          RESULT_VARIABLE result
  )
  if (NOT ${result} EQUAL 0)
    message(DEBUG "${Python3_EXECUTABLE} ${src_dir}/version.py ${src_dir} ${gen_dir}")
    message(AUTHOR_WARNING "Unable to discover git repository version, using fall-back VERSION file")
    file(STRINGS ${CURRENT_LIST_DIR}/VERSION version_file_contents)
    list(GET version_file_contents 0 returned_version)
  else()
    file(WRITE ${CURRENT_LIST_DIR}/VERSION "${returned_version}")
  endif()
  set(${version} ${returned_version} PARENT_SCOPE)
endfunction()

function(checkSetup name)
  add_custom_target(AlwaysCheck COMMAND ${CMAKE_COMMAND}
    -DRUN_CHECK=1
    -Dgen_dir=${gen_dir}
    -Dsrc_dir=${src_dir}
    -P ${CURRENT_LIST_DIR}/check.cmake
    BYPRODUCTS ${post_configure_file}
  )
  checkVersion(CHECK_VERSION)
  if (NOT DEFINED CHECK_VERSION)
    set(CHECK_VERSION "0.0.0")
  endif()
  set("${name}_VERSION" ${CHECK_VERSION} PARENT_SCOPE)
endfunction()

function(addCheckDependency target)
  target_include_directories(${target} PUBLIC ${gen_dir})
  add_dependencies(${target} AlwaysCheck)
endfunction()

if (RUN_CHECK)
  checkVersion(CHECK_VERSION)
  if (NOT DEFINED CHECK_VERSION)
    set(SAFE_VERSION "0.0.0")
  endif()
endif()
