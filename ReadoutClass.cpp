// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief ESS UDP readout generator class implementation
///
//===----------------------------------------------------------------------===//
#include "ReadoutClass.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <algorithm>

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

void Readout::setPulseTime(uint32_t PHI, uint32_t PLO, uint32_t PPHI, uint32_t PPLO) {
  phi = PHI;
  plo = PLO;
  pphi = PPHI;
  pplo = PPLO;
}

void Readout::newPacket() {
  memset(buffer, 0x00, sizeof(buffer));
  hp->Padding0 = 0;
  hp->Version = 0;
  hp->CookieAndType = (Type << 24) + 0x535345;
  hp->OutputQueue = OutputQueue;
  hp->TotalLength = sizeof(struct PacketHeaderV0);
  hp->SeqNum = SeqNum++;
  hp->TimeSource = 0;
  hp->PulseHigh = phi;
  hp->PulseLow = plo;
  hp->PrevPulseHigh = pphi;
  hp->PrevPulseLow = pplo;
  DataSize = sizeof(struct PacketHeaderV0);
}

void Readout::sockOpen(const std::string& addr, int remote_port) {
  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0) {
    throw std::runtime_error("socket() failed");
  }

  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(remote_port);

  int ret = inet_aton(addr.c_str(), &remoteSockAddr.sin_addr);
  if (ret == 0) {
    if (verbosity > -1) printf("setRemoteSocket(): invalid ip address %s", addr.c_str());
    throw std::runtime_error("sock_open() failed");
  }

  hp = (PacketHeaderV0 *)&buffer[0];
}

int Readout::addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh, uint32_t TimeLow, uint8_t Tube, uint16_t AmplA, uint16_t AmplB) {
  if (DataSize >= MaxDataSize) {
    send();
    newPacket();
  }
  if (verbosity > 2){
    std::cout << "Add to the packet " << unsigned(Ring) << " " << unsigned(FEN) << " ";
    std::cout << TimeHigh << " " << TimeLow << " " << unsigned(Tube) << " " << AmplA;
    std::cout << " " << AmplB << std::endl;
  }
  auto *dp = (struct CaenData *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct CaenData);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->Tube = Tube;
  dp->AmplA = AmplA;
  dp->AmplB = AmplB;

  DataSize += dp->Length;
  hp->TotalLength = DataSize;
  return 0;
}

int Readout::send() {
  char addr_buffer[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &remoteSockAddr.sin_addr.s_addr, addr_buffer, sizeof(addr_buffer));
  // convert the port number for 'network byte order' to host byte order
  auto addr_port = ntohs(remoteSockAddr.sin_port);
  if (verbosity > 1) {
    std::cout << "Send the packet, to " << addr_buffer << ":" << addr_port << std::endl;
  }

  auto ret = static_cast<int>(sendto(
      fd, (char *)buffer, DataSize, SEND_FLAGS, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr)));
  if (ret < 0 && verbosity > -1) {
    printf("socket sendto() failed: returns %d\n", ret);
  }
  return ret;
}

int check_and_send_tcp(const char* addr, const char* port, const char* msg, const int verbosity){
  int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    if (verbosity > -1) std::cout << "failed to open socket for command" << std::endl;
    return -1;
  }
  struct addrinfo hints{}, *results, *result;
  memset(&hints, 0, sizeof(hints));

  int s = getaddrinfo(addr, port, &hints, &results);
  if (s != 0){
    if (verbosity > -1) std::cout << "getaddrinfo: error for " << addr << std::endl;
    return -2;
  }
  int sfd;

  for (result = results; result != nullptr; result = result->ai_next){
    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1) continue;
    if (connect(sfd, result->ai_addr, result->ai_addrlen) != -1) break;
    close(sfd);
  }
  freeaddrinfo(results);
  if (result == nullptr) return 0;

  if (msg != nullptr) {
    if (write(sfd, msg, strlen(msg)) != strlen(msg)) {
      if (verbosity > -1) std::cout << "Failed to send (full) message " << msg << std::endl;
      return -1;
    }
    char buf[500];
    auto n = read(sfd, buf, 500);
    if (n == -1) return 0;
    if (verbosity > 1) {
      std::string str(msg);
      str.erase(std::remove(str.begin(), str.end(), '\n'), str.cend());
      std::cout << "Response to message '" << str << "': " << buf << std::endl;
    }
  }
  return 1;
}

int Readout::command_shutdown() {
  char tcp[10];
  sprintf(tcp, "%d", tcp_port);
  int ok = check_and_send_tcp(ipaddr.c_str(), tcp, "EXIT\n", verbosity);
  if (ok < 0) {
    if (verbosity > -1) std::cout << "Could not connect to " << ipaddr << ":" << tcp_port << std::endl;
    return -3;
  }
  if (ok == 0) {
    if (verbosity > -1) std::cout << "Could not read response from EXIT command" << std::endl;
    return -2;
  }
  // ok == 1 means the connection succeeded and the response was read.
  // Let's now check that the EXIT command was executed
  ok *= 100; // try up to 100 times
  while (ok-- > 0){
    ok = check_and_send_tcp(ipaddr.c_str(), tcp, nullptr, verbosity);
  }
  if (ok > 0 && verbosity > 0){
    // the server is still alive.
    std::cout << "The server is still alive after a successful EXIT command" << std::endl;
  }
  return 0;
}
