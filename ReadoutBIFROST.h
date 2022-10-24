#ifndef __BIFROST_READOUT_WRAPPER
#define __BIFROST_READOUT_WRAPPER

#ifdef __cplusplus
extern "C" {
#endif
  struct bifrost_readout;
  typedef struct bifrost_readout bifrost_readout_t;

  // Create a new BIFROST Readout object
  bifrost_readout_t * bifrost_readout_create(char* address, int port);

  // Destroy an existing BIFROST Readout object
  void bifrost_readout_destroy(bifrost_readout_t* br);

  // Add a readout value to the transmission buffer of the BIFROST Readout object
  // Automatically transmits the packet if it is full.
  void bifrost_readout_add(
      bifrost_readout_t* br, uint8_t ring, uint8_t fen, 
      uint32_t time_high, uint32_t time_low,
      uint8_t tube, uint16_t amplitude_a, uint16_t amplitude_b
  );
  // Send the current data buffer for the BIFROST Readout object
  void bifrost_readout_send(bifrost_readout_t* br);
  // Update the pulse and previous pulse times for the BIFROST Readout object
  void bifrost_readout_setPulseTime(
      bifrost_readout_t* br, uint32_t pulse_high, uint32_t pulse_low, 
      uint32_t prev_high, uint32_t prev_low
  );
  // Initialize a new packet with no readouts for the BIFROST Readout object
  void bifrost_readout_newPacket(bifrost_readout_t* br);

#ifdef __cplusplus
}
#endif

#endif
