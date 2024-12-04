#pragma once

#include <initializer_list>
#include <type_traits>

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <ranges>
#include <stack>
#include <tuple>
#include <vector>

namespace impl::huffman {

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
    if (count == 8) {
      flush();
    }

    buffer <<= 1;
    buffer  |= val;
    ++count;
  }

  constexpr void write_bits(uint8_t bits) {
    int bit = 7;
    while (bit > 0) {
      if (((bits & 0b10000000) >> 7) == 1) {
        break;
      }
      bits <<= 1;
      --bit;
    }

    while (bit >= 0) {
      if (((bits & 0b10000000) >> 7) == 1) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
      bits <<= 1;
      --bit;
    }
  }

  constexpr void flush() {
    buffer           <<= 8 - count;
    *output_iterator   = buffer;

    count  = 0;
    buffer = 0;

    ++output_iterator;
  }

  [[nodiscard]] constexpr uint8_t count_bits_in_buffer() const {
    return count;
  }
};

// MSB IS STILL MSB, bits flushed right
template<typename Itr>
class bit_reader {
  Itr     input_iterator;
  uint8_t count {0};

public:
  constexpr explicit bit_reader(Itr input_iterator):
    input_iterator(input_iterator) {
  }

  constexpr uint8_t read_bit() {
    if (count == 8) {
      count = 0;
      ++input_iterator;
    }

    return *input_iterator >> (7 - count++);
  }

  constexpr uint8_t read_byte() {
    uint8_t result = 0;
    uint8_t read   = 0;

    while (count != 8) {
      result <<= 1;
      result  |= ((*input_iterator >> (7 - count++)) & 1);
      ++read;
    }

    count = 0;
    ++input_iterator;

    while (read != 8) {
      result <<= 1;
      result  |= ((*input_iterator >> (7 - count++)) & 1);
      ++read;
    }

    return result;
  }
};

using code = std::optional<uint8_t>;
using byte = std::optional<uint8_t>;

struct huffman_node {
  size_t        frequency;
  huffman_node* left;
  huffman_node* right;
  byte          byte;
  uint8_t       code;

  constexpr std::weak_ordering operator<=>(const huffman_node& other) const {
    return frequency <=> other.frequency;
  }
};

struct huffman_decode_node {
  byte    byte;
  uint8_t code;
};

}    // namespace impl::huffman

namespace huffman {

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> encode(Itr begin, Itr end) {
  auto [tree, tree_end] = [begin, end] {
    std::array<impl::huffman::huffman_node, 256 * 2 - 1>
    tree_;    // size is num of huffman nodes for max symbols

    auto tree_end_ = tree_.begin();

    std::array cache_idx = [] {
      std::array<int16_t, 256> cache_result;
      std::ranges::fill(cache_result, -1);
      return cache_result;
    }();

    for (auto itr = begin; itr != end; ++itr) {
      const uint8_t byte = *itr;

      if (cache_idx[byte] == -1) {
        cache_idx[byte] = std::distance(tree_.begin(), tree_end_);
        *tree_end_      = {1, nullptr, nullptr, byte, 0};
        ++tree_end_;
      } else {
        ++tree_[cache_idx[byte]].frequency;
      }
    }

    std::make_heap(tree_.begin(), tree_end_, std::greater {});

    return std::pair {tree_, tree_end_};
  }();

  const auto    tree_begin   = tree.begin();
  const uint8_t symbol_count = static_cast<uint8_t>(
  std::distance(tree_begin, tree_end));    // 0 = 256 symbols
  const size_t huffman_node_count = (symbol_count * 2) - 1;

  while (tree_end != tree_begin + huffman_node_count) {
    const auto first = tree.front();
    std::pop_heap(tree_begin, tree_end, std::greater {});
    const auto second = tree.front();
    std::push_heap(tree_begin, tree_end, std::greater {});

    *tree_end = {first.frequency + second.frequency,
                 nullptr,
                 nullptr,
                 std::nullopt,
                 0};
    ++tree_end;
    std::push_heap(tree_begin, tree_end, std::greater {});
  }

  // min heap of all nodes in huffman tree at this point

  std::pop_heap(tree_begin, tree_end, std::greater {});
  auto left = tree_end - 1;

  for (auto itr = tree_end - 1; itr != tree_begin; --itr) {
    std::pop_heap(tree_begin, itr, std::greater {});
    auto right = itr - 1;

    tree.front().left  = std::to_address(left);
    tree.front().right = std::to_address(right);

    left = right;
  }

  tree.front().left  = std::to_address(left);
  tree.front().right = std::to_address(left + 1);

  // root at tree_begin at this point

  // uint8_t pad_bits -> (bits) tree -> (bits) msg
  std::vector<uint8_t> compressed_msg;
  compressed_msg.reserve(std::distance(begin, end));
  compressed_msg.emplace_back(0);    // pad bits

  impl::huffman::bit_writer writer {std::back_inserter(compressed_msg)};

  std::array<impl::huffman::code, 256> codes {};

  // left is 0, right is 1
  std::stack<impl::huffman::huffman_node*> dfs_stack {
    {std::to_address(tree_begin)}};
  while (!dfs_stack.empty()) {
    auto* node = dfs_stack.top();
    dfs_stack.pop();

    if (!node->byte.has_value()) {
      node->left->code  <<= 1;
      node->right->code <<= 1;
      node->right->code  |= 1;
      dfs_stack.push(node->left);
      dfs_stack.push(node->right);
      writer.write_bit<0>();    // 0 means not a leaf
    } else {
      codes[node->byte.value()] = node->code;
      writer.write_bit<1>();    // 1 and byte means leaf
      writer.write_bits(node->byte.value());
    }
  }

  for (auto itr = begin; itr != end; ++itr) {
    const auto code = codes[*itr].value();
    writer.write_bits(code);
  }

  const uint8_t pad_bits = 8 - writer.count_bits_in_buffer();
  compressed_msg[0]      = pad_bits;
  writer.flush();

  return compressed_msg;
}

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> decode(Itr begin, Itr end) {
  const size_t bits_to_read = (std::distance(begin + 1, end) * 8) - *begin;

  auto [bytes_by_code, reader, bits_read] = [begin] {
    impl::huffman::bit_reader reader_ {begin + 1};
    size_t                    bits_read_ = 0;

    std::array<std::optional<uint8_t>, 256> bytes_by_code_ {};

    const auto read = [&reader_, &bits_read_](uint8_t prev_code,
                                              uint8_t right_1_left_0) {
      const uint8_t new_code = (prev_code << 1) | right_1_left_0;

      if (reader_.read_bit() == 0) {
        ++bits_read_;
        return impl::huffman::huffman_decode_node {.byte = std::nullopt,
                                                   .code = new_code};
      }

      bits_read_ += 9;
      return impl::huffman::huffman_decode_node {.byte = reader_.read_byte(),
                                                 .code = new_code};
    };

    std::stack decode_stack {{read(0, 0)}};
    while (!decode_stack.empty()) {
      const auto [byte, code] = decode_stack.top();
      decode_stack.pop();

      if (byte.has_value()) {
        bytes_by_code_[code] = byte.value();
      } else {
        decode_stack.push(read(code, 0));
        decode_stack.push(read(code, 1));
      }
    }

    return std::tuple {bytes_by_code_, reader_, bits_read_};
  }();

  std::vector<uint8_t> decompressed_msg;
  decompressed_msg.reserve(std::distance(begin, end));

  while (bits_read < bits_to_read) {
    uint8_t code = reader.read_bit();
    ++bits_read;

    while (bytes_by_code[code] == std::nullopt) {
      code <<= 1;
      code |= reader.read_bit();
      ++bits_read;
    }

    decompressed_msg.emplace_back(bytes_by_code[code].value());
  }

  return decompressed_msg;
}

}    // namespace huffman
