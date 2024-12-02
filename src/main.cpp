#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <vector>
#include <raylib.h>
#include "SOCA.hpp"
#include "CUT.hpp"

// UTILS v

template<size_t size>
constexpr std::array<uint8_t, size> create_array() {
  std::array<uint8_t, size> arr {};

  std::mt19937                  gen {std::random_device {}()};
  std::uniform_int_distribution dist {0, 255};

  for (size_t i = 0; i < size; ++i) {
    arr[i] = static_cast<uint8_t>(dist(gen));
  }

  return arr;
}

template<size_t size>
constexpr bool verify_array(const std::array<uint8_t, size>& arr1,
                            const std::array<uint8_t, size>& arr2) {
  for (size_t i = 0; i < size; ++i) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

template<size_t size>
void display_array(const std::array<uint8_t, size>& arr, int height_pos) {
  int width_pos = 0;
  for (const auto& elem : arr) {
    if (elem & 1) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 2) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 4) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 8) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 16) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 32) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 64) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;

    if (elem & 128) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }
    width_pos += 10;
  }
}

// UTILS ^
// SOCA v

template<size_t size>
void soca_2itr_visual(std::array<uint8_t, size> arr, uint8_t rule, int itr) {
  int height_pos = 0;
  for (int i = 0; i < itr / 2; ++i) {
    std::array<uint8_t, size / 2> arr_older {};
    std::array<uint8_t, size / 2> arr_newer {};
    std::copy(arr.begin(), arr.begin() + size / 2, arr_older.begin());
    std::copy(arr.begin() + size / 2, arr.end(), arr_newer.begin());
    display_array(arr_older, height_pos);
    height_pos += 10;
    display_array(arr_newer, height_pos);
    height_pos += 10;
    soca::forward(arr.begin(), arr.end(), rule);
  }

  for (int i = itr / 2; i < itr; ++i) {
    soca::reverse(arr.begin(), arr.end(), rule);
    std::array<uint8_t, size / 2> arr_older {};
    std::array<uint8_t, size / 2> arr_newer {};
    std::copy(arr.begin(), arr.begin() + size / 2, arr_older.begin());
    std::copy(arr.begin() + size / 2, arr.end(), arr_newer.begin());
    display_array(arr_newer, height_pos);
    height_pos += 10;
    display_array(arr_older, height_pos);
    height_pos += 10;
  }
}

// SOCA ^
// HUFFMAN v

template<size_t size> requires (size % 8 == 0)
constexpr double calculate_huffman_coded_size(const std::array<uint8_t, size>& arr_in) {
  std::array<uint8_t, 256> freq {};

  for (size_t i = 0; i < size / 8; i += 8) {
    uint8_t byte = 0;
    for (int j = 0; j < 8; ++j) {
      byte |= arr_in[i * 8 + j] << j;
    }
    ++freq[byte];
  }

  double result = 0.0;

  for (size_t i = 0; i < 256; ++i) {
    if (freq[i] != 0) {
      result += -std::log2(static_cast<double>(freq[i]) / 256);
    }
  }

  return result;
}

struct HuffmanResult {
  std::vector<uint8_t> data;
  std::vector<uint8_t> codes;
};

struct FrequencyPair {
  uint8_t byte;
  size_t  freq;
};

// std::vector<FrequencyPair> get_frequences() {
//
// }

// HUFFMAN ^
// MEASURE v

template<size_t size>
long long measure(std::array<uint8_t, size> arr, int itr) {
  const auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < itr; ++i) {
    soca::forward<113>(arr.begin(), arr.end());
  }
  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

  return duration;
}

int main() {
  constexpr static size_t itr = 50;

  InitWindow(1280, 1000, "SOCA Triple: 0");

  SetTargetFPS(120);

  static constexpr auto input_byte_size = 256 / 8;

  const auto arr = create_array<input_byte_size>();

  const auto duration = measure(arr, itr);

  for (uint8_t i = 0; i < 255; ++i) {
    auto out = arr;
    for (int j = 0; j < itr; ++j) {
      soca::forward(out.begin(), out.end(), i);
    }

    auto rev = out;
    for (int j = 0; j < itr; ++j) {
      soca::reverse(rev.begin(), rev.end(), i);
    }

    if (!verify_array(arr, rev)) {
      fmt::println("{}, FAILURE", i);
    } else {
      fmt::println("{}, SUCCESS", i);
    }
  }

  fmt::println("SOCA_time: {}us", duration);

  uint8_t rule = 0;

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(WHITE);

    if (IsKeyPressed(KEY_LEFT)) {
      if (rule != 0) {
        --rule;
        SetWindowTitle(fmt::format("SOCA Triple: {}", rule).c_str());
      }
    }

    if (IsKeyPressed(KEY_RIGHT)) {
      if (rule != 255) {
        ++rule;
        SetWindowTitle(fmt::format("SOCA Triple: {}", rule).c_str());
      }
    }

    soca_2itr_visual(arr, rule, itr);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
