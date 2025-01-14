#include "enums.h"

DetectorType detectorType_from_int(const int type){
  switch(type){
    case 0x00: return Reserved;
    case 0x10: return TTLMonitor;
    case 0x30: return LOKI;
    case 0x34: return BIFROST;
    case 0x38: return MIRACLES;
    case 0x40: return CSPEC;
    case 0x44: return NMX;
    case 0x48: return FREIA;
    case 0x50: return TREX;
    case 0x60: return DREAM;
    case 0x64: return MAGIC;
    default: throw std::runtime_error("Undefined DetectorType");
  }
}

ReadoutType readoutType_from_detectorType(const DetectorType type){
  switch(type){
    case TTLMonitor: return ReadoutType::TTLMonitor;
    case LOKI:
    case BIFROST:
    case MIRACLES:
    case CSPEC: return ReadoutType::CAEN;
    case NMX:
    case FREIA:
    case TREX: return ReadoutType::VMM3;
    case DREAM: return ReadoutType::DREAM;
    case MAGIC: return ReadoutType::MAGIC;
    default: throw std::runtime_error("No ReadoutType for provided DetectorType");
  }
}

ReadoutType readoutType_from_int(const int int_type) {
  return readoutType_from_detectorType(detectorType_from_int(int_type));
}


HighFive::EnumType<DetectorType> create_detector_enum_type(){
  return {
      {"TTLMonitor", DetectorType::TTLMonitor},
      {"LOKI", DetectorType::LOKI},
      {"BIFROST", DetectorType::MIRACLES},
      {"CSPEC", DetectorType::CSPEC},
      {"NMX", DetectorType::NMX},
      {"FREIA", DetectorType::FREIA},
      {"TREX", DetectorType::TREX},
      {"DREAM", DetectorType::DREAM},
      {"MAGIC", DetectorType::MAGIC},
  };
}

HighFive::EnumType<ReadoutType> create_readout_enum_type() {
  return {
      {"TTLMonitor", ReadoutType::TTLMonitor},
      {"CAEN", ReadoutType::CAEN},
      {"VMM3", ReadoutType::VMM3},
      {"DREAM", ReadoutType::DREAM},
      {"MAGIC", ReadoutType::MAGIC},
  };
}



namespace HighFive {
  template<> DataType create_datatype<ReadoutType>(){return create_readout_enum_type();}
  template<> DataType create_datatype<DetectorType>(){return create_detector_enum_type();}
}