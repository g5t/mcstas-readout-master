list(APPEND BIN_SOURCES
        main.cpp
        helper.cpp
)

foreach(BIN_SOURCE IN LISTS BIN_SOURCES)
    #    message(STATUS "Setting compile options for ${BIN_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${BIN_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

foreach(BIN_TARGET IN LISTS CONFIG_BIN_TARGETS)
    target_sources(${BIN_TARGET} PRIVATE ${BIN_SOURCES})
    target_include_directories(${BIN_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_BINARY_DIR} ${ARGS_INCLUDE_DIR})
endforeach()

if (READOUT_BUILD_TESTS)
    add_test(NAME replay_config_help_test COMMAND ${READOUT_CONFIG_BINARY} --help)
    add_test(NAME replay_config_show_test COMMAND ${READOUT_CONFIG_BINARY} --show libdir)
endif()