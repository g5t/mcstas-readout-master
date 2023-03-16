// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief UDP readout generator class
///
//===----------------------------------------------------------------------===//
#pragma once

#include "Structs.h"
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <string>
#include <sys/socket.h>

enum class Verbosity {silent, errors, warnings, info, details};

class Readout {
public:
  Readout(std::string IpAddress, int UDPPort, int TCPPort, int Type=0x34)
      : ipaddr(IpAddress), port(UDPPort), tcp_port(TCPPort), Type(Type) {
    sockOpen(ipaddr, port);
  }

  // Adds a readout to the transmission buffer.
  // If there is no room left, transmit and initialize a new packet
  int addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow,
                 uint8_t Tube, uint16_t AmplA, uint16_t AmplB);

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
  int Type; // 0x34 for BIFROST, 0x41 for He3CSPEC

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
