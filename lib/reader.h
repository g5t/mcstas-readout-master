#pragma once
#include <cstring>
#include "ReadoutClass.h"

class Reader{
  std::string filename;
  std::optional<HighFive::File> file;
  std::optional<HighFive::DataSet> dataset;
  DetectorType detector{DetectorType::Reserved};
  ReadoutType readout{ReadoutType::CAEN};
public:
  DetectorType detector_type() const {return detector;}
  ReadoutType readout_type() const {return readout;}

  explicit Reader(const std::string& filename): filename{filename} {
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
    std::string version, dataset_name;
    file->getAttribute("version").read(version);
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
    file->getAttribute("events").read(dataset_name);
    try {
      dataset = file->getDataSet(dataset_name);
    } catch (HighFive::Exception & ex) {
      std::cout << "Accessing dataset \"" << dataset_name << "\" failed with error message:\n";
      std::cout << ex.what() << std::endl;
      dataset = std::nullopt;
      return;
    }
    try {
      dataset->getAttribute("detector").read(detector);
      dataset->getAttribute("readout").read(readout);
    } catch (HighFive::Exception & ex) {
      std::cout << "Error determining dataset \"" << dataset_name << "\" detector and readout types, with message:\n";
      std::cout << ex.what() << std::endl;
      dataset = std::nullopt;
      return;
    }
    if (auto shape = dataset->getDimensions(); shape.size() != 1){
      std::stringstream s;
      s << "The dataset is expected to be 1-D not " << shape.size();
      throw std::runtime_error(s.str());
    }
  }

  [[nodiscard]] size_t size() const {
    return dataset.has_value() ? dataset->getDimensions().back() : 0;
  }

  auto get_CAEN(size_t index, size_t count) const {
    if (readout != ReadoutType::CAEN){ throw std::runtime_error("Non CAEN readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested");}
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<CAEN_event> event{};
    dataset->select({index}, {count}).read(event);
    return event;
  }
  auto get_TTLMonitor(size_t index, size_t count) const {
    if (readout != ReadoutType::TTLMonitor){ throw std::runtime_error("Non TTLMonitor readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<TTLMonitor_event> event{};
    dataset->select({index}, {count}).read(event);
    return event;
  }
  auto get_VMM3(size_t index, size_t count) const{
    if (readout != ReadoutType::VMM3) { throw std::runtime_error("Non VMM3 readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<VMM3_event> event{};
    dataset->select({index}, {count}).read(event);
    return event;
  }
  auto get_DREAM(size_t index, size_t count) const{
    if (readout != ReadoutType::DREAM) { throw std::runtime_error("Non VMM3 readout type"); }
    if (index >= size()) { throw std::runtime_error("Out of bounds event requested"); }
    if (index + count > size()) { throw std::runtime_error("Out of bounds event requested");}
    std::vector<DREAM_event> event{};
    dataset->select({index}, {count}).read(event);
    return event;
  }

  auto all_CAEN() const {
    if (readout != ReadoutType::CAEN){ throw std::runtime_error("Non CAEN readout type"); }
    std::vector<CAEN_event> events;
    dataset->read(events);
    return events;
  }
  auto all_TTLMonitor() const {
    if (readout != ReadoutType::TTLMonitor){ throw std::runtime_error("Non TTLMonitor readout type"); }
    std::vector<TTLMonitor_event> events;
    dataset->read(events);
    return events;
  }
  auto all_VMM3() const {
    if (readout != ReadoutType::VMM3){ throw std::runtime_error("Non VMM3 readout type"); }
    std::vector<VMM3_event> events;
    dataset->read(events);
    return events;
  }
  auto all_DREAM() const {
    if (readout != ReadoutType::DREAM){ throw std::runtime_error("Non DREAM readout type"); }
    std::vector<DREAM_event> events;
    dataset->read(events);
    return events;
  }
};
