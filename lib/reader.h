#pragma once
#include <cstring>
#include "ReadoutClass.h"


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


class Reader{
  std::string filename;
  std::optional<HighFive::File> file;
  std::optional<HighFive::DataSet> dataset;
  std::optional<HighFive::DataType> datatype;
  DetectorType detector{DetectorType::Reserved};
  ReadoutType readout{ReadoutType::CAEN};
public:
  RL_API DetectorType detector_type() const {return detector;}
  RL_API ReadoutType readout_type() const {return readout;}

  RL_API explicit Reader(const std::string& filename): filename{filename} {
    try {
      file = HighFive::File(filename, HighFive::File::ReadOnly);  
    } catch (HighFive::Exception & ex) {
      std::cout << "Error opening file " << filename << ":\n" << ex.what();
      file = std::nullopt;
      return;
    }    
    std::string program;
    if (file->hasAttribute("program")) file->getAttribute("program").read(program);
    if (program != "libreadout") {
      throw std::runtime_error("The provided HDF file was not produced using libreadout");
    }
    auto version = file->getAttribute("version").read<std::string>();
    auto this_version = std::string(reinterpret_cast<const char *>(libreadout::version::version_number));
    if (version != this_version){
      std::cout << "The file was produced using libreadout " << version;
      std::cout << " not current " << this_version << std::endl;
    }
    if (!file->hasAttribute("events")){
      std::stringstream s;
      s << "libreadout " << this_version << " expects a file attribute, \"events\", which is not present";
      throw std::runtime_error(s.str());
    }
    auto dataset_name = file->getAttribute("events").read<std::string>();
    try {
      dataset = file->getDataSet(dataset_name);
    } catch (HighFive::Exception & ex) {
      std::cout << "Accessing dataset \"" << dataset_name << "\" failed with error message:\n";
      std::cout << ex.what() << std::endl;
      dataset = std::nullopt;
      return;
    }
    try {
      detector = detectorType_from_name(dataset->getAttribute("detector").read<std::string>());
      readout = readoutType_from_name(dataset->getAttribute("readout").read<std::string>());
    } catch (HighFive::Exception & ex) {
      std::cout << "Error determining dataset \"" << dataset_name << "\" detector and readout types, with message:\n";
      std::cout << ex.what() << std::endl;
      dataset = std::nullopt;
      return;
    }
    datatype = file->getDataType(readoutType_name(readout));
    if (auto shape = dataset->getDimensions(); shape.size() != 1){
      std::stringstream s;
      s << "The dataset is expected to be 1-D not " << shape.size();
      throw std::runtime_error(s.str());
    }
  }

  RL_API ~Reader() = default;

  RL_API [[nodiscard]] size_t size() const {
    return dataset.has_value() ? dataset->getDimensions().back() : 0;
  }

  RL_API auto get_CAEN(size_t index, size_t count) const {
    if (readout != ReadoutType::CAEN){ throw std::runtime_error("Non CAEN readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested");}
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    auto dt = HighFive::create_datatype<CAEN_event>();
    std::vector<CAEN_event> event(count);
    dataset->select({index}, {count}).read_raw(event.data(), datatype.value());
    return event;
  }
  RL_API auto get_TTLMonitor(size_t index, size_t count) const {
    if (readout != ReadoutType::TTLMonitor){ throw std::runtime_error("Non TTLMonitor readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<TTLMonitor_event> event(count);
    dataset->select({index}, {count}).read_raw(event.data(), datatype.value());
    return event;
  }
  RL_API auto get_VMM3(size_t index, size_t count) const{
    if (readout != ReadoutType::VMM3) { throw std::runtime_error("Non VMM3 readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<VMM3_event> event(count);
    dataset->select({index}, {count}).read_raw(event.data(), datatype.value());
    return event;
  }
  RL_API auto get_DREAM(size_t index, size_t count) const{
    if (readout != ReadoutType::DREAM) { throw std::runtime_error("Non VMM3 readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<DREAM_event> event(count);
    dataset->select({index}, {count}).read_raw(event.data(), datatype.value());
    return event;
  }

  RL_API auto all_CAEN() const {return get_CAEN(0, size());}
  RL_API auto all_TTLMonitor() const {return get_TTLMonitor(0, size());}
  RL_API auto all_VMM3() const {return get_VMM3(0, size());}
  RL_API auto all_DREAM() const {return get_DREAM(0, size());}
};
