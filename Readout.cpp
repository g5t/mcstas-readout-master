#include <string>

#include "Readout.h"
#include "ReadoutClass.h"
#include "efu_time.h"

#ifdef __cplusplus
extern "C" {
#endif
  struct readout{
    void *obj;
    void *rep;
    void *time;
  };

  void readout_setFirstPulseTime(readout_t * r_ptr)
  {
    efu_time * rep, * pulse;
    if (r_ptr == nullptr) return;
    rep = static_cast<efu_time*>(r_ptr->rep);
    pulse = static_cast<efu_time*>(r_ptr->time);
    auto prev = *pulse - rep;
    auto obj = static_cast<Readout*>(r_ptr->obj);
    obj->setPulseTime(pulse->high(), pulse->low(), prev.high(), prev.low());
    obj->newPacket();
  }

  // Create a new Readout object
  readout_t * readout_create(char* address, int port, int command_port, double source_frequency, int type){
    readout_t* r_ptr;
//    Readout *obj;
//    efu_time *rep, *time;

    std::string string_address(address);

    r_ptr = (typeof(r_ptr))malloc(sizeof(*r_ptr));
    r_ptr->obj = new Readout(string_address, port, command_port, type);
    r_ptr->rep = new efu_time(1/source_frequency);
    r_ptr->time = new efu_time();

    readout_setFirstPulseTime(r_ptr);
    return r_ptr;
  }

  // Destroy an existing Readout object
  void readout_destroy(readout_t* r_ptr){
    if (r_ptr == nullptr) return;
    delete static_cast<Readout*>(r_ptr->obj);
    delete static_cast<efu_time*>(r_ptr->rep);
    delete static_cast<efu_time*>(r_ptr->time);
    free(r_ptr);
  }

  // Add a readout value to the transmission buffer of the Readout object
  // Automatically transmits the packet if it is full.
//  void readout_add(
//      readout_t* r_ptr, uint8_t ring, uint8_t fen,
//      double time_of_flight,
//      uint8_t tube, uint16_t amplitude_a, uint16_t amplitude_b
//  )
//  {
//    //std::cout << "Adding readout to ReadoutMaster with " << unsigned(ring) << ", " << unsigned(fen);
//    //std::cout << ", " << time_of_flight << ", " << unsigned(channel) << ", ";
//    //std::cout << a << ", " << b << std::endl;
//    readout_setPulseTime(r_ptr);
//    Readout* obj;
//    if (r_ptr == nullptr) return;
//    obj = static_cast<Readout*>(r_ptr->obj);
//    auto tof = efu_time(time_of_flight);
//    /* The ReadoutMaster would not know if there was more than one frame during
//     * the neutron's flight time:
//     *  - Find the time from the most-recent pulse time, assuming no moderator
//     *    emission time (this is already unreasonable)
//     *  - Add the last pulse time to get a wall-clock time
//    */
//    // tof = (tof % (static_cast<efu_time*>(r_ptr->rep))) + static_cast<efu_time*>(r_ptr->time);
//    /* At present, simulations focus only on the secondary spectrometer with
//     * likely-unphysical energy bandwidth neutrons, so there is no way to work
//     * out the number of source periods necessary to add to the event time minus
//     * the last pulse time in order to recover the true time of flight.
//     * Until such time as the simulations include the primary spectrometer with
//     * physically relevant choppers, chopper speeds, and chopper phases, the
//     * event time reported should be the 'true' ToF plus the last pulse time.
//     * */
//    tof = tof + static_cast<efu_time*>(r_ptr->time);
//    obj->addReadout(ring, fen, tof.high(), tof.low(), tube, amplitude_a, amplitude_b);
//  }

  void readout_add(readout_t * r_ptr, uint8_t ring, uint8_t fen, double time_of_flight, const void * data){
    readout_setPulseTime(r_ptr);
    Readout * obj;
    if (r_ptr == nullptr) return;
    obj = static_cast<Readout *>(r_ptr->obj);
    auto tof = efu_time(time_of_flight);
    // the real data should have tof modulo the source period
    tof = tof + static_cast<efu_time*>(r_ptr->time);
    obj->addReadout(ring, fen, tof.high(), tof.low(), data);
  }

  // Send the current data buffer for the Readout object
  void readout_send(readout_t* r_ptr)
  {
    Readout* obj;
    if (r_ptr == nullptr) return;
    obj = static_cast<Readout*>(r_ptr->obj);
    obj->send();
  }
  // Update the pulse and previous pulse times for the Readout object
  void readout_setPulseTime(readout_t* r_ptr)
  {
    if (r_ptr == nullptr) return;
    auto obj = static_cast<Readout*>(r_ptr->obj);
    auto prev = static_cast<efu_time*>(r_ptr->time);
    auto rep = static_cast<efu_time*>(r_ptr->rep);
    auto now = new efu_time();
    if ((*now - prev) >= rep){
//      std::cout << "Send out packet and update time! From " << prev << " to " << now << std::endl;
      *now = *prev + *rep * ((*now - prev) / *rep);
      // The ESS Caen EFUs require (now - prev) <= 5 * rep; so we should fake it
      if ((*now - prev) > (*rep * 5u)){
        *prev = *now - *rep;
      }
//      std::cout << "Modified updated time " << now << std::endl;
      obj->send();
      obj->setPulseTime(now->high(), now->low(), prev->high(), prev->low());
      obj->newPacket();
      r_ptr->time = now;
      delete prev;
    } else {
      delete now;
    }
  }

  // Initialize a new packet with no readouts for the Readout object
  void readout_newPacket(readout_t* r_ptr)
  {
    Readout* obj;
    if (r_ptr == nullptr) return;
    obj = static_cast<Readout*>(r_ptr->obj);
    obj->newPacket();
  }

  int readout_shutdown(readout_t* r_ptr)
  {
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->command_shutdown();
  }

  // Set the verbose level of the readout sender to emit nothing to standard output
  int readout_silent(readout_t* r_ptr){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(Verbosity::silent);
  }
  // Set the verbose level of the readout sender to emit extra error messages to standard output
  int readout_print_errors(readout_t* r_ptr){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(Verbosity::errors);
  }
  // Set the verbose level of the readout sender to emit warnings and extra error messages to standard output
  int readout_print_warnings(readout_t* r_ptr){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(Verbosity::warnings);
  }
  // Set the verbose level of the readout sender to emit info, warnings and extra error messages to standard output
  int readout_print_info(readout_t* r_ptr){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(Verbosity::info);
  }
  // Set the verbose level of the readout sender to emit extra detail messages to standard output
  int readout_print_details(readout_t* r_ptr){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(Verbosity::details);
  }

  // Set the verbose level from an integer -- look at ReadoutClass.h
  int readout_verbose(readout_t* r_ptr, int v){
    Readout* obj;
    if (r_ptr == nullptr) return 0;
    obj = static_cast<Readout*>(r_ptr->obj);
    return obj->verbose(v);
  }


  // 
#ifdef __cplusplus
}
#endif
