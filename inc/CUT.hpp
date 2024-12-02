#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace blocks {

template<size_t block_size>
using block = std::array<uint8_t, block_size>;

template<size_t block_size,
         typename AllocatorBlock = std::allocator<block<block_size>>>
struct cut_result {
  std::vector<block<block_size>, AllocatorBlock> blocks;
  uint32_t                                       pad_bytes;
};

template<size_t block_size,
         typename AllocatorByte  = std::allocator<uint8_t>,
         typename AllocatorBlock = std::allocator<block<block_size>>>
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

template<size_t block_size,
         typename AllocatorByte  = std::allocator<uint8_t>,
         typename AllocatorBlock = std::allocator<block<block_size>>>
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

}    // namespace blocks

namespace chunks {

template<size_t blocks_in_chunk, size_t block_size>
using chunk = std::array<blocks::block<block_size>, blocks_in_chunk>;

template<size_t blocks_in_chunk,
         size_t block_size,
         typename AllocatorChunk =
         std::allocator<chunk<blocks_in_chunk, block_size>>>
struct cut_result {
  std::vector<chunk<blocks_in_chunk, block_size>, AllocatorChunk> chunks;
  uint32_t                                                        pad_blocks;
};

template<
size_t blocks_in_chunk,
size_t block_size,
typename AllocatorBlock = std::allocator<blocks::block<block_size>>,
typename AllocatorChunk = std::allocator<chunk<blocks_in_chunk, block_size>>>
constexpr cut_result<blocks_in_chunk, block_size, AllocatorChunk> cut(
std::vector<blocks::block<block_size>, AllocatorBlock> data) {
  //todo
}

template<
size_t blocks_in_chunk,
size_t block_size,
typename AllocatorBlock = std::allocator<blocks::block<block_size>>,
typename AllocatorChunk = std::allocator<chunk<blocks_in_chunk, block_size>>>
constexpr std::vector<blocks::block<block_size>, AllocatorBlock> recombine(
  cut_result<blocks_in_chunk, block_size, AllocatorChunk> chunks) {
  //todo
}

}    // namespace chunks
