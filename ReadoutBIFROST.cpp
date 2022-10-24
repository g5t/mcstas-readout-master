#include <string>

#include "BifrostReadout.h"
#include "ReadoutBIFROST.h"


#ifdef __cplusplus
extern "C" {
#endif
  struct bifrost_readout{
    void *obj;
  };

  // Create a new BIFROST Readout object
  bifrost_readout_t * bifrost_readout_create(char* address, int port){
    bifrost_readout_t* br;
    BifrostReadout *obj;

    std::string string_address(address);

    br = (typeof(br))malloc(sizeof(*br));
    obj = new BifrostReadout(string_address, port);
    br->obj = obj;

    return br;
  }

  // Destroy an existing BIFROST Readout object
  void bifrost_readout_destroy(bifrost_readout_t* br){
    if (br == NULL) return;
    delete static_cast<BifrostReadout*>(br->obj);
    free(br);
  }

  // Add a readout value to the transmission buffer of the BIFROST Readout object
  // Automatically transmits the packet if it is full.
  void bifrost_readout_add(
      bifrost_readout_t* br, uint8_t ring, uint8_t fen, 
      uint32_t time_high, uint32_t time_low,
      uint8_t tube, uint16_t amplitude_a, uint16_t amplitude_b
  )
  {
    BifrostReadout* obj;
    if (br == NULL) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->addReadout(ring, fen, time_high, time_low, tube, amplitude_a, amplitude_b);
  }

  // Send the current data buffer for the BIFROST Readout object
  void bifrost_readout_send(bifrost_readout_t* br)
  {
    BifrostReadout* obj;
    if (br == NULL) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->send();
  }
  // Update the pulse and previous pulse times for the BIFROST Readout object
  void bifrost_readout_setPulseTime(
      bifrost_readout_t* br, uint32_t pulse_high, uint32_t pulse_low, 
      uint32_t prev_high, uint32_t prev_low
  )
  {
    BifrostReadout* obj;
    if (br == NULL) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->setPulseTime(pulse_high, pulse_low, prev_high, prev_low);
  }
  // Initialize a new packet with no readouts for the BIFROST Readout object
  void bifrost_readout_newPacket(bifrost_readout_t* br)
  {
    BifrostReadout* obj;
    if (br == NULL) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->newPacket();
  }

  // 
#ifdef __cplusplus
}
#endif
