cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("mcstas-udp-transmit" CXX)

set(CMAKE_MACOSX_RPATH OFF)

include_directories(.)

add_library(ReadoutBIFROST SHARED ReadoutBIFROST.cpp ReadoutBIFROST.h BifrostReadout.cpp BifrostReadout.h)
set_target_properties(ReadoutBIFROST PROPERTIES SUFFIX ".so")

set_property(TARGET ReadoutBIFROST PROPERTY CXX_STANDARD 11)

#add_custom_target(runsim
#    COMMAND mcrun "broadcaster.instr" "-n" "100000" "--no-output" "sendPackets=1"
#    DEPENDS ReadoutBIFROST
#    WORKING_DIRECTORY ..
#    COMMENT "Running McStas simulation")
