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
    const uint8_t bit = ((*input_iterator >> (7 - count++)) & 1);

    if (count == 8) {
      count = 0;
      ++input_iterator;
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

  constexpr Itr get_iterator() const {
    return input_iterator;
  }
};

struct huffman_node {
  size_t        frequency;
  huffman_node* left;
  huffman_node* right;
  byte          byte;

  constexpr std::weak_ordering operator<=>(const huffman_node& other) const {
    return frequency <=> other.frequency;
  }
};

struct huffman_decode_node {
  huffman_decode_node* left;
  huffman_decode_node* right;
  byte                 byte;
};

void encode_node(huffman_node* node, auto& writer, auto& table, code code) {
  if (node->byte.has_value()) {
    writer.template write_bit<1>();
    writer.write_byte(node->byte.value());
    table[node->byte.value()] = code;
  } else {
    writer.template write_bit<0>();

    encode_node(node->left,
                writer,
                table,
                huffman::code {.bits  = code.bits << 1,
                               .count = static_cast<uint8_t>(code.count + 1)});

    encode_node(node->right,
                writer,
                table,
                huffman::code {.bits = code.bits << 1 |= 1,
                               .count = static_cast<uint8_t>(code.count + 1)});
  }
}

huffman_decode_node* decode_node(auto& itr, auto& reader) {
  if (reader.read_bit() == 1) {
    *itr++ = huffman_decode_node {.left  = nullptr,
                                  .right = nullptr,
                                  .byte  = reader.read_byte()};

    return std::to_address(itr - 1);
  }

  huffman_decode_node* left   = decode_node(itr, reader);
  huffman_decode_node* right  = decode_node(itr, reader);

  *itr++ =
  huffman_decode_node {.left = left, .right = right, .byte = std::nullopt};

  return std::to_address(itr - 1);
}

}    // namespace impl::huffman

namespace huffman {

template<typename ItrIn, typename ItrOut>
requires((std::forward_iterator<ItrIn> &&
          std::is_same_v<typename ItrIn::value_type, uint8_t>) ||
         std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<ItrIn>>,
                        uint8_t>) &&
        (std::output_iterator<ItrOut, uint8_t> ||
         std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<ItrOut>>,
                        uint8_t>)
void encode(ItrIn begin, ItrIn end, ItrOut out) {
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
      *tree_end++ = {.frequency = 1,
                     .left      = nullptr,
                     .right     = nullptr,
                     .byte      = byte};
    } else {
      ++tree[*cache_idx[byte]].frequency;
    }
  }

  // make intermediate nodes
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
      .byte      = std::nullopt};
    std::push_heap(tree_begin, tree_end, std::greater {});
  }

  // Output format:
  // (bits) tree -> (bits) msg -> (pad bits)

  impl::huffman::bit_writer writer {out};

  std::array<impl::huffman::code, 256> codes_by_byte {};

  if (tree_begin->byte.has_value()) {    // corner case for single byte repeated
    codes_by_byte[tree_begin->byte.value()] = {.bits  = {1},
                                               .count = 1};    // 1 bit code
    writer.template write_bit<1>();
    writer.write_byte(tree_begin->byte.value());
  } else {
    // codes -> left is 0, right is 1
    // Tree output -> 0 is not leaf, 1 + byte is leaf, left first dfs
    impl::huffman::encode_node(std::to_address(tree_begin),
                               writer,
                               codes_by_byte,
                               {.bits = 0, .count = 0});
  }

  // Msg output
  for (auto itr = begin; itr != end; ++itr) {
    writer.write_code(codes_by_byte[*itr]);
  }

  // Pad bits output
  writer.flush();
}

template<typename ItrIn, typename ItrOut>
requires((std::input_iterator<ItrIn> &&
          std::is_same_v<typename ItrIn::value_type, uint8_t>) ||
         std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<ItrIn>>,
                        uint8_t>) &&
        (std::output_iterator<ItrOut, uint8_t> ||
         std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<ItrOut>>,
                        uint8_t>)
void decode(ItrIn begin, ItrIn end, ItrOut out) {
  std::array<impl::huffman::huffman_decode_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols, this holds lifetimes of nodes

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  impl::huffman::bit_reader reader {begin};

  auto* root = impl::huffman::decode_node(tree_end, reader);
  if (std::distance(tree_begin, tree_end) ==
      1) {    // corner case for single byte repeated
    const auto byte = root->byte.value();
    while (reader.get_iterator() != end) {
      if (reader.read_bit() == 1) {
        *out++ = byte;
      }
    }
  }

  auto* node = root;
  while (reader.get_iterator() != end) {
    while (node != nullptr && reader.get_iterator() != end &&
           !node->byte.has_value()) {
      if (reader.read_bit() == 0) {
        node = node->left;
      } else {
        node = node->right;
      }
    }

    if (node != nullptr && node->byte.has_value()) {
      *out++ = node->byte.value();
      node   = root;
    }
  }
}

}    // namespace huffman
