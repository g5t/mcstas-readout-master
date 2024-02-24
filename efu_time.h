//
// Created by g on 26/10/22.
//

#ifndef MCSTAS_UDP_TRANSMIT_EFU_TIME_H
#define MCSTAS_UDP_TRANSMIT_EFU_TIME_H
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

static const uint64_t ticks = 88052499;

class efu_time {
protected:
  uint32_t _h;
  uint32_t _l;
public:
  efu_time(uint32_t h, uint32_t l): _h(h), _l(l) {};
  explicit efu_time(double t): _h(static_cast<uint32_t>(t)), _l(static_cast<uint32_t>(std::fmod(t, 1.0) * ticks)){}
  efu_time(): _h(0), _l(0) {
    uint64_t nps = 1000000000u;
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto s = ns / nps;
    auto t = (static_cast<uint64_t>(ns % nps) * ticks) / nps;
    _h = static_cast<uint32_t>(s);
    _l = static_cast<uint32_t>(t);
  }

  uint32_t high() const {return _h;}
  uint32_t low() const {return _l;}

  bool operator==(const efu_time& other) const {
    return (_h == other._h) && (_l == other._l);
  }
  bool operator<(const efu_time& other) const {
    if (_h < other._h) return true;
    if (_h > other._h) return false;
    return _l < other._l;
  }
  bool operator>(const efu_time& other) const {
    if (_h > other._h) return true;
    if (_h < other._h) return false;
    return _l > other._l;
  }

  bool operator>=(const efu_time * other) const {
    return (*this > *other || *this == *other);
  }

  efu_time operator+(const efu_time & other) const {
    uint32_t h = _h + other._h;
    auto r = static_cast<uint64_t>(_l) + static_cast<uint64_t>(other._l);
    if (r > ticks) {
      h += static_cast<uint32_t>(r / ticks);
      r = r % ticks;
    }
    return {h, static_cast<uint32_t>(r)};
  }

  efu_time operator+(const efu_time * other) const {
    return *this + *other;
  }

  efu_time operator-(const efu_time & other) const {
    if (*this < other) {
      std::cout << *this << " - " << other << std::endl;
      throw std::runtime_error("Subtracting a larger time?");
    }
    uint32_t h = _h - other._h;
    if (_l >= other._l) return {h, _l - other._l};
    auto r = ticks + static_cast<uint64_t>(_l) - static_cast<uint64_t>(other._l);
    return {h - 1u, static_cast<uint32_t>(r)};
  }

  efu_time operator-(const efu_time * other) const {
    return *this - *other;
  }

  uint64_t total_ticks() const {
    return static_cast<uint64_t>(_h) * ticks + static_cast<uint64_t>(_l);
  }

  uint32_t operator/(const efu_time & other) const {
    return static_cast<uint32_t>(total_ticks() / other.total_ticks());
  }

  efu_time operator*(const uint32_t m) const {
    auto h = _h * m;
    auto r = static_cast<uint64_t>(_l) * static_cast<uint64_t>(m);
    if (r > ticks){
      h += static_cast<uint32_t>(r / ticks);
      r = r % ticks;
    }
    return {h, static_cast<uint32_t>(r)};
  }

  efu_time operator%(const efu_time& m) const {
    return *this - m * (*this / m);
  }

  efu_time operator%(const efu_time * m) const {
    return *this % *m;
  }

  friend std::ostream& operator<<(std::ostream& os, const efu_time& dt) {
    os << "(" << dt.high() << "," << dt.low() << ")";
    return os;
  }
  friend std::ostream & operator<<(std::ostream& os, const efu_time* dt) {
    os << "(" << dt->high() << "," << dt->low() << ")";
    return os;
  }
};

#endif // MCSTAS_UDP_TRANSMIT_EFU_TIME_H
