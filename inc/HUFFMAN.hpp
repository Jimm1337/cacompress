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

  constexpr void write_byte(uint8_t byte) {
    for (int bit = 7; bit >= 0; --bit) {
      if (((byte & (1 << bit)) >> 7) == 1) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
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

    return ((*input_iterator >> (7 - count++)) & 1);
  }

  constexpr uint8_t read_byte() {
    uint8_t result = 0;

    for (int bit = 7; bit >= 0; --bit) {
      result |= read_bit() << bit;
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
  // TREE

  std::array<impl::huffman::huffman_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  std::array<std::optional<uint16_t>, 256> cache_idx {};

  for (auto itr = begin; itr != end; ++itr) {
    const uint8_t byte = *itr;

    if (!cache_idx[byte].has_value()) {
      cache_idx[byte] =
      static_cast<int16_t>(std::distance(tree.begin(), tree_end));
      *tree_end++ = {.frequency = 1,
                     .left      = nullptr,
                     .right     = nullptr,
                     .byte      = byte,
                     .code      = 0};
    } else {
      ++tree[*cache_idx[byte]].frequency;
    }
  }

  std::make_heap(tree.begin(), tree_end, std::greater {});

  auto park_itr = tree.rbegin();
  while (std::distance(tree_begin, tree_end) > 1) {
    std::pop_heap(tree_begin, tree_end, std::greater {});
    std::pop_heap(tree_begin, tree_end - 1, std::greater {});
    *park_itr++ = *(--tree_end);
    *park_itr++ = *(--tree_end);
    *tree_end++ = impl::huffman::huffman_node {
      .frequency = (park_itr - 2)->frequency + (park_itr - 1)->frequency,
      .left      = std::to_address(park_itr - 2),
      .right     = std::to_address(park_itr - 1),
      .byte      = std::nullopt,
      .code      = 0};
    std::push_heap(tree_begin, tree_end, std::greater {});
  }

  // uint8_t pad_bits -> (bits) tree -> (bits) msg
  std::vector<uint8_t> compressed;
  compressed.reserve(std::distance(begin, end));
  compressed.emplace_back(static_cast<uint8_t>(0));    // pad bits

  impl::huffman::bit_writer writer {std::back_inserter(compressed)};

  std::array<impl::huffman::code, 256> codes_by_byte {};

  // left is 0, right is 1
  std::stack<impl::huffman::huffman_node*> dfs_stack {
    {std::to_address(tree_begin)}};
  while (!dfs_stack.empty()) {
    auto* node = dfs_stack.top();
    dfs_stack.pop();

    if (!node->byte.has_value()) {
      node->left->code  = node->code << 1;
      node->right->code = (node->code << 1) | 1;
      dfs_stack.push(node->left);
      dfs_stack.push(node->right);
      writer.write_bit<0>();    // 0 means not a leaf
    } else {
      codes_by_byte[node->byte.value()] = node->code;
      writer.write_bit<1>();    // 1 and byte means leaf
      writer.write_byte(node->byte.value());
    }
  }

  // MSG

  for (auto itr = begin; itr != end; ++itr) {
    const auto code = codes_by_byte[*itr].value();
    writer.write_bits(code);
  }

  const uint8_t pad_bits = 8 - writer.count_bits_in_buffer();
  compressed[0]          = pad_bits;
  writer.flush();

  return compressed;
}

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> decode(Itr begin, Itr end) {
  const size_t bits_to_read = (std::distance(begin + 1, end) * 8) - *begin;

  // TREE

  impl::huffman::bit_reader reader {begin + 1};
  size_t                    bits_read = 0;

  const auto read_node = [&reader, &bits_read](uint8_t prev_code,
                                               uint8_t right_1_left_0) {
    const uint8_t new_code = (prev_code << 1) | right_1_left_0;

    if (reader.read_bit() == 0) {
      ++bits_read;
      return impl::huffman::huffman_decode_node {.byte = std::nullopt,
                                                 .code = new_code};
    }

    bits_read += 9;
    return impl::huffman::huffman_decode_node {.byte = reader.read_byte(),
                                               .code = new_code};
  };

  std::array<impl::huffman::byte, 256>           bytes_by_code {};
  std::stack<impl::huffman::huffman_decode_node> decode_stack {
    {read_node(0, 0)}};
  while (!decode_stack.empty()) {
    const auto [byte, code] = decode_stack.top();
    decode_stack.pop();

    if (byte.has_value()) {
      bytes_by_code[code] = byte;
    } else {
      decode_stack.push(read_node(code, 0));
      decode_stack.push(read_node(code, 1));
    }
  }

  // todo: reconstruct tree instead of this greedy approach

  // MSG

  std::vector<uint8_t> decompressed_msg;
  decompressed_msg.reserve(std::distance(begin, end));

  while (bits_read <= bits_to_read) {
    uint8_t code = reader.read_bit();
    ++bits_read;

    while (bytes_by_code[code] == std::nullopt) {
      code <<= 1;
      code  |= reader.read_bit();
      ++bits_read;
    }

    decompressed_msg.emplace_back(bytes_by_code[code].value());
  }

  return decompressed_msg;
}

}    // namespace huffman
