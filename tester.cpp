#include "Readout.h"

int main(){
  // readout_t * bifrost_readout_create(char* address, int port, double source_frequency, int type);
  char addr[] = "127.0.0.1";
  auto ro = readout_create(addr, 9000, 8888, 1/14., 0x34);
  
  readout_newPacket(ro);

  uint16_t max = 1000;
  for (uint16_t i=0; i<max; ++i){
    uint8_t ring = 1;
    uint8_t fen = 0;
    uint8_t tube = 3;
    double tof = static_cast<double>(i)/static_cast<double>(max);
    readout_add(ro, ring, fen, tof, tube, i, max-i);
  }
  readout_send(ro);

  readout_destroy(ro);
  return 0;
}
