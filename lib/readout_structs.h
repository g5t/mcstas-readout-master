#pragma once
#ifdef __cplusplus
#include <cstdint>
#endif

struct CAEN_readout {
  uint8_t channel;
  uint16_t a;
  uint16_t b;
  uint16_t c;
  uint16_t d;
};

struct TTLMonitor_readout {
  uint8_t channel;
  uint8_t pos;
  uint16_t adc;
};

struct DREAM_readout {
  uint8_t om;
  uint8_t cathode;
  uint8_t anode;
};

struct VMM3_readout {
  uint16_t bc;
  uint16_t otadc;
  uint8_t geo;
  uint8_t tdc;
  uint8_t vmm;
  uint8_t channel;
};
