// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief UDP readout generator class
///
//===----------------------------------------------------------------------===//
#pragma once

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#endif

#include <string>
#include <utility>
#include <optional>

#include "Structs.h"
#include "Readout.h"
#include "enums.h"
#include "hdf_interface.h"
#include "version.hpp"
#include "efu_time.h"

class Readout {
public:
  Readout(
      std::string IpAddress,
        const int UDPPort,
        const int TCPPort,
        const int Type=0x34,
        efu_time p = efu_time(1),
        efu_time t = efu_time()
  ): Type(detectorType_from_int(Type)),
     ipaddr(std::move(IpAddress)),
     port(UDPPort),
     tcp_port(TCPPort),
     period(p),
     time(t)
  {
    sockOpen(ipaddr, port);
    auto prev = time - period;
    setPulseTime(time.high(), time.low(), prev.high(), prev.low());
    newPacket();
  }

  ~Readout() {
    // ensure any buffered data is sent before the object is destroyed
    send();
  }

  // Adds a readout to the transmission buffer.
  // If there is no room left, transmit and initialize a new packet
  void addReadout(uint8_t Ring, uint8_t FEN, double tof, double weight, const void * data);
  // Specializations for handled data types
  void addReadout(uint8_t Ring, uint8_t FEN, efu_time t, const CAEN_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, efu_time t, const TTLMonitor_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, efu_time t, const DREAM_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, efu_time t, const VMM3_readout_t * data);

  void saveReadout(uint8_t Ring, uint8_t FEN, double tof, double weight, const void * data);
  template<class T> void saveReadout(T data){
    if (file.has_value() and dataset.has_value()) {
      auto ds = dataset.value();
      // the dataset should be 1-D ... hopefully that's true
      auto pos = ds.getDimensions().back();
      auto size = pos + 1;
      ds.resize({size});
      ds.select({pos}, {1}).write(data); // select(offset, count)
    }
  }

  // send the current data buffer
  int send();

  // Update the pulse and previous pulse times
  void setPulseTime(uint32_t PHI, uint32_t PLO, uint32_t PPHI, uint32_t PPLO);

  void update_time(){
    auto now = efu_time();
    if ((now - time) >= &period){
      now = time + period * ((now - time) / period);
    }
    // The ESS Caen EFUs require (now - prev) <= 5 * rep; so we should fake it
    if ((now - time) > (period * 5u)) {
      time = now - period;
    }
    send();
    setPulseTime(now.high(), now.low(), time.high(), time.low());
    newPacket();
    time = now;
  }

  // Query the current pulse and previous pulse times
  [[nodiscard]] std::pair<uint32_t, uint32_t> lastPulseTime() const;
  [[nodiscard]] std::pair<uint32_t, uint32_t> prevPulseTime() const;
  [[nodiscard]] std::pair<uint32_t, uint32_t> lastEventTime() const;

  // Initialize a new packet with no readouts
  void newPacket();

  // Tell the (remote) device to shut down
  int command_shutdown() const;

  // Set verbosity via enum
  int verbose(const Verbosity v){
    switch (v) {
      case Verbosity::details: verbosity=3; break;
      case Verbosity::info: verbosity=2; break;
      case Verbosity::warnings: verbosity=1; break;
      case Verbosity::errors: verbosity=0; break;
      case Verbosity::silent: verbosity=-1; break;
      default: verbosity=0;
    }
    return verbosity;
  }
  int verbose(const int v){verbosity = v; return verbosity;}

  void dump_to(const std::string & filename, const std::string & dataset_name = "events"){
    // Do we need to keep this reference ot the file?
    file = HighFive::File(filename, HighFive::File::OpenOrCreate);
    // we want to output an events list which can grow forever
    auto dataspace = HighFive::DataSpace({0}, {HighFive::DataSpace::UNLIMITED});
    // but should chunk file operations to avoid too much disk IO?
    HighFive::DataSetCreateProps props;
    props.add(HighFive::Chunking(std::vector<hsize_t>{100}));
    dataset = file.value().createDataSet(dataset_name, dataspace, datatype(), props);
    // Assign useful information as attributes:
    /* FIXME C++20 has char8_t but C++17 does not, so these strings _might_ already be chars
     *       instead of unsigned chars. If that's the case this lambda is a non-op.
     */
    auto u8str = [](const auto * p){return std::string(reinterpret_cast<const char *>(p));};
    file->createAttribute<std::string>("program", "libreadout");
    file->createAttribute<std::string>("version", u8str(libreadout::version::version_number));
    file->createAttribute<std::string>("revision", u8str(libreadout::version::git_revision));
    file->createAttribute<std::string>("events", dataset_name);
    dataset->createAttribute("detector", Type);
    dataset->createAttribute("readout", readoutType_from_detectorType(Type));
  }

  void enable_network() {network = true;}
  void disable_network() {network = false;}

private:
  HighFive::CompoundType datatype() const {
    using namespace HighFive;
    switch (readoutType_from_detectorType(Type)){
      case ReadoutType::CAEN: return create_datatype<CAEN_event>();
      case ReadoutType::TTLMonitor: return create_datatype<TTLMonitor_event>();
      case ReadoutType::DREAM: return create_datatype<DREAM_event>();
      case ReadoutType::VMM3: return create_datatype<VMM3_event>();
      default: throw std::runtime_error("Saving this readout type is not implemented yet!");
    }
  }

  void check_size_and_send();

  // setup socket for transmission
  void sockOpen(const std::string& addr, int remote_port);
//  void commandOpen(std::string ipaddr, int port);

  // Packet header
  uint32_t phi{0}; // pulse and prev pulse high and low
  uint32_t plo{0};
  uint32_t pphi{0};
  uint32_t pplo{0};

  uint32_t lasthi{0};
  uint32_t lastlo{0};

  int SeqNum{0};
  int OutputQueue{0};
  DetectorType Type;

  // TX Buffer
  PacketHeaderV0 *hp{};
  char buffer[9000]{};
  const int MaxDataSize{8950};
  int DataSize{0};
  // IP and port number
  std::string ipaddr;
  int port{9000};
  // BSD Socket specifics
  int fd{}; // socket file descriptor
  struct sockaddr_in remoteSockAddr{};

  int tcp_port{8888};

  int verbosity{0};

  std::optional<HighFive::File> file{std::nullopt};
  std::optional<HighFive::DataSet> dataset{std::nullopt};
  bool network{true};
  efu_time period, time;
};
