#pragma once

#include <bitset>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <queue>
#include <ranges>
#include <stack>
#include <vector>

namespace impl::huffman {

struct code {
  std::bitset<256> bits;    // corner case for max code length is 255
  uint8_t          count;

  using hash = unsigned long;
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
      if (code.bits.test(bit)) {
        write_bit<1>();
      } else {
        write_bit<0>();
      }
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

struct huffman_decode_node {
  huffman_decode_node* left;
  huffman_decode_node* right;
  byte                 byte;
};

}    // namespace impl::huffman

namespace huffman {

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> encode(Itr begin, Itr end) {
  std::array<impl::huffman::huffman_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols, this holds lifetimes of nodes

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  // make leaf nodes
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

  // make intermediate nodes (no codes yet)
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

  if (tree_begin->byte.has_value()) {    // corner case for single byte repeated
    codes_by_byte[tree_begin->byte.value()] = {.bits  = {1},
                                               .count = 1};    // 1 bit code
    writer.write_bit<1>();
    writer.write_byte(tree_begin->byte.value());
  } else {
    // codes -> left is 0, right is 1, Tree output -> 0 is not leaf, 1 + byte is leaf, push left first
    std::stack<impl::huffman::huffman_node*> stack {
      {std::to_address(tree_begin)}};    // start at root
    while (!stack.empty()) {
      auto node = stack.top();
      stack.pop();

      if (node->byte.has_value()) {
        writer.write_bit<1>();
        writer.write_byte(node->byte.value());

        codes_by_byte[node->byte.value()] = node->code;
      } else {
        writer.write_bit<0>();

        node->left->code        = node->code;
        node->left->code.bits <<= 1;
        node->left->code.bits.set(0, false);
        node->left->code.count += 1;
        stack.push(node->left);

        node->right->code        = node->code;
        node->right->code.bits <<= 1;
        node->right->code.bits.set(0, true);
        node->right->code.count += 1;
        stack.push(node->right);
      }
    }
  }

  // Msg output
  for (auto itr = begin; itr != end; ++itr) {
    writer.write_code(codes_by_byte[*itr]);
  }

  // Pad bits output
  const uint8_t pad_bits = 8 - writer.count_bits_in_buffer();
  compressed[0]          = pad_bits;
  writer.flush();

  return compressed;
}

template<typename Itr>
requires std::is_same_v<typename Itr::value_type, uint8_t> &&
         std::random_access_iterator<Itr>
std::vector<uint8_t> decode(Itr begin, Itr end) {
  std::array<impl::huffman::huffman_decode_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols, this holds lifetimes of nodes

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  impl::huffman::bit_reader reader {begin + 1};
  const size_t bits_to_read = (std::distance(begin + 1, end) * 8) - *begin;
  size_t       bits_read    = 0;

  if (reader.read_bit() == 1) {    // corner case for single byte repeated
    return std::vector(bits_to_read - 9, reader.read_byte());
  }
  bits_read += 1;

  *tree_end++ = {
    .left  = nullptr,
    .right = nullptr,
    .byte  = std::nullopt,
  };

  std::stack<impl::huffman::huffman_decode_node*> stack {
    {std::to_address(tree_begin)}};
  while (!stack.empty()) {
    auto* node = stack.top();
    stack.pop();

    if (reader.read_bit() == 1) {
      node->byte  = reader.read_byte();
      bits_read  += 9;
    } else {
      bits_read += 1;

      *tree_end++ = impl::huffman::huffman_decode_node {
        .left  = nullptr,
        .right = nullptr,
        .byte  = std::nullopt,
      };
      stack.push(std::to_address(tree_end - 1));
      node->left = std::to_address(tree_end - 1);

      *tree_end++ = impl::huffman::huffman_decode_node {
        .left  = nullptr,
        .right = nullptr,
        .byte  = std::nullopt,
      };
      stack.push(std::to_address(tree_end - 1));
      node->right = std::to_address(tree_end - 1);
    }
  }

  std::vector<uint8_t> decompressed;

  while (bits_read + 1 < bits_to_read) {
    auto* node = std::to_address(tree_begin);

    while (!node->byte.has_value()) {
      if (reader.read_bit() == 0) {
        node = node->left;
      } else {
        node = node->right;
      }
      bits_read += 1;
    }

    decompressed.emplace_back(node->byte.value());
  }

  return decompressed;
}

}    // namespace huffman
