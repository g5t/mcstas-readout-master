#pragma once
#include <atomic>

class UDPStats {
public:
  std::atomic<int> packets{0};
  std::atomic<int> readouts{0};
};