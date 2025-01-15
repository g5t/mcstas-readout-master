#pragma once
#include "hdf_interface.h"

#ifdef WIN32
// Export symbols if compile flags "READOUT_SHARED" and "READOUT_EXPORT" are set on Windows.
    #ifdef READOUT_SHARED
        #ifdef READOUT_EXPORT
            #define RL_API __declspec(dllexport)
        #else
            #define RL_API __declspec(dllimport)
        #endif
    #else
        // Disable definition if linking statically.
        #define RL_API
    #endif
#else
// Disable definition for non-Win32 systems.
#define RL_API
#endif

enum class Verbosity {
  silent,
  errors,
  warnings,
  info,
  details
};

enum DetectorType {
  Reserved = 0x00,
  TTLMonitor = 0x10,
  LOKI = 0x30,
  BIFROST = 0x34,
  MIRACLES = 0x38,
  CSPEC = 0x40,
  NMX = 0x44,
  FREIA = 0x48,
  TREX = 0x50,
  DREAM = 0x60,
  MAGIC = 0x64
};

enum class ReadoutType {
  TTLMonitor,
  CAEN,
  VMM3,
  DREAM,
  MAGIC,
};

DetectorType detectorType_from_int(int);
ReadoutType readoutType_from_detectorType(DetectorType type);
ReadoutType readoutType_from_int(int int_type);

DetectorType detectorType_from_name(const std::string & name);
std::string detectorType_name(DetectorType);
ReadoutType readoutType_from_name(const std::string & name);
std::string readoutType_name(ReadoutType);
//
//DetectorType detectorType_from_name(std::string && name);
//ReadoutType readoutType_from_name(std::string && name);