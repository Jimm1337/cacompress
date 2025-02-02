#pragma once

#include "UTIL.hpp"

#include <array>
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

template<typename Itr>
constexpr void write_code(util::bit_writer<Itr>& writer, code code) {
  for (int bit = code.count - 1; bit >= 0; --bit) {
    if (code.bits.test(bit)) {
      writer.template write_bit<1>();
    } else {
      writer.template write_bit<0>();
    }
  }
}

void traverse_node(huffman_node* node, auto& table, code code) {
  if (node->byte.has_value()) {
    table[node->byte.value()] = code;
  } else {
    traverse_node(
    node->left,
    table,
    huffman::code {.bits  = code.bits << 1,
                   .count = static_cast<uint8_t>(code.count + 1)});

    traverse_node(
    node->right,
    table,
    huffman::code {.bits = code.bits << 1 |= 1,
                   .count = static_cast<uint8_t>(code.count + 1)});
  }
}

void encode_tree(huffman_node* node, auto& writer) {
  if (node->byte.has_value()) {
    writer.template write_bit<1>();
    writer.write_byte(node->byte.value());
  } else {
    writer.template write_bit<0>();
    encode_tree(node->left, writer);
    encode_tree(node->right, writer);
  }
}

huffman_decode_node* decode_tree(auto& itr, auto& reader) {
  if (reader.read_bit() == 1) {
    *itr++ = huffman_decode_node {.left  = nullptr,
                                  .right = nullptr,
                                  .byte  = reader.read_byte()};

    return std::to_address(itr - 1);
  }

  huffman_decode_node* left  = decode_tree(itr, reader);
  huffman_decode_node* right = decode_tree(itr, reader);

  *itr++ =
  huffman_decode_node {.left = left, .right = right, .byte = std::nullopt};

  return std::to_address(itr - 1);
}

}    // namespace impl::huffman

namespace huffman {

template<typename ItrIn, typename ItrOut>
requires std::forward_iterator<ItrIn> &&
         std::output_iterator<ItrOut, uint8_t>
void encode(ItrIn begin, ItrIn end, ItrOut out) {
  std::array<impl::huffman::huffman_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols, this holds lifetimes of nodes

  const auto tree_begin = tree.begin();
  auto       tree_end   = tree.begin();

  size_t tree_bit_size = 0;

  // make leaf nodes
  std::array<std::optional<uint16_t>, 256> cache_idx {};
  for (auto itr = begin; itr != end; ++itr) {
    const uint8_t byte = *itr;

    if (!cache_idx[byte].has_value()) {
      tree_bit_size += 10;

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

  --tree_bit_size;
  std::array<size_t, 256> freq_by_byte {};
  for (auto itr = tree_begin; itr != tree_end; ++itr) {
    freq_by_byte[itr->byte.value()] = itr->frequency;
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
  // (byte) pad bits count -> (bits) tree -> (bits) msg -> (bits) pad to byte

  std::array<impl::huffman::code, 256> codes_by_byte {};

  if (tree_begin->byte.has_value()) {    // corner case for single byte repeated
    codes_by_byte[tree_begin->byte.value()] = {.bits  = {1},
                                               .count = 1};    // 1 bit code
  } else {
    // codes -> left is 0, right is 1
    impl::huffman::traverse_node(std::to_address(tree_begin),
                                 codes_by_byte,
                                 impl::huffman::code {.bits = {0}, .count = 0});
  }

  util::bit_writer writer {out};

  // Pad bits count output
  // 1 + tree_size + msg_size -> to be written, first bit is 1 for no pad and 0 for pad
  size_t msg_bit_size = 0;
  for (size_t i = 0; i < 256; ++i) {
    msg_bit_size += codes_by_byte[i].count * freq_by_byte[i];
  }

  const uint8_t pad_bits = 8 - ((8 + tree_bit_size + msg_bit_size) % 8);

  writer.write_byte(pad_bits);

  // Tree output
  impl::huffman::encode_tree(std::to_address(tree_begin), writer);

  // Msg output
  for (auto itr = begin; itr != end; ++itr) {
    impl::huffman::write_code(writer, codes_by_byte[*itr]);
  }

  // Pad bits output
  if (pad_bits != 0) {
    writer.flush();
  }
}

template<typename ItrIn, typename ItrOut>
requires std::random_access_iterator<ItrIn> &&
         std::output_iterator<ItrOut, uint8_t>
void decode(ItrIn begin, ItrIn end, ItrOut out) {
  std::array<impl::huffman::huffman_decode_node, 256 * 2 - 1>
  tree;    // size is num of huffman nodes for max symbols, this holds lifetimes of nodes

  const size_t bits_to_read =
  (std::distance(std::next(begin), end) * 8) - *begin;
  size_t bits_read = 0;

  util::bit_reader reader {std::next(begin)};

  auto                                tree_space_itr = tree.begin();
  impl::huffman::huffman_decode_node* root =
  impl::huffman::decode_tree(tree_space_itr, reader);

  // Tree bit size calculation
  for (const auto& node : tree) {
    if (node.byte.has_value()) {
      bits_read += 10;
    }
  }
  --bits_read;

  // Corner case for single byte repeated
  if (root->byte.has_value()) {
    while (bits_read++ < bits_to_read) {
      *out++ = root->byte.value();
    }
  }

  impl::huffman::huffman_decode_node* node = root;
  while (bits_read < bits_to_read) {
    while (!node->byte.has_value()) {
      if (reader.read_bit() == 1) {
        node = node->right;
      } else {
        node = node->left;
      }
      ++bits_read;
    }

    *out++ = node->byte.value();
    node   = root;
  }
}

}    // namespace huffman
