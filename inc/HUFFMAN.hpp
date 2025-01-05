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

struct code {
  std::bitset<8> bits;
  uint8_t        count;

  using hash = unsigned long;

  [[nodiscard]] hash hashed() const {
    return bits.to_ulong() | (static_cast<hash>(count) << 8);
  }

  [[nodiscard]] static constexpr code from_hash(hash hash) {
    return {.bits  = {static_cast<uint8_t>(hash & 0xFF)},
            .count = static_cast<uint8_t>(hash >> 8)};
  }
};

using byte = std::optional<uint8_t>;

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

  constexpr void write_code(code code) {
    for (int bit = code.count - 1; bit >= 0; --bit) {
      if (code.bits.test(bit) == 1) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
    }
  }

  constexpr void write_byte(uint8_t byte) {
    const std::bitset<8> bits {byte};
    for (int bit = 7; bit >= 0; --bit) {
      if (bits.test(bit) == 1) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
    }
  }

  constexpr void flush() {
    buffer             <<= 8 - count;
    *output_iterator++   = buffer;

    count  = 0;
    buffer = 0;
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

struct huffman_node {
  size_t        frequency;
  huffman_node* left;
  huffman_node* right;
  byte          byte;
  code          code;

  constexpr std::weak_ordering operator<=>(const huffman_node& other) const {
    return frequency <=> other.frequency;
  }
};

struct encode_stack_node {
  huffman_node* node;
  code          code;
  bool          right;
};

struct huffman_decode_node {
  byte byte;
  code code;
};

}    // namespace impl::huffman

namespace huffman {

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> encode(Itr begin, Itr end) {
  std::array<impl::huffman::huffman_node, 256 * 2>
  tree;    // size is num of huffman nodes for max symbols

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  // make tree w/ leaf nodes
  std::array<std::optional<uint16_t>, 256> cache_idx {};
  for (auto itr = begin; itr != end; ++itr) {
    const uint8_t byte = *itr;

    if (!cache_idx[byte].has_value()) {
      cache_idx[byte] =
      static_cast<int16_t>(std::distance(tree.begin(), tree_end));
      *tree_end++ = {
        .frequency = 1,
        .left      = nullptr,
        .right     = nullptr,
        .byte      = byte,
        .code      = impl::huffman::code {.bits = {0}, .count = 0}
      };
    } else {
      ++tree[*cache_idx[byte]].frequency;
    }
  }

  // make tree w/ intermediate nodes (no codes yet)
  std::make_heap(tree_begin, tree_end, std::greater {});
  auto park_itr = tree.rbegin();
  while (std::distance(tree_begin, tree_end) > 1) {
    std::pop_heap(tree_begin, tree_end, std::greater {});
    *park_itr++ = *(--tree_end);
    std::pop_heap(tree_begin, tree_end, std::greater {});
    *park_itr++ = *(--tree_end);
    *tree_end++ = impl::huffman::huffman_node {
      .frequency = (park_itr - 2)->frequency + (park_itr - 1)->frequency,
      .left      = std::to_address(park_itr - 2),
      .right     = std::to_address(park_itr - 1),
      .byte      = std::nullopt,
      .code      = impl::huffman::code {.bits = {0}, .count = 0}
    };
    std::push_heap(tree_begin, tree_end, std::greater {});
  }

  // Output format:
  // uint8_t pad_bits count -> (bits) tree -> (bits) msg -> (pad bits)

  std::vector<uint8_t> compressed;
  compressed.reserve(std::distance(begin, end));
  compressed.emplace_back(static_cast<uint8_t>(0));    // pad bits

  impl::huffman::bit_writer writer {std::back_inserter(compressed)};

  std::array<impl::huffman::code, 256> codes_by_byte {};

  // codes -> left is 0, right is 1, Tree output -> 0 is not leaf, 1 + byte is leaf
  std::stack<impl::huffman::encode_stack_node> dfs_stack {{{
    .node  = std::to_address(tree_begin),
    .code  = {.bits = {0}, .count = 0},
    .right = false,
  }}};    // start at root
  while (!dfs_stack.empty()) {
    auto node = dfs_stack.top();
    dfs_stack.pop();
    node.node->code.bits  = node.code.bits << 1;
    node.node->code.count = node.code.count + 1;
    node.node->code.bits.set(0, node.right);

    if (!node.node->byte.has_value()) {
      dfs_stack.push(
      {.node = node.node->right, .code = node.node->code, .right = true});
      dfs_stack.push(
      {.node = node.node->left, .code = node.node->code, .right = false});
      writer.write_bit<0>();    // 0 means not a leaf
    } else {
      codes_by_byte[node.node->byte.value()] = node.node->code;
      writer.write_bit<1>();    // 1 and byte means leaf
      writer.write_byte(node.node->byte.value());
    }
  }

  // Msg output
  for (auto itr = begin; itr != end; ++itr) {
    writer.write_code(codes_by_byte[*itr]);
  }

  // Pad bits output
  const uint8_t pad_bits = 8 - writer.count_bits_in_buffer();

  if (pad_bits != 8) {
    compressed[0] = pad_bits;
    writer.flush();
  }

  return compressed;
}

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> decode(Itr begin, Itr end) {
  const size_t bits_to_read = (std::distance(begin + 1, end) * 8) - *begin;
  impl::huffman::bit_reader reader {begin + 1};
  size_t                    bits_read = 0;

  // reading routine for tree
  const auto read_node = [&reader, &bits_read](impl::huffman::code prev_code,
                                               bool                right) {
    const impl::huffman::code new_code = [&prev_code, right] {
      auto new_code_ = prev_code.bits << 1;
      return impl::huffman::code {
        .bits  = right ? new_code_.set(0, true) : new_code_,
        .count = static_cast<uint8_t>(prev_code.count + 1)};
    }();

    if (reader.read_bit() == 0) {
      ++bits_read;
      return impl::huffman::huffman_decode_node {.byte = std::nullopt,
                                                 .code = new_code};
    }

    bits_read += 9;
    return impl::huffman::huffman_decode_node {.byte = reader.read_byte(),
                                               .code = new_code};
  };

  // get bytes by code (have to hash whole or else there could be conflicts with leading 0s)
  std::array<impl::huffman::byte, 4096>
  bytes_by_hashed_code {};    // size is max of hashed code (8 + 4) bits
  std::stack<impl::huffman::huffman_decode_node> decode_stack {
    {read_node({.bits = {0}, .count = 1}, false)}};
  while (!decode_stack.empty()) {
    const auto [byte, code] = decode_stack.top();
    decode_stack.pop();

    if (byte.has_value()) {
      bytes_by_hashed_code[code.hashed()] = byte;
    } else {
      decode_stack.push(read_node(code, false));
      decode_stack.push(read_node(code, true));
    }
  }

  // read msg
  std::vector<uint8_t> decompressed_msg;
  decompressed_msg.reserve(std::distance(begin, end));

  while (bits_read <= bits_to_read) {
    impl::huffman::code code {.bits = reader.read_bit(), .count = 1};
    ++bits_read;

    while (bytes_by_hashed_code[code.hashed()] == std::nullopt) {
      code.bits <<= 1;
      code.bits.set(0, reader.read_bit() == 1);
      ++code.count;
      ++bits_read;
    }

    decompressed_msg.emplace_back(bytes_by_hashed_code[code.hashed()].value());
  }

  return decompressed_msg;
}

}    // namespace huffman
