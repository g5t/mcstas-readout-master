#ifndef READOUT_WRAPPER
#define READOUT_WRAPPER

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif
  struct readout;
  typedef struct readout readout_t;

  // Create a new Readout object
  // type == 0x34 for BIFROST, 0x41 for He3CSPEC
  readout_t * readout_create(char* address, int port, int command_port, double source_frequency, int type);

  // Destroy an existing Readout object
  void readout_destroy(readout_t* r_ptr);

  // Add a readout value to the transmission buffer of the Readout object
  // Automatically transmits the packet if it is full.
  void readout_add(
      readout_t* r_ptr, uint8_t ring, uint8_t fen,
      double time_of_flight,
      uint8_t tube, uint16_t amplitude_a, uint16_t amplitude_b
  );
  // Send the current data buffer for the Readout object
  void readout_send(readout_t* r_ptr);
  // Update the pulse and previous pulse times for the Readout object
  void readout_setPulseTime(readout_t* r_ptr);

  // Initialize a new packet with no readouts for the Readout object
  void readout_newPacket(readout_t* r_ptr);

  // Send the command-port of the Event Formation Unit the exit signal
  int readout_shutdown(readout_t* r_ptr);

#ifdef __cplusplus
}
#endif

#endif
