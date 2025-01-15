// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief ESS and Bifrost readout data structures
///
//===----------------------------------------------------------------------===//
#pragma once

#ifdef __cplusplus
#include <cinttypes>
#else
#include <inttypes.h>
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif


// Header common to all ESS readout data
// Reviewed ICD (version 2) packet header version 0
// ownCloud: https://project.esss.dk/owncloud/index.php/s/DWNer23727TiI1x
PACK(struct PacketHeaderV0 {
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
     });

// Consistent with BIFROST ICD
// unused uint32_t at end replaced by C & D to extend to LOKI?
PACK(struct CaenData {
       uint8_t Ring;
       uint8_t FEN;
       uint16_t Length;
       uint32_t TimeHigh;
       uint32_t TimeLow;
       uint8_t OMFlag;
       uint8_t Tube;
       uint16_t SeqNum;
       uint16_t AmplA;
       uint16_t AmplB;
       uint16_t AmplC;
       uint16_t AmplD;
     });

// From TTLMon ICD
// TBD
PACK(struct TTLMonitorData {
       uint8_t Ring;
       uint8_t FEN;
       uint16_t Length;
       uint32_t TimeHigh;
       uint32_t TimeLow;
       uint8_t Pos;
       uint8_t Channel;
       uint16_t ADC;
     });

// From EFU src/modules/dream/readout/DataParser.h
PACK(struct DreamData {
       uint8_t Ring;
       uint8_t FEN;
       uint16_t Length;
       uint32_t TimeHigh;
       uint32_t TimeLow;
       uint8_t OM;
       uint8_t Unused;
       uint8_t Cathode;
       uint8_t Anode;
     });

// from EFU src/common/readout/vmm3/VMM3Parser.h
// Used by: FREIA, NMX, TREX
PACK(struct VMM3Data {
       uint8_t Ring;
       uint8_t FEN;
       uint16_t Length;
       uint32_t TimeHigh;
       uint32_t TimeLow;
       uint16_t BC;
       uint16_t OTADC;
       uint8_t GEO;
       uint8_t TDC;
       uint8_t VMM;
       uint8_t Channel;
     });
