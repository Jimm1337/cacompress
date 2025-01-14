#pragma once

#include <bitset>
#include <cstdint>
#include <iterator>
#include <cmath>
#include <cstdlib>

namespace util {

// FIRST ADDED IS MSB
template<typename Itr>
class bit_writer {
  Itr     output_iterator;
  uint8_t buffer {0};
  uint8_t count {0};

public:
  constexpr explicit bit_writer(Itr output_iterator):
    output_iterator(output_iterator) {
  }

  template<uint8_t val>
  requires(val == 0 || val == 1)
  constexpr void write_bit() {
    buffer <<= 1;
    buffer  |= val;
    ++count;

    if (count == 8) {
      flush();
    }
  }

  constexpr void write_byte(uint8_t byte) {
    const std::bitset<8> bits {byte};
    for (int bit = 7; bit >= 0; --bit) {
      if (bits.test(bit)) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
    }
  }

  constexpr void flush() {
    buffer             <<= 8 - count;
    *output_iterator++   = buffer;
    buffer               = 0;
    count                = 0;
  }
};

// MSB IS STILL MSB, bits flushed right
template<typename Itr>
class bit_reader {
  Itr     input_iterator_;
  uint8_t buffer_ {0};
  uint8_t count_ {8};

public:
  constexpr explicit bit_reader(Itr input_iterator):
    input_iterator_(input_iterator) {
  }

  constexpr uint8_t read_bit() {
    if (count_ == 8) {
      buffer_ = *input_iterator_;
      count_  = 0;
    }

    const uint8_t bit = ((buffer_ >> (7 - count_++)) & 1u);

    if (count_ == 8) {
      ++input_iterator_;
    }

    return bit;
  }

  constexpr uint8_t read_byte() {
    uint8_t result = 0;

    for (int bit = 7; bit >= 0; --bit) {
      result |= read_bit() << bit;
    }

    return result;
  }
};

template <typename Itr>
constexpr double calculate_entropy(Itr begin, Itr end) {
  const size_t size = std::distance(begin, end);

  double result = 0.0;
  for (auto itr = begin; itr != end; ++itr) {
    result += -std::log2(static_cast<double>(*itr) / size);
  }
  return result;
}

}    // namespace util