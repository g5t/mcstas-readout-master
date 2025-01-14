list(APPEND CXX_SOURCES
        Readout.cpp
        ReadoutClass.cpp
        enums.cpp
        hdf_interface.cpp
        replay.cpp
)

foreach(CXX_SOURCE IN LISTS CXX_SOURCES)
    #    message(STATUS "Setting compile options for ${CXX_SOURCE} to ${SOURCE_FILE_COMPILE_OPTIONS}")
    set_source_files_properties(${CXX_SOURCE} PROPERTIES COMPILE_OPTIONS "${SOURCE_FILE_COMPILE_OPTIONS}")
endforeach()

foreach(CXX_TARGET IN LISTS LIB_TARGETS)
    target_sources(${CXX_TARGET} PRIVATE ${CXX_SOURCES})
    #target_include_directories(${CXX_TARGET} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
    target_include_directories(
            ${CXX_TARGET}
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
            $<INSTALL_INTERFACE:>  # <prefix>
    )
endforeach()