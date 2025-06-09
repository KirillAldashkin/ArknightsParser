#pragma once

#include <cstdint>
#include <expected>
#include <utility>
#include <cstring>
#include <memory>
#include <string>
#include <format>
#include <optional>
#include <algorithm>

#include <common/data_reader.h>
#include <common/memory.h>
#include <platform/endian.h>

#include <unity/flags.h>
#include <unity/unpack.h>

namespace unity {

namespace file {

template<common::DataView Source>
struct Bundle {
 public:
  struct Header {
    char* signature;
    std::uint32_t version;
    char* unity_version;
    char* unity_revision;
    std::uint64_t size;
    std::uint32_t packed_info_size;
    std::uint32_t unpacked_info_size;
    ArchiveFlags flags;

    void Read(common::DataReader<Source&>& from);
  };  // struct Header

  struct Block {
    static constexpr std::size_t size_of = 10;

    platform::u32be unpacked_size;
    platform::u32be packed_size;
    platform::ByteOrdered<BlockFlags, platform::Endian::B> flags;

    CompressionType compression() const;

    // can only be returned from bundle info memory:
    Block() = delete;
    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(Block&&) = delete;
  };  // struct Block

  struct File {
    platform::u64be offset;
    platform::u64be size;
    platform::u32be flags;
    char path_first;

    const char* path() const;
    const File& next() const;

    // can only be returned from bundle info memory:
    File() = delete;
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    File(File&&) = delete;
    File& operator=(File&&) = delete;
  };

  static std::expected<Bundle, std::string> Read(Source&& from);

  Header header;
  std::uint32_t block_count;
  std::uint32_t file_count;

  const Block& block(uint32_t index);
  const File& first_file();
  const char* data();

  std::optional<std::string> UnpackData(std::size_t offset, std::span<char> buffer);

 private:
  Bundle(Source&& from);

  const char* blocks_;
  const char* files_;
  const char* data_;
  std::unique_ptr<char[]> unpacked_info_;
  Source source_;
};  // class Bundle

template<common::DataView Source>
const char* Bundle<Source>::File::path() const {
  return &path_first;
}

template<common::DataView Source>
const Bundle<Source>::File& Bundle<Source>::File::next() const {
  const char* str = path();
  while (*str != '\0') ++str;
  return *reinterpret_cast<const File*>(++str);
}

template<common::DataView Source>
std::expected<Bundle<Source>, std::string> Bundle<Source>::Read(Source&& from) {
  Bundle data {std::forward<Source>(from)};
  common::DataReader<Source&> reader{data.source_};

  data.header.Read(reader);
  if (strcmp(data.header.signature, "UnityFS"))
    return std::unexpected("Unknown bundle signature");
  
  if (data.header.version >= 7)
    reader.AlignTo(16);

  // unpack data

  char* packed_info;
  auto src_size = data.header.packed_info_size;
  auto dst_size = data.header.unpacked_info_size;
  if (data.header.flags << ArchiveFlags::BlocksInfoFromEnd) {
    auto pos = reader.position;
    reader.position = reader.from.size() - src_size;
    packed_info = reader.template ReadArray<char>(src_size);
    reader.position = pos;
  } else if (data.header.flags << ArchiveFlags::BlocksDirInfoCombined) {
    packed_info = reader.template ReadArray<char>(src_size);    
  } else {
    return std::unexpected("Invalid block info positioning in bundle");
  }

  data.unpacked_info_ = std::make_unique<char[]>(dst_size);

  auto compress = static_cast<CompressionType>(data.header.flags & ArchiveFlags::CompressionMask);
  auto decomp_result = Unpack(compress, std::span{packed_info, src_size}, 
                                        std::span{data.unpacked_info_.get(), dst_size});
  if (decomp_result) return std::unexpected(*std::move(decomp_result));

  // read blocks

  char* info_raw = data.unpacked_info_.get() + 16;

  data.block_count = *reinterpret_cast<platform::u32be*>(info_raw);
  data.blocks_ = reinterpret_cast<const char*>(info_raw += 4);
  info_raw += data.block_count * Block::size_of;

  // read files

  data.file_count = *reinterpret_cast<platform::u32be*>(info_raw);
  data.files_ = (info_raw += 4);

  if (data.header.flags << ArchiveFlags::BlockInfoWithPadding)
    reader.AlignTo(16);

  data.data_ = reinterpret_cast<const char*>(reader.current());
  return std::move(data);
}

template<common::DataView Source>
const Bundle<Source>::Block& Bundle<Source>::block(uint32_t index) {
  return *reinterpret_cast<const Block*>(blocks_ + 10 * index);
}

template<common::DataView Source>
const Bundle<Source>::File& Bundle<Source>::first_file() {
  return *reinterpret_cast<const File*>(files_);
}

template<common::DataView Source>
const char* Bundle<Source>::data() {
  return data_;
}

template<common::DataView Source>
std::optional<std::string> Bundle<Source>::UnpackData(std::size_t offset, 
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

template<common::DataView Source>
Bundle<Source>::Bundle(Source&& from) 
  : source_{std::forward<Source>(from)}
  {}

template<common::DataView Source>
void Bundle<Source>::Header::Read(common::DataReader<Source&>& from) {
  using flags_be = platform::ByteOrdered<ArchiveFlags, platform::Endian::B>;

  signature = from.template ReadNullTerm<char>();
  version = from.template Read<platform::u32be>();
  unity_version = from.template ReadNullTerm<char>();
  unity_revision = from.template ReadNullTerm<char>();
  size = from.template Read<platform::u64be>();
  packed_info_size = from.template Read<platform::u32be>();
  unpacked_info_size = from.template Read<platform::u32be>();
  flags = from.template Read<flags_be>();
}

template<common::DataView Source>
CompressionType Bundle<Source>::Block::compression() const {
  return static_cast<CompressionType>(flags & BlockFlags::CompressionMask);
}

}  // namespace file

}  // namespace unity
