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
  constexpr void insert() {
    if (count == 8) {
      flush();
    }

    buffer <<= 1;
    buffer  |= val;
    ++count;
  }

  constexpr void insert(uint8_t val) {
    int i = 7;
    while (i > 0) {
      if (((val & 0b10000000) >> 7) == 1) {
        break;
      }
      val <<= 1;
      --i;
    }

    while (i >= 0) {
      if (((val & 0b10000000) >> 7) == 1) {
        insert<1>();
      } else {
        insert<0>();
      }
      val <<= 1;
      --i;
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

  template<uint8_t bits_to_read>
  requires(bits_to_read <= 8)
  constexpr uint8_t read() {
    uint8_t result = 0;
    uint8_t read   = 0;

    while (count != 8 && read != bits_to_read) {
      result <<= 1;
      result  |= ((*input_iterator >> (7 - count)) & 1);
      ++count;
      ++read;
    }

    if (count == 8) {
      count = 0;
      ++input_iterator;
    }

    while (read != bits_to_read) {
      result <<= 1;
      result  |= ((*input_iterator >> (7 - count)) & 1);
      ++count;
      ++read;
    }

    return result;
  }
};

using code = std::optional<uint8_t>;

struct huffman_node {
  size_t                 frequency;
  huffman_node*          left;
  huffman_node*          right;
  std::optional<uint8_t> byte;
  uint8_t                code;

  constexpr std::weak_ordering operator<=>(const huffman_node& other) const {
    return frequency <=> other.frequency;
  }
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
      std::array<int64_t, 256> cache_result;
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

    return std::pair{tree_, tree_end_};
  }();

  const auto    tree_begin         = tree.begin();
  const uint8_t symbol_count       = static_cast<uint8_t>(std::distance(tree_begin, tree_end)); // 0 = 256 symbols
  const size_t  huffman_node_count = (symbol_count * 2) - 1;

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

  // root at tree_begin

  std::array<impl::huffman::code, 256> codes;

  // uint8_t pad_bits -> uint8_t symbol count -> (bits) tree -> (bits) msg
  std::vector<uint8_t> compressed_msg;
  compressed_msg.reserve(std::distance(begin, end));
  compressed_msg.emplace_back(0);    // pad bits
  compressed_msg.emplace_back(symbol_count);

  impl::huffman::bit_writer writer {std::back_inserter(compressed_msg)};

  // left is 0, right is 1
  std::stack<impl::huffman::huffman_node*> dfs_stack {{std::to_address(tree_begin)}};
  while (!dfs_stack.empty()) {
    auto* node = dfs_stack.top();
    dfs_stack.pop();

    if (!node->byte.has_value()) {
      node->left->code  <<= 1;
      node->right->code <<= 1;
      node->right->code  |= 1;
      dfs_stack.push(node->left);
      dfs_stack.push(node->right);
      writer.insert<0>();
    } else {
      codes[node->byte.value()] = node->code;
      writer.insert<1>();
      writer.insert(node->byte.value());
    }
  }

  for (auto itr = begin; itr != end; ++itr) {
    const auto code = codes[*itr].value();
    writer.insert(code);
  }

  const uint8_t pad_bits = 8 - writer.count_bits_in_buffer();
  compressed_msg[0]      = pad_bits;
  writer.flush();

  return compressed_msg;
}

}    // namespace huffman
