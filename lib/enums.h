#pragma once
#include "hdf_interface.h"

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

namespace HighFive {
  template<> DataType create_datatype<DetectorType>();
  template<> DataType create_datatype<ReadoutType>();
}