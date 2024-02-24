#ifndef READOUT_WRAPPER
#define READOUT_WRAPPER

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#endif

struct CAEN_readout {
  uint8_t caen_readout_channel;
  uint16_t caen_readout_a;
  uint16_t caen_readout_b;
  uint16_t caen_readout_c;
  uint16_t caen_readout_d;
};
typedef struct CAEN_readout CAEN_readout_t;

struct TTLMonitor_readout {
  uint8_t ttlmonitor_readout_channel;
  uint8_t ttlmonitor_readout_pos;
  uint16_t ttlmonitor_readout_adc;
};
typedef struct TTLMonitor_readout TTLMonitor_readout_t;

struct DREAM_readout {
  uint8_t dream_readout_om;
  uint8_t dream_readout_cathode;
  uint8_t dream_readout_anode;
};
typedef struct DREAM_readout DREAM_readout_t;

struct VMM3_readout {
  uint16_t vmm3_readout_bc;
  uint16_t vmm3_readout_otadc;
  uint8_t vmm3_readout_geo;
  uint8_t vmm3_readout_tdc;
  uint8_t vmm3_readout_vmm;
  uint8_t vmm3_readout_channel;
};
typedef struct VMM3_readout VMM3_readout_t;


  struct readout;
  typedef struct readout readout_t;

  // Create a new Readout object
  // type == 0x34 for BIFROST, 0x41 for He3CSPEC
  readout_t * readout_create(const char* address, int port, int command_port, double source_frequency, int type);

  // Destroy an existing Readout object
  void readout_destroy(readout_t* r_ptr);

  // Add a readout value to the transmission buffer of the Readout object
  // Automatically transmits the packet if it is full.
//  void readout_add(
//      readout_t* r_ptr, uint8_t ring, uint8_t fen,
//      double time_of_flight,
//      uint8_t channel, uint16_t a, uint16_t b
//  );
  void readout_add(readout_t* r_ptr, uint8_t ring, uint8_t fen, double time_of_flight, const void* data);
  // Send the current data buffer for the Readout object
  void readout_send(readout_t* r_ptr);
  // Update the pulse and previous pulse times for the Readout object
  void readout_setPulseTime(readout_t* r_ptr);

  // Initialize a new packet with no readouts for the Readout object
  void readout_newPacket(readout_t* r_ptr);

  // Send the command-port of the Event Formation Unit the exit signal
  int readout_shutdown(readout_t* r_ptr);

  // Set the verbose level of the readout sender to emit nothing to standard output
  int readout_silent(readout_t* r_ptr);
  // Set the verbose level of the readout sender to emit extra error messages to standard output
  int readout_print_errors(readout_t* r_ptr);
  // Set the verbose level of the readout sender to emit warnings and extra error messages to standard output
  int readout_print_warnings(readout_t* r_ptr);
  // Set the verbose level of the readout sender to emit info, warnings and extra error messages to standard output
  int readout_print_info(readout_t* r_ptr);
  // Set the verbose level of the readout sender to emit extra detail messages to standard output
  int readout_print_details(readout_t* r_ptr);
  // Set the verbose level from an integer -- look at ReadoutClass.h
  int readout_verbose(readout_t* r_ptr, int);

#ifdef __cplusplus
}
#endif

#endif
