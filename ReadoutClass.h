// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief UDP readout generator class
///
//===----------------------------------------------------------------------===//
#pragma once

#include "Structs.h"
#include "Readout.h"
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <string>
#include <sys/socket.h>

enum class Verbosity {silent, errors, warnings, info, details};
enum DetectorType {Reserved = 0x00, TTLMonitor = 0x10, LOKI = 0x30, BIFROST = 0x34, MIRACLES = 0x38, CSPEC = 0x40,
                   NMX = 0x44, FREIA = 0x48, TREX = 0x50, DREAM = 0x60, MAGIC = 0x64};
enum class ReadoutType {TTLMonitor, CAEN, VMM3, DREAM, MAGIC};

DetectorType detectorType_from_int(int);

class Readout {
public:
  Readout(std::string IpAddress, int UDPPort, int TCPPort, int Type=0x34)
      : ipaddr(IpAddress), port(UDPPort), tcp_port(TCPPort), Type(detectorType_from_int(Type)) {
    sockOpen(ipaddr, port);
  }



  // Adds a readout to the transmission buffer.
  // If there is no room left, transmit and initialize a new packet
  void addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, const void * data);
  // Specializations for handled data types
  void addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, const CAEN_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, const TTLMonitor_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, const DREAM_readout_t * data);
  void addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, const VMM3_readout_t * data);

  // send the current data buffer
  int send();

  // Update the pulse and previous pulse times
  void setPulseTime(uint32_t PHI, uint32_t PLO, uint32_t PPHI, uint32_t PPLO);

  // Initialize a new packet with no readouts
  void newPacket();

  // Tell the (remote) device to shut down
  int command_shutdown();

  // Set verbosity via enum
  int verbose(Verbosity v){
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
  int verbose(int v){verbosity = v; return verbosity;}

private:
  void check_size_and_send();

  // setup socket for transmission
  void sockOpen(const std::string& ipaddr, int remote_port);
//  void commandOpen(std::string ipaddr, int port);

  // Packet header
  uint32_t phi{0}; // pulse and prev pulse high and low
  uint32_t plo{0};
  uint32_t pphi{0};
  uint32_t pplo{0};

  int SeqNum{0};
  int OutputQueue{0};
  DetectorType Type;

  // TX Buffer
  PacketHeaderV0 *hp;
  char buffer[9000];
  const int MaxDataSize{8950};
  int DataSize{0};
  // IP and port number
  std::string ipaddr;
  int port{9000};
  // BSD Socket specifics
  int fd; // socket file descriptor
  struct sockaddr_in remoteSockAddr;

  int tcp_port{8888};

  int verbosity{0};
};
