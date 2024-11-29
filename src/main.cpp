#include <fmt/core.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <limits>
#include <optional>
#include <random>
#include <raylib.h>

using SOCA_Rule = std::array<uint8_t, 16>;

struct SOCA_PAIR {
  SOCA_Rule forward;
  SOCA_Rule reverse;
};

constexpr SOCA_Rule create_rule_rev(SOCA_Rule rule) {
  SOCA_Rule out {};

  for (size_t i = 0; i < rule.size(); ++i) {
    const uint8_t t_2   = (i & 0b1000U) >> 3U;
    const uint8_t t_1_l = (i & 0b0100U) >> 2U;
    const uint8_t t_1_c = (i & 0b0010U) >> 1U;
    const uint8_t t_1_r = i & 0b0001U;
    const uint8_t t_now = rule[i];

    const size_t out_idx =
    (t_now << 3U) | (t_1_l << 2U) | (t_1_c << 1U) | t_1_r;

    out[out_idx] = t_2;
  }

  return out;
}

// t-2 L(t-1) t-1 R(t-1) -> t
constexpr static SOCA_Rule forward_rule {
  // 0b1010001001011101
  1,    // 0000
  0,    // 0001
  1,    // 0010
  1,    // 0011
  1,    // 0100
  0,    // 0101
  1,    // 0110
  0,    // 0111
  0,    // 1000
  1,    // 1001
  0,    // 1010
  0,    // 1011
  0,    // 1100
  1,    // 1101
  0,    // 1110
  1,    // 1111
};

// t L(t-1) t-1 R(t-1) -> t-2
constexpr static SOCA_Rule reverse_rule = create_rule_rev(forward_rule);

constexpr SOCA_PAIR create_pair(uint16_t rule_number) {
  SOCA_Rule forward = {};

  for (auto i = 0U; i < 16U; ++i) {
    forward[i] = (rule_number & (1U << i)) >> i;
  }

  return SOCA_PAIR {.forward = forward, .reverse = create_rule_rev(forward)};
}

constexpr static SOCA_PAIR rules_12345   = create_pair(1'2345);    // bad
constexpr static SOCA_PAIR rules_6543    = create_pair(6543);      // bad
constexpr static SOCA_PAIR rules_default = {forward_rule,
                                            reverse_rule};    // good ? why ?
constexpr static SOCA_PAIR rules_test1 =
create_pair(0b10101010'01010101);    // also good ? why ?
constexpr static SOCA_PAIR rules_test2 =
create_pair(0b10111010'01000101);    // good
constexpr static SOCA_PAIR rules_test = create_pair(0b10111010'01000101);

//todo: consteval create array size int16_max of optional<SOCA_PAIR> and try create all possible good rules

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
constexpr std::array<uint8_t, size> do_soca(
const std::array<uint8_t, size>& arr_time_1,
const std::array<uint8_t, size>& arr_time_2,
const SOCA_Rule&                 rule) {
  std::array<uint8_t, size> arr_out;

  const int first_left_1   = arr_time_1[size - 1];
  const int first_center_1 = arr_time_1[0];
  const int first_center_2 = arr_time_2[0];
  const int first_right_1  = arr_time_1[1];

  const int index_first = (first_center_2 << 3) | (first_left_1 << 2) |
                          (first_center_1 << 1) | first_right_1;

  arr_out[0] = rule[index_first];

  const int last_left_1   = arr_time_1[size - 2];
  const int last_center_1 = arr_time_1[size - 1];
  const int last_center_2 = arr_time_2[size - 1];
  const int last_right_1  = arr_time_1[0];

  const int index_last = (last_center_2 << 3) | (last_left_1 << 2) |
                         (last_center_1 << 1) | last_right_1;

  arr_out[size - 1] = rule[index_last];

  for (int i = 1; i < size - 1; ++i) {
    const int left_1   = arr_time_1[i - 1];
    const int center_1 = arr_time_1[i];
    const int center_2 = arr_time_2[i];
    const int right_1  = arr_time_1[i + 1];

    const int index =
    (center_2 << 3) | (left_1 << 2) | (center_1 << 1) | right_1;

    arr_out[i] = rule[index];
  }

  return arr_out;
}

template<size_t size, size_t itr>
constexpr std::array<std::array<uint8_t, size>, itr> soca_generate(
const std::array<uint8_t, size>& arr_now,
const std::array<uint8_t, size>& arr_time_1,
const SOCA_Rule&                 rule) {
  std::array<std::array<uint8_t, size>, itr> arr_out;

  arr_out[0] = arr_now;
  arr_out[1] = arr_time_1;

  for (size_t i = 2; i < itr; ++i) {
    arr_out[i] = do_soca(arr_out[i - 1], arr_out[i - 2], rule);
  }

  return arr_out;
}

template<size_t size, size_t itr>
constexpr std::array<std::array<uint8_t, size>, itr> soca_forward(
const std::array<uint8_t, size>& arr,
const SOCA_PAIR&                 rules) {
  return soca_generate<size, itr>(arr, arr, rules.forward);
}

template<size_t size, size_t itr>
constexpr std::array<std::array<uint8_t, size>, itr> soca_reverse(
const std::array<uint8_t, size>& arr_now,
const std::array<uint8_t, size>& arr_prev,
const SOCA_PAIR&                 rules) {
  return soca_generate<size, itr>(arr_now, arr_prev, rules.reverse);
}

void create_all_rules(std::string_view filename) {
  std::vector<std::optional<SOCA_PAIR>> rules;

  std::array<uint8_t, 20> testing_array = {0, 1, 1, 0, 1, 0, 0, 0, 1, 1,
                                           1, 1, 0, 0, 1, 0, 1, 0, 1, 1};

  for (uint16_t i = 0; i < std::numeric_limits<uint16_t>::max(); ++i) {
    auto rule     = create_pair(i);
    auto soca_out = soca_forward<20, 20>(testing_array, rule);
    auto soca_rev = soca_reverse<20, 20>(soca_out[19], soca_out[18], rule);

    if (verify_array(soca_out.front(), soca_rev.back())) {
      rules.emplace_back(std::move(rule));
    } else {
      rules.emplace_back(std::nullopt);
    }
  }

  std::ofstream file(filename.data());
  for (uint16_t i = 0; i < rules.size(); ++i) {
    if (rules.at(i).has_value()) {
      file << i << '\n';
    }
  }
}

template<size_t size>
void display_array(const std::array<uint8_t, size>& arr,
                   int                              height_pos,
                   int                              square_side_length) {
  int width_pos = 0;
  for (const auto& elem : arr) {
    if (elem == 1) {
      DrawRectangle(width_pos,
                    height_pos,
                    square_side_length,
                    square_side_length,
                    BLACK);
    }
    width_pos += square_side_length;
  }
}

std::vector<uint16_t> load_valid_rules(std::string_view filename) {
  std::vector<uint16_t> valid_rule_numbers;

  std::ifstream file(filename.data());

  while (!file.eof()) {
    uint16_t rule_number;
    file >> rule_number;
    valid_rule_numbers.emplace_back(rule_number);
  }

  return valid_rule_numbers;
}

template<size_t size, size_t itr>
requires(itr % 2 == 0) && (itr > 4)
std::array<std::array<uint8_t, size>, itr> rule_result_array(
const std::array<uint8_t, size>& input,
uint16_t                         rule) {
  const auto rules = create_pair(rule);

  const std::array<std::array<uint8_t, size>, itr / 2> forward =
  soca_forward<size, itr / 2>(input, rules);
  const std::array<std::array<uint8_t, size>, itr / 2> reverse =
  soca_reverse<size, itr / 2>(forward[itr / 2 - 1],
                              forward[itr / 2 - 2],
                              rules);

  std::array<std::array<uint8_t, size>, itr> out {};
  for (size_t i = 0; i < itr / 2; ++i) {
    out[i] = forward[i];
  }
  for (size_t i = 0; i < itr / 2; ++i) {
    out[i + itr / 2] = reverse[i];
  }

  return out;
}

template<size_t size, size_t itr>
void display_result(const std::array<std::array<uint8_t, size>, itr>& result) {
  int height_pos = 0;

  for (const auto& elem : result) {
    display_array<size>(elem, height_pos, 10);
    height_pos += 10;
  }
}

int main() {
  create_all_rules("../../data/rules2.txt");
  const auto valid_rules = load_valid_rules("../../data/rules2.txt");
  auto rule = valid_rules.begin();

  InitWindow(1000, 1000, fmt::format("SOCA: {}", valid_rules[0]).c_str());

  SetTargetFPS(120);

  const auto arr = create_array<100>();
  auto       out = rule_result_array<100, 100>(arr, *rule);

  if (verify_array(out.front(), out.back())) {
    fmt::println("{}, SUCCESS", *rule);
  } else {
    fmt::println("{}, FAILURE", *rule);
  }

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(WHITE);

    if (IsKeyPressed(KEY_LEFT)) {
      if (rule != valid_rules.begin()) {
        --rule;
        out = rule_result_array<100, 100>(arr, *rule);
      }

      SetWindowTitle(fmt::format("SOCA: {}", *rule).c_str());

      if (verify_array(out.front(), out.back())) {
        fmt::println("{}, SUCCESS", *rule);
      } else {
        fmt::println("{}, FAILURE", *rule);
      }
    }

    if (IsKeyPressed(KEY_RIGHT)) {
      if (rule != valid_rules.end() - 1) {
        ++rule;
        out = rule_result_array<100, 100>(arr, *rule);
      }

      SetWindowTitle(fmt::format("SOCA: {}", *rule).c_str());

      if (verify_array(out.front(), out.back())) {
        fmt::println("{}, SUCCESS", *rule);
      } else {
        fmt::println("{}, FAILURE", *rule);
      }
    }

    display_result(out);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
