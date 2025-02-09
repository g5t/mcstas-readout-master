#pragma once
#include <cstring>
#include "ReadoutClass.h"
#include "enums.h"


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


class Writer{
  std::string filename;
  std::optional<HighFive::File> file;
  std::optional<HighFive::DataSet> dataset;
  std::optional<HighFive::DataType> datatype;
  DetectorType detector{DetectorType::Reserved};
  ReadoutType readout{ReadoutType::CAEN};
  int verbosity{0};
public:
  RL_API DetectorType detector_type() const {return detector;}
  RL_API void detector_type(DetectorType type) {detector = type;}
  RL_API ReadoutType readout_type() const {return readout;}
  RL_API void readout_type(ReadoutType type) {readout = type;}
  RL_API void verbose(int v) {verbosity = v;}

  RL_API explicit Writer(
      const std::string & filename,
      DetectorType detectorType,
      ReadoutType readoutType,
      const std::string & dataset_name = "events"
      )
      : filename{filename}, detector{detectorType}, readout{readoutType} {
    try {
      file = HighFive::File(filename, HighFive::File::OpenOrCreate);
    } catch (HighFive::Exception & ex) {
      std::cout << "Error opening file " << filename << " for writing:\n" << ex.what();
      file = std::nullopt;
      return;
    }

    // we want to output an events list which can grow forever
    auto dataspace = HighFive::DataSpace({0}, {HighFive::DataSpace::UNLIMITED});
    // but should chunk file operations to avoid too much disk IO?
    HighFive::DataSetCreateProps props;
    props.add(HighFive::Chunking(std::vector<hsize_t>{100}));

    auto hc_type = hdf_compound_type();
    hc_type.commit(file.value(), readoutType_name(readout));
    dataset = file.value().createDataSet(dataset_name, dataspace, hc_type, props);

    // Assign useful information as attributes:
    /* FIXME C++20 has char8_t but C++17 does not, so these strings _might_ already be chars
     *       instead of unsigned chars. If that's the case this lambda is a non-op.
     */
    auto u8str = [](const auto * p){return std::string(reinterpret_cast<const char *>(p));};
    file->createAttribute<std::string>("program", "libreadout");
    file->createAttribute<std::string>("version", u8str(libreadout::version::version_number));
    file->createAttribute<std::string>("revision", u8str(libreadout::version::git_revision));
    file->createAttribute<std::string>("events", dataset_name);
    dataset->createAttribute("detector", detectorType_name(detector));
    dataset->createAttribute("readout", readoutType_name(readout));
  }

  RL_API void saveReadout(const uint8_t Ring, const uint8_t FEN, const double tof, const double weight, const void * data){
    if (!dataset.has_value()){
      if (verbosity > 1) std::cout << "No readout saved to file due to no dataset available" << std::endl;
      return;
    }
    const auto type = readoutType_from_detectorType(detector);
    switch (type) {
      case ReadoutType::CAEN: return saveReadout(CAEN_event(Ring, FEN, tof, weight, static_cast<const CAEN_readout_t*>(data)));
      case ReadoutType::TTLMonitor: return saveReadout(TTLMonitor_event(Ring, FEN, tof, weight, static_cast<const TTLMonitor_readout_t*>(data)));
      case ReadoutType::DREAM: return saveReadout(DREAM_event(Ring, FEN, tof, weight, static_cast<const DREAM_readout_t*>(data)));
      case ReadoutType::VMM3: return saveReadout(VMM3_event(Ring, FEN, tof, weight, static_cast<const VMM3_readout_t*>(data)));
      default: throw std::runtime_error("This readout data type not implemented yet!");
    }
  }

  template<class T> void saveReadout(T data){
    if (file.has_value() and dataset.has_value()) {
      auto & ds = dataset.value();
      // the dataset should be 1-D ... hopefully that's true
      auto pos = ds.getDimensions().back();
      auto size = pos + 1;
      ds.resize({size});
      ds.select({pos}, {1}).write(data); // select(offset, count)
    }
  }

private:
  HighFive::CompoundType hdf_compound_type() const {
    using namespace HighFive;
    switch (readout){
      case ReadoutType::CAEN: return create_datatype<CAEN_event>();
      case ReadoutType::TTLMonitor: return create_datatype<TTLMonitor_event>();
      case ReadoutType::DREAM: return create_datatype<DREAM_event>();
      case ReadoutType::VMM3: return create_datatype<VMM3_event>();
      default: throw std::runtime_error("Saving this readout type is not implemented yet!");
    }
  }
};