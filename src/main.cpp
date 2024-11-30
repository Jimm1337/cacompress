#include <fmt/core.h>

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <raylib.h>

// UTILS v

template<size_t size>
constexpr std::array<uint8_t, size> create_array() {
  std::array<uint8_t, size> arr {};

  std::mt19937                  gen {std::random_device {}()};
  std::uniform_int_distribution dist {0, 1};

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
    if (elem == 1) {
      DrawRectangle(width_pos, height_pos, 10, 10, BLACK);
    }

    width_pos += 10;
  }
}

template<size_t size>
void display_result(const std::vector<std::array<uint8_t, size>>& result) {
  int height_pos = 0;

  for (const auto& elem : result) {
    display_array(elem, height_pos);
    height_pos += 10;
  }
}

// UTILS ^

using SOCA_Rule = std::array<uint8_t, 16>;

constexpr SOCA_Rule create_triple_rule(uint8_t rule_number) {
  SOCA_Rule out {};

  const std::bitset<8>& rule {rule_number};

  for (uint8_t i = 0; i < 8; ++i) {
    const size_t index_0 = i;
    const size_t index_1 = i + (1U << 3U);

    out[index_0] = rule[i] ? 1 : 0;
    out[index_1] = rule[i] ? 0 : 1;
  }

  return out;
}

template<size_t size>
constexpr std::array<uint8_t, size> soca_triple_forward(
const std::array<uint8_t, size>& arr_in,
const SOCA_Rule&                 rule,
int                           itr) {
  // cut in half the input -> first half t - 1, second half t

  // do this in blocks of 1024, save information about added bytes in input to align on 1024 bits (form -> n bytes added - save n as uint8_t)

  constexpr static auto half_size = size / 2;

  std::vector<std::array<uint8_t, half_size>> arr_out;
  arr_out.resize(itr);

  arr_out[0] = std::array<uint8_t, half_size> {};
  arr_out[1] = std::array<uint8_t, half_size> {};
  std::copy(arr_in.begin(), arr_in.begin() + half_size, arr_out[0].begin());
  std::copy(arr_in.begin() + half_size, arr_in.end(), arr_out[1].begin());

  for (int j = 2; j < itr; ++j) {
    for (int i = 0; i < half_size; ++i) {
      arr_out[j][i] =
      rule[(arr_out[j - 2][i] << 3) |
           (arr_out[j - 1][(i + half_size - 1) % half_size] << 2) |
           (arr_out[j - 1][i] << 1) | (arr_out[j - 1][(i + 1) % half_size])];
    }
  }

  std::array<uint8_t, size> ret {};
  std::copy(arr_out.back().begin(),
            arr_out.back().end(),
            ret.begin() + half_size);                           // front -> first
  std::copy(arr_out[arr_out.size() - 2].begin(),    // back -> second
            arr_out[arr_out.size() - 2].end(),
            ret.begin());
  return ret;
}

template<size_t size>
constexpr std::array<uint8_t, size> soca_triple_reverse(
const std::array<uint8_t, size>& arr_in,
const SOCA_Rule&                 rule,
int                           itr) {
  constexpr static auto half_size = size / 2;

  std::vector<std::array<uint8_t, half_size>> arr_out;
  arr_out.resize(itr);

  arr_out[0] = std::array<uint8_t, half_size> {};
  arr_out[1] = std::array<uint8_t, half_size> {};
  std::copy(arr_in.begin() + half_size, arr_in.end(), arr_out[0].begin());
  std::copy(arr_in.begin(), arr_in.begin() + half_size, arr_out[1].begin());

  for (int j = 2; j < itr; ++j) {
    for (int i = 0; i < half_size; ++i) {
      arr_out[j][i] =
      rule[(arr_out[j - 2][i] << 3) |
           (arr_out[j - 1][(i + half_size - 1) % half_size] << 2) |
           (arr_out[j - 1][i] << 1) | (arr_out[j - 1][(i + 1) % half_size])];
    }
  }

  std::array<uint8_t, size> ret {};
  std::copy(arr_out[arr_out.size() - 1].begin(),
            arr_out[arr_out.size() - 1].end(),
            ret.begin());
  std::copy(arr_out[arr_out.size() - 2].begin(),
            arr_out[arr_out.size() - 2].end(),
            ret.begin() + half_size);

  return ret;
}

template<size_t size>
void soca_triple_visual(const std::array<uint8_t, size>& arr_in,
                        const SOCA_Rule&                 rule,
                        int                           itr) {
  constexpr static auto half_size = size / 2;

  std::vector<std::array<uint8_t, half_size>> arr_out;
  arr_out.resize(itr);

  arr_out[0] = std::array<uint8_t, half_size> {};
  arr_out[1] = std::array<uint8_t, half_size> {};
  std::copy(arr_in.begin(), arr_in.begin() + half_size, arr_out[0].begin());
  std::copy(arr_in.begin() + half_size, arr_in.end(), arr_out[1].begin());

  for (int j = 2; j < itr / 2; ++j) {
    for (int i = 0; i < half_size; ++i) {
      arr_out[j][i] =
      rule[(arr_out[j - 2][i] << 3) |
           (arr_out[j - 1][(i + half_size - 1) % half_size] << 2) |
           (arr_out[j - 1][i] << 1) | (arr_out[j - 1][(i + 1) % half_size])];
    }
  }

  std::array<uint8_t, size> arr_result {};


  arr_out[itr / 2]     = std::array<uint8_t, half_size> {};
  arr_out[itr / 2 + 1] = std::array<uint8_t, half_size> {};
  std::copy(arr_out[itr / 2 - 1].begin(),
            arr_out[itr / 2 - 1].end(),
            arr_out[itr / 2].begin());
  std::copy(arr_out[itr / 2 - 2].begin(),
            arr_out[itr / 2 - 2].end(),
            arr_out[itr / 2 + 1].begin());

  for (int j = itr / 2 + 2; j < itr; ++j) {
    for (int i = 0; i < half_size; ++i) {
      arr_out[j][i] =
      rule[(arr_out[j - 2][i] << 3) |
           (arr_out[j - 1][(i + half_size - 1) % half_size] << 2) |
           (arr_out[j - 1][i] << 1) | (arr_out[j - 1][(i + 1) % half_size])];
    }
  }

  std::array<uint8_t, size> arr_rev_result {};

  std::copy(arr_out[itr / 2 + 1].begin(),
            arr_out[itr / 2 + 1].end(),
            arr_result.begin());
  std::copy(arr_out[itr / 2].begin(),
            arr_out[itr / 2].end(),
            arr_result.begin() + half_size);

  std::copy(arr_out[itr - 1].begin(),
            arr_out[itr - 1].end(),
            arr_rev_result.begin());
  std::copy(arr_out[itr - 2].begin(),
            arr_out[itr - 2].end(),
            arr_rev_result.begin() + half_size);

  display_result(arr_out);
}

int main() {
  uint8_t rule = 0;

  constexpr static size_t itr = 100;

  InitWindow(1000, 1000, fmt::format("SOCA Triple: {}", rule).c_str());

  SetTargetFPS(120);

  const auto arr = create_array<200>();

  for (uint8_t i = 0; i < 255; ++i) {
    auto out = soca_triple_forward(arr, create_triple_rule(i), itr);
    auto rev = soca_triple_reverse(out, create_triple_rule(i), itr);

    if (verify_array(arr, rev)) {
      fmt::println("{}, SUCCESS", i);
    } else {
      fmt::println("{}, FAILURE", i);
      rule = i;
      SetWindowTitle(fmt::format("SOCA Triple: {}", rule).c_str());
      break;
    }
  }

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

    soca_triple_visual(arr, create_triple_rule(rule), itr);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
