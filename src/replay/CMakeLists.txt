list(APPEND BIN_SOURCES
        main.cpp
)

foreach(BIN_SOURCE IN LISTS BIN_SOURCES)
    #    message(STATUS "Setting compile options for ${BIN_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${BIN_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

foreach(BIN_TARGET IN LISTS BIN_TARGETS)
    target_sources(${BIN_TARGET} PRIVATE ${BIN_SOURCES})
    target_include_directories(${BIN_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${ARGS_INCLUDE_DIR})
endforeach()


if (READOUT_TESTS)
    add_test(NAME replay_binary_test COMMAND ${READOUT_REPLAY_BINARY} --help)
endif()