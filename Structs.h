// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief ESS and Bifrost readout data structures
///
//===----------------------------------------------------------------------===//
#pragma once

#include <inttypes.h>

// Header common to all ESS readout data
// Reviewed ICD (version 2) packet header version 0
// ownCloud: https://project.esss.dk/owncloud/index.php/s/DWNer23727TiI1x
struct PacketHeaderV0 {
  uint8_t Padding0;
  uint8_t Version;
  uint32_t CookieAndType;
  uint16_t TotalLength;
  uint8_t OutputQueue;
  uint8_t TimeSource;
  uint32_t PulseHigh;
  uint32_t PulseLow;
  uint32_t PrevPulseHigh;
  uint32_t PrevPulseLow;
  uint32_t SeqNum;
} __attribute__((packed));

// As per BIFROST ICD -- matches all CAEN readout instruments?
struct CaenData {
  uint8_t Ring;
  uint8_t FEN;
  uint16_t Length;
  uint32_t TimeHigh;
  uint32_t TimeLow;
  uint8_t OMFlag;
  uint8_t Tube;
  uint16_t Unused16;
  uint16_t AmplA;
  uint16_t AmplB;
  uint32_t Unused32;
} __attribute__((packed));
