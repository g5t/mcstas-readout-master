#ifndef READOUT_WRAPPER
#define READOUT_WRAPPER

#ifdef __cplusplus
extern "C" {
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

  typedef struct CAEN_readout CAEN_readout_t;
  typedef struct TTLMonitor_readout TTLMonitor_readout_t;
  typedef struct DREAM_readout DREAM_readout_t;
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
  void readout_add(readout_t* r_ptr, uint8_t ring, uint8_t fen, double time_of_flight, double weight, const void* data);
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

  // Control file output for the Readout object
  void readout_dump_to(readout_t * r_ptr, const char * filename);

  // Allow disabling and enabling network communication (on by default)
  void readout_disable_network(readout_t * r_ptr);
  void readout_enable_network(readout_t * r_ptr);

#ifdef __cplusplus
}
#endif

#endif
