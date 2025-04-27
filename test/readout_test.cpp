#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include "cluon-complete.hpp"

#include <Readout.h>
#include <Structs.h>
#include "test_utils.h"

TEST_CASE("Send and receive CAEN packets","[c][CAEN]"){
  const uint16_t max{1000};
  uint32_t detector_type{0x34};
  int detector_port{9000};
  auto stats = std::make_shared<UDPStats>();

  cluon::UDPReceiver detector_receiver("127.0.0.1", detector_port,
      [stats,detector_type](std::string && data, std::string &&, std::chrono::system_clock::time_point &&) noexcept {;
        // data must contain [PacketHeaderV0, readout, readout, ...].
        auto ptr = data.data();
        auto * header = reinterpret_cast<PacketHeaderV0*>(ptr);
        REQUIRE(header->Padding0 == 0);
        REQUIRE(header->Version == 0);
        auto type = (header->CookieAndType) >> 24;
        auto cookie =  (header->CookieAndType - (type << 24));
        REQUIRE(cookie == 0x535345);  // ESS identifier
        REQUIRE(detector_type == type);
        ptr += sizeof(PacketHeaderV0);
        size_t readout_size =sizeof(struct CaenData);
        auto readouts = (header->TotalLength - sizeof(PacketHeaderV0)) / readout_size;
//        auto stats = Singleton::instance();
        for (size_t i=0; i<readouts; ++i){
          auto *r = reinterpret_cast<CaenData *>(ptr + i * readout_size);
          REQUIRE(r->Ring == 1);
          REQUIRE(r->FEN == 0);
          REQUIRE(r->Tube == 3);
          REQUIRE(r->AmplA == stats->readouts + i);
          REQUIRE(r->AmplB == max - i - stats->readouts);
          REQUIRE(r->AmplC == 0);
          REQUIRE(r->AmplD == 0);
        }
        stats->packets++;
        stats->readouts += readouts;
      });
  REQUIRE(detector_receiver.isRunning());

  char addr[] = "127.0.0.1";
  {
    auto detector_efu = readout_create(addr, detector_port, 8888, 1 / 14., static_cast<int>(detector_type));
    CAEN_readout_t caen_data;
    for (uint16_t i = 0; i < max; ++i) {
      uint8_t ring = 1;
      uint8_t fen = 0;
      uint8_t tube = 3;
      double tof = static_cast<double>(i) / static_cast<double>(max);
      caen_data.channel = tube;
      caen_data.a = i;
      caen_data.b = max - i;
      caen_data.c = 0;
      caen_data.d = 0;
      // Setting the weight to 0, otherwise it is used to send a random number of packets
      readout_add(detector_efu, ring, fen, tof, 0., static_cast<const void *>(&caen_data));
    }
    readout_destroy(detector_efu);
  }
  auto expected = max;
  if (stats->readouts < expected){
    // wait a bit in case the receiver is doing something?
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  REQUIRE(stats->readouts == expected);
}


TEST_CASE("Send and receive TTLMonitor packets","[c]"){
  const uint16_t max{1000};
  uint32_t monitor_type{0x10};
  int monitor_port{9001};
  auto stats = std::make_shared<UDPStats>();

  cluon::UDPReceiver monitor_receiver("127.0.0.1", monitor_port,
    [stats,monitor_type](std::string && data, std::string &&, std::chrono::system_clock::time_point &&) noexcept {
      // data must contain [PacketHeaderV0, readout, readout, ...].
      auto ptr = data.data();
      auto * header = reinterpret_cast<PacketHeaderV0*>(ptr);
      REQUIRE(header->Padding0 == 0);
      REQUIRE(header->Version == 0);
      auto type = (header->CookieAndType) >> 24;
      auto cookie =  (header->CookieAndType - (type << 24));
      REQUIRE(cookie == 0x535345);  // ESS identifier
      REQUIRE(monitor_type == type);
      ptr += sizeof(PacketHeaderV0);
      size_t readout_size = sizeof(struct TTLMonitorData);
      auto readouts = (header->TotalLength - sizeof(PacketHeaderV0)) / readout_size;
      for (size_t i=0; i < readouts; ++i){
        auto *r = reinterpret_cast<TTLMonitorData *>(ptr + i * readout_size);
        REQUIRE(r->Ring == 0);
        REQUIRE(r->FEN == 100);
        REQUIRE(r->Pos == 3);
        REQUIRE((r->Channel == 1 || r->Channel == 0));
      }
      stats->packets++;
      stats->readouts += readouts;
    });
  REQUIRE(monitor_receiver.isRunning());

  {
    char addr[] = "127.0.0.1";
    auto monitor_efu = readout_create(addr, monitor_port, 8889, 1 / 14., static_cast<int>(monitor_type));
    TTLMonitor_readout_t ttl_data;
    for (uint16_t i = 0; i < max; ++i) {
      uint8_t tube = 3;
      double tof = static_cast<double>(i) / static_cast<double>(max);
      ttl_data.pos = tube;
      ttl_data.channel = 0;
      ttl_data.adc = i;
      // Setting the weight to 0, otherwise it is used to send a random number of packets
      readout_add(monitor_efu, 0, 100, tof, 0.0, static_cast<const void *>(&ttl_data));
      ttl_data.channel = 1;
      ttl_data.adc = max - i;
      readout_add(monitor_efu, 0, 100, tof, 0.0, static_cast<const void *>(&ttl_data));
    }
    readout_destroy(monitor_efu);
  }
  // the lambda function defined above gets called for each packet
  // and all 2 * max produced events are received
  auto expected = 2 * max;
  if (stats->readouts < expected){
    // wait a bit in case the receiver is doing something?
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  REQUIRE(stats->readouts == expected);
}