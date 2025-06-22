#include "unity/file/bundle.h"

#include <format>

#include <common/memory.h>

namespace unity {

namespace file {

const char* Bundle::File::name() const {
  return &name_first;
}

const Bundle::File& Bundle::File::next() const {
  const char* str = name();
  while (*str != '\0') ++str;
  return *reinterpret_cast<const File*>(++str);
}

const Bundle::Block& Bundle::block(uint32_t index) {
  return *reinterpret_cast<const Block*>(blocks_ + 10 * index);
}

const Bundle::File& Bundle::first_file() {
  return *reinterpret_cast<const File*>(files_);
}

std::optional<std::string> Bundle::UnpackData(std::size_t offset, 
                                              std::span<char> buffer) {

  // first block
  std::size_t packed_pos = 0;
  std::size_t unpacked_pos = 0;
  std::uint32_t block_idx = 0;
  const Block* cur_block = reinterpret_cast<const Block*>(blocks_);
  while ((block_idx < block_count) && (unpacked_pos + cur_block->unpacked_size <= offset)) {
    packed_pos += cur_block->packed_size;
    unpacked_pos += cur_block->unpacked_size;
    ++block_idx;
    cur_block = common::ByteOffset(cur_block, Block::size_of);
  }
  if (block_idx == block_count) return "Offset is too large";

  // unpack right suffix of the first block
  if (offset != unpacked_pos) {
    std::size_t begin_offset = offset - unpacked_pos;
    std::size_t used_size = std::min(cur_block->unpacked_size - begin_offset, buffer.size());

    std::unique_ptr<char[]> mem;
    std::span<char> mem_span;
    // don't create new buffer and reorder stuff in a given one when possible
    if (buffer.size() < cur_block->unpacked_size) {
      mem = std::make_unique<char[]>(cur_block->unpacked_size);
      mem_span = {mem.get(), cur_block->unpacked_size};
    } else {
      mem_span = buffer.subspan(0, cur_block->unpacked_size);
    }

    // unpack block
    auto unpack_err = Unpack(
      cur_block->compression(), 
      {data_ + packed_pos, cur_block->packed_size},
      mem_span
    );
    if (unpack_err) return unpack_err;

    // keep only needed part
    common::CopyTo(
      common::AsConst(mem_span.subspan(begin_offset, used_size)),
      buffer.subspan(0, used_size)
    );

    // move to the next, full, block
    buffer = buffer.subspan(used_size);
    packed_pos += cur_block->packed_size;
    unpacked_pos += cur_block->unpacked_size;
    ++block_idx;
    cur_block = common::ByteOffset(cur_block, Block::size_of);
  }

  // unpack full blocks
  while ((block_idx < block_count) && (buffer.size() >= cur_block->unpacked_size)) {
    auto unpack_err = Unpack(
      cur_block->compression(), 
      {data_ + packed_pos, cur_block->packed_size},
      buffer.subspan(0, cur_block->unpacked_size)
    );
    if (unpack_err) return unpack_err;

    // move to the next, full, block
    buffer = buffer.subspan(cur_block->unpacked_size);
    packed_pos += cur_block->packed_size;
    unpacked_pos += cur_block->unpacked_size;
    ++block_idx;
    cur_block = common::ByteOffset(cur_block, Block::size_of);
  }
  
  // unpack prefix of the last block 
  if (!buffer.empty()) {
    std::unique_ptr<char[]> mem = std::make_unique<char[]>(cur_block->unpacked_size);
    std::span<char> mem_span = {mem.get(), cur_block->unpacked_size};

    auto unpack_err = Unpack(
      cur_block->compression(), 
      {data_ + packed_pos, cur_block->packed_size},
      mem_span
    );
    if (unpack_err) return unpack_err;

    // keep only needed part
    common::CopyTo(
      common::AsConst(mem_span.subspan(0, buffer.size())),
      buffer
    );

    if (block_idx == block_count) return "Offset and/or size is too large";
  }

  return std::nullopt;
}

CompressionType Bundle::Block::compression() const {
  return static_cast<CompressionType>(flags & BlockFlags::CompressionMask);
}

}  // namespace file

}  // namespace unity