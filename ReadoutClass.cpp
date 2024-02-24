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
#include <tuple>

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

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

void Readout::setPulseTime(const uint32_t PHI, const uint32_t PLO, const uint32_t PPHI, const uint32_t PPLO) {
  phi = PHI;
  plo = PLO;
  pphi = PPHI;
  pplo = PPLO;
}

std::pair<uint32_t, uint32_t> Readout::lastPulseTime() const {
  return std::make_pair(phi, plo);
}
std::pair<uint32_t, uint32_t> Readout::prevPulseTime() const {
  return std::make_pair(pphi, pplo);
}
std::pair<uint32_t, uint32_t> Readout::lastEventTime() const {
  return std::make_pair(lasthi, lastlo);
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

static int hostname_to_ip(const char * hostname, char * ip, const int verbosity){
  struct addrinfo *server_info, hints{};
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int rv;
  if ((rv = getaddrinfo(hostname, "http", &hints, &server_info)) != 0){
    if (verbosity > -1) fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    throw std::runtime_error("Hostname resolution failed");
  }
  rv = 1;
  for (struct addrinfo *p=server_info; p != nullptr; p = p->ai_next){
    const auto h = (struct sockaddr_in *) p->ai_addr;
    if (h->sin_addr.s_addr) {
      strcpy(ip, inet_ntoa(h->sin_addr));
      rv = 0;
    }
  }
  freeaddrinfo(server_info);
  return rv;
}

void Readout::sockOpen(const std::string& addr, const int remote_port) {
  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0) {
    throw std::runtime_error("socket() failed");
  }

  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(remote_port);

  // Attempt to resolve the address, in case it is a FQDN and not a string-encode IP address:
  char ip[100];
  if (hostname_to_ip(addr.c_str(), ip, verbosity)) {
    if (verbosity > -1) printf("Failed to identify a non-zero IP address for %s\n", addr.c_str());
    throw std::runtime_error("sockOpen() failed due to 0.0.0.0 IP address");
  }

  const int ret = inet_aton(ip, &remoteSockAddr.sin_addr);
  if (ret == 0) {
    if (verbosity > -1) printf("sockOpen(): invalid ip address %s", ip);
    throw std::runtime_error("sockOpen() failed");
  }

  hp = (PacketHeaderV0 *)&buffer[0];
}

void Readout::check_size_and_send() {
  if (DataSize >= MaxDataSize) {
    send();
    newPacket();
  }
}

void Readout::addReadout(const uint8_t Ring, const uint8_t FEN, const uint32_t TimeHigh, const uint32_t TimeLow, const CAEN_readout_t *data) {
  check_size_and_send();
  if (verbosity > 2){
    std::cout << "Add to the packet Ring=" << static_cast<unsigned>(Ring) << " FEN=" << static_cast<unsigned>(FEN);
    std::cout << " TimeHigh=" << TimeHigh << " TimeLow=" << TimeLow << " Tube=" << static_cast<unsigned>(data->caen_readout_channel);
    std::cout << " AmplA=" << data->caen_readout_a << " AmplB=" << data->caen_readout_b << std::endl;
  }
  auto *dp = (struct CaenData *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct CaenData);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->Tube = data->caen_readout_channel;
  dp->AmplA = data->caen_readout_a;
  dp->AmplB = data->caen_readout_b;
  dp->AmplC = data->caen_readout_c;
  dp->AmplD = data->caen_readout_d;
  DataSize += dp->Length;
  hp->TotalLength = DataSize;
}
void Readout::addReadout(const uint8_t Ring, const uint8_t FEN, const uint32_t TimeHigh, const uint32_t TimeLow, const TTLMonitor_readout_t *data) {
  if (verbosity > 2){
    std::cout << "Add to the packet Ring=" << static_cast<unsigned>(Ring) << " FEN=" << static_cast<unsigned>(FEN);
    std::cout << " TimeHigh=" << TimeHigh << " TimeLow=" << TimeLow << " Pos=" << static_cast<unsigned>(data->ttlmonitor_readout_pos);
    std::cout << " Channel=" << static_cast<unsigned>(data->ttlmonitor_readout_channel) << " ADC=" << data->ttlmonitor_readout_adc << std::endl;
  }
  check_size_and_send();
  auto *dp = (struct TTLMonitorData *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct TTLMonitorData);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->Pos = data->ttlmonitor_readout_pos;
  dp->Channel = data->ttlmonitor_readout_channel;
  dp->ADC = data->ttlmonitor_readout_adc;
  DataSize += dp->Length;
  hp->TotalLength = DataSize;
}


void Readout::addReadout(const uint8_t Ring, const uint8_t FEN, const uint32_t TimeHigh, const uint32_t TimeLow, const DREAM_readout_t *data) {
  check_size_and_send();
  auto *dp = (struct DreamData *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct DreamData);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->OM = data->dream_readout_om;
  dp->Cathode = data->dream_readout_cathode;
  dp->Anode = data->dream_readout_anode;
  DataSize += dp->Length;
  hp->TotalLength = DataSize;
}
void Readout::addReadout(const uint8_t Ring, const uint8_t FEN, const uint32_t TimeHigh, const uint32_t TimeLow, const VMM3_readout_t *data) {
  check_size_and_send();
  auto *dp = (struct VMM3Data *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct VMM3Data);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->BC = data->vmm3_readout_bc;
  dp->OTADC = data->vmm3_readout_otadc;
  dp->GEO = data->vmm3_readout_geo;
  dp->TDC = data->vmm3_readout_tdc;
  dp->VMM = data->vmm3_readout_vmm;
  dp->Channel = data->vmm3_readout_channel;
  DataSize += dp->Length;
  hp->TotalLength = DataSize;
}

void Readout::addReadout(const uint8_t Ring, const uint8_t FEN, const uint32_t TimeHigh, const uint32_t TimeLow, const void *data) {
  lasthi = TimeHigh;
  lastlo = TimeLow;
  const auto type = readoutType_from_detectorType(Type);
  switch (type) {
    case ReadoutType::CAEN: return addReadout(Ring, FEN, TimeHigh, TimeLow, static_cast<const CAEN_readout_t*>(data));
    case ReadoutType::TTLMonitor: return addReadout(Ring, FEN, TimeHigh, TimeLow, static_cast<const TTLMonitor_readout_t*>(data));
    case ReadoutType::DREAM: return addReadout(Ring, FEN, TimeHigh, TimeLow, static_cast<const DREAM_readout_t*>(data));
    case ReadoutType::VMM3: return addReadout(Ring, FEN, TimeHigh, TimeLow, static_cast<const VMM3_readout_t*>(data));
    default: throw std::runtime_error("This readout data type not implemented yet!");
  }
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
  const int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0) {
    if (verbosity > -1) std::cout << "failed to open socket for command" << std::endl;
    return -1;
  }
  struct addrinfo hints{}, *results, *result;
  memset(&hints, 0, sizeof(hints));

  const int s = getaddrinfo(addr, port, &hints, &results);
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

int Readout::command_shutdown() const {
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
