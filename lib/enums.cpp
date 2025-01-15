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

std::string detectorType_name(DetectorType dt){
  switch (dt) {
    case DetectorType::TTLMonitor: return "DetectorType::TTLMonitor";
    case DetectorType::LOKI: return "DetectorType::LOKI";
    case DetectorType::BIFROST: return "DetectorType::BIFROST";
    case DetectorType::MIRACLES: return "DetectorType::MIRACLES";
    case DetectorType::CSPEC: return "DetectorType::CSPEC";
    case DetectorType::NMX: return "DetectorType::NMX";
    case DetectorType::FREIA: return "DetectorType::FREIA";
    case DetectorType::TREX: return "DetectorType::TREX";
    case DetectorType::DREAM: return "DetectorType::DREAM";
    case DetectorType::MAGIC: return "DetectorType::MAGIC";
    default: return "DetectorType::Reserved";
  }
}
std::string readoutType_name(ReadoutType rt){
  switch (rt){
    case ReadoutType::TTLMonitor: return "ReadoutType::TTLMonitor";
    case ReadoutType::CAEN: return "ReadoutType::CAEN";
    case ReadoutType::VMM3: return "ReadoutType::VMM3";
    case ReadoutType::DREAM: return "ReadoutType::DREAM";
    case ReadoutType::MAGIC: return "ReadoutType::MAGIC";
    default: return "";
  }
}
DetectorType detectorType_from_name(const std::string & name){
  if (name == "DetectorType::TTLMonitor") return DetectorType::TTLMonitor;
  if (name == "DetectorType::LOKI") return DetectorType::LOKI;
  if (name == "DetectorType::BIFROST") return DetectorType::BIFROST;
  if (name == "DetectorType::MIRACLES") return DetectorType::MIRACLES;
  if (name == "DetectorType::CSPEC") return DetectorType::CSPEC;
  if (name == "DetectorType::NMX") return DetectorType::NMX;
  if (name == "DetectorType::FREIA") return DetectorType::FREIA;
  if (name == "DetectorType::TREX") return DetectorType::TREX;
  if (name == "DetectorType::DREAM") return DetectorType::DREAM;
  if (name == "DetectorType::MAGIC") return DetectorType::MAGIC;
  if (name == "DetectorType::Reserved") return DetectorType::Reserved;
  std::stringstream s;
  s << "Provided name=" << name << " is not a known DetectorType";
  throw std::runtime_error(s.str());
}
ReadoutType readoutType_from_name(const std::string & name){
  if (name == "ReadoutType::TTLMonitor") return ReadoutType::TTLMonitor;
  if (name == "ReadoutType::CAEN") return ReadoutType::CAEN;
  if (name == "ReadoutType::VMM3") return ReadoutType::VMM3;
  if (name == "ReadoutType::DREAM") return ReadoutType::DREAM;
  if (name == "ReadoutType::MAGIC") return ReadoutType::MAGIC;
  std::stringstream s;
  s << "Provided name=" << name << " is not a known ReadoutType";
  throw std::runtime_error(s.str());
}
