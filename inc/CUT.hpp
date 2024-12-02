#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

template<size_t block_size,
         typename Allocator = std::allocator<std::array<uint8_t, block_size>>>
struct cut_result {
  std::vector<std::array<uint8_t, block_size>, Allocator> blocks;
  uint32_t                                                pad_bytes;
};

template<
size_t block_size,
typename AllocatorByte  = std::allocator<uint8_t>,
typename AllocatorBlock = std::allocator<std::array<uint8_t, block_size>>>
constexpr cut_result<block_size, AllocatorBlock> cut(
std::vector<uint8_t, AllocatorByte> data) {
  const size_t data_size   = data.size();
  const size_t pad_bytes   = data_size % block_size;
  const size_t block_count = (data_size + pad_bytes) / block_size;

  data.resize(data_size + pad_bytes, 0);
  cut_result<block_size, AllocatorBlock> result;
  result.blocks.reserve(block_count);
  result.pad_bytes = pad_bytes;

  for (auto pos = data.begin(); pos != data.end(); pos += block_size) {
    result.blocks.emplace_back([pos] {
      std::array<uint8_t, block_size> block {};
      std::copy_n(pos, block_size, block.begin());
      return block;
    }());
  }

  return result;
}

template<
size_t block_size,
typename AllocatorByte  = std::allocator<uint8_t>,
typename AllocatorBlock = std::allocator<std::array<uint8_t, block_size>>>
constexpr std::vector<uint8_t, AllocatorByte> recombine(
cut_result<block_size, AllocatorBlock> blocks) {
  const size_t pad_bytes   = blocks.pad_bytes;
  const size_t block_count = blocks.blocks.size();
  const size_t data_size   = (block_count * block_size) - pad_bytes;

  std::vector<uint8_t, AllocatorByte> data;
  data.reserve(data_size);

  for (auto block = blocks.blocks.begin(); block != blocks.blocks.end() - 1;
       ++block) {
    std::copy_n(block->begin(), block_size, std::back_inserter(data));
  }

  std::copy_n(blocks.blocks.back().begin(),
              block_size - pad_bytes,
              std::back_inserter(data));

  return data;
}
