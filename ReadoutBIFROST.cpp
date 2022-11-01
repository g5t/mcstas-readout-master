#include <string>

#include "BifrostReadout.h"
#include "ReadoutBIFROST.h"
#include "efu_time.h"

#ifdef __cplusplus
extern "C" {
#endif
  struct bifrost_readout{
    void *obj;
    void *rep;
    void *time;
  };

  void bifrost_readout_setFirstPulseTime(bifrost_readout_t * br)
  {
    efu_time * rep, * pulse;
    if (br == nullptr) return;
    rep = static_cast<efu_time*>(br->rep);
    pulse = static_cast<efu_time*>(br->time);
    auto prev = *pulse - rep;
    auto obj = static_cast<BifrostReadout*>(br->obj);
    obj->setPulseTime(pulse->high(), pulse->low(), prev.high(), prev.low());
    obj->newPacket();
  }

  // Create a new BIFROST Readout object
  bifrost_readout_t * bifrost_readout_create(char* address, int port, double source_frequency){
    bifrost_readout_t* br;
    BifrostReadout *obj;
    efu_time *rep, *time;

    std::string string_address(address);

    br = (typeof(br))malloc(sizeof(*br));
    br->obj = new BifrostReadout(string_address, port);
    br->rep = new efu_time(1/source_frequency);
    br->time = new efu_time();

    bifrost_readout_setFirstPulseTime(br);
    return br;
  }

  // Destroy an existing BIFROST Readout object
  void bifrost_readout_destroy(bifrost_readout_t* br){
    if (br == nullptr) return;
    delete static_cast<BifrostReadout*>(br->obj);
    delete static_cast<efu_time*>(br->rep);
    delete static_cast<efu_time*>(br->time);
    free(br);
  }

  // Add a readout value to the transmission buffer of the BIFROST Readout object
  // Automatically transmits the packet if it is full.
  void bifrost_readout_add(
      bifrost_readout_t* br, uint8_t ring, uint8_t fen, 
      double time_of_flight,
      uint8_t tube, uint16_t amplitude_a, uint16_t amplitude_b
  )
  {
    bifrost_readout_setPulseTime(br);
    BifrostReadout* obj;
    if (br == nullptr) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    auto tof = efu_time(time_of_flight);
    /* The ReadoutMaster would not know if there was more than one frame during
     * the neutron's flight time:
     *  - Find the time from the most-recent pulse time, assuming no moderator
     *    emission time (this is already unreasonable)
     *  - Add the last pulse time to get a wall-clock time
    */
    // tof = (tof % (static_cast<efu_time*>(br->rep))) + static_cast<efu_time*>(br->time);
    /* At present, simulations focus only on the secondary spectrometer with
     * likely-unphysical energy bandwidth neutrons, so there is no way to work
     * out the number of source periods necessary to add to the event time minus
     * the last pulse time in order to recover the true time of flight.
     * Until such time as the simulations include the primary spectrometer with
     * physically relevant choppers, chopper speeds, and chopper phases, the
     * event time reported should be the 'true' ToF plus the last pulse time.
     * */
    tof = tof + static_cast<efu_time*>(br->time);
    obj->addReadout(ring, fen, tof.high(), tof.low(), tube, amplitude_a, amplitude_b);
  }

  // Send the current data buffer for the BIFROST Readout object
  void bifrost_readout_send(bifrost_readout_t* br)
  {
    BifrostReadout* obj;
    if (br == nullptr) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->send();
  }
  // Update the pulse and previous pulse times for the BIFROST Readout object
  void bifrost_readout_setPulseTime(bifrost_readout_t* br)
  {
    if (br == nullptr) return;
    auto obj = static_cast<BifrostReadout*>(br->obj);
    auto prev = static_cast<efu_time*>(br->time);
    auto rep = static_cast<efu_time*>(br->rep);
    auto now = new efu_time();
    if ((*now - prev) >= rep){
      std::cout << "Send out packet and update time! From " << prev << " to " << now << std::endl;
      *now = *prev + *rep * ((*now - prev) / *rep);
      std::cout << "Modified updated time " << now << std::endl;
      obj->send();
      obj->setPulseTime(now->high(), now->low(), prev->high(), prev->low());
      obj->newPacket();
      br->time = now;
      delete prev;
    } else {
      delete now;
    }
  }

  // Initialize a new packet with no readouts for the BIFROST Readout object
  void bifrost_readout_newPacket(bifrost_readout_t* br)
  {
    BifrostReadout* obj;
    if (br == nullptr) return;
    obj = static_cast<BifrostReadout*>(br->obj);
    obj->newPacket();
  }

  // 
#ifdef __cplusplus
}
#endif
