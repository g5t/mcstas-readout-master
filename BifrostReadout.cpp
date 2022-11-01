// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Bifrost UDP readout generator class implementation
///
//===----------------------------------------------------------------------===//
#include "BifrostReadout.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

void BifrostReadout::setPulseTime(uint32_t PHI, uint32_t PLO, uint32_t PPHI,
                                  uint32_t PPLO) {
  phi = PHI;
  plo = PLO;
  pphi = PPHI;
  pplo = PPLO;
}

void BifrostReadout::newPacket() {
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
  return;
}

void BifrostReadout::sockOpen(std::string ipaddr, int port) {
  fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0) {
    throw std::runtime_error("socket() failed");
  }

  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(port);

  int ret = inet_aton(ipaddr.c_str(), &remoteSockAddr.sin_addr);
  if (ret == 0) {
    printf("setRemoteSocket(): invalid ip address %s", ipaddr.c_str());
    throw std::runtime_error("sock_open() failed");
  }

  hp = (PacketHeaderV0 *)&buffer[0];
}

int BifrostReadout::addReadout(uint8_t Ring, uint8_t FEN, uint32_t TimeHigh,
                               uint32_t TimeLow, uint8_t Tube, uint16_t AmplA,
                               uint16_t AmplB) {
  if (DataSize >= MaxDataSize) {
    send();
    newPacket();
  }
  struct BifrostData *dp = (struct BifrostData *)(buffer + DataSize);
  dp->Ring = Ring;
  dp->FEN = FEN;
  dp->Length = sizeof(struct BifrostData);
  dp->TimeHigh = TimeHigh;
  dp->TimeLow = TimeLow;
  dp->Tube = Tube;
  dp->AmplA = AmplA;
  dp->AmplB = AmplB;

  DataSize += dp->Length;
  hp->TotalLength = DataSize;
  return 0;
}

int BifrostReadout::send() {
  int ret = sendto(fd, (char *)buffer, DataSize, SEND_FLAGS,
                   (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    printf("socket sendto() failed: returns %d\n", ret);
  }
  return ret;
}
