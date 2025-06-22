#pragma once

#include <cstdint>
#include <expected>
#include <utility>
#include <cstring>
#include <memory>
#include <string>
#include <optional>
#include <algorithm>
#include <span>

#include <common/any.h>
#include <common/data_reader.h>
#include <platform/endian.h>

#include <unity/flags.h>
#include <unity/unpack.h>

namespace unity {

namespace file {

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

    template<common::DataView Source>
    void Read(common::DataReader<Source>& from);
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
    char name_first;

    const char* name() const;
    const File& next() const;

    // can only be returned from bundle info memory:
    File() = delete;
    File(const File&) = delete;
    File& operator=(const File&) = delete;
    File(File&&) = delete;
    File& operator=(File&&) = delete;
  };

  template<common::DataView Source>
  static std::expected<Bundle, std::string> Read(Source&& from);

  Header header;
  std::uint32_t block_count;
  std::uint32_t file_count;

  const Block& block(uint32_t index);
  const File& first_file();

  std::optional<std::string> UnpackData(std::size_t offset, std::span<char> buffer);

 private:
  template<common::DataView Source>
  Bundle(Source&& from);

  const char* blocks_;
  const char* files_;
  const char* data_;
  std::unique_ptr<char[]> unpacked_info_;
  common::Any source_;
};  // class Bundle

template<common::DataView Source>
std::expected<Bundle, std::string> Bundle::Read(Source&& from) {
  Bundle data {std::forward<Source>(from)};
  common::DataReader<Source&> reader{data.source_.GetUnchecked<Source>()};

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
Bundle::Bundle(Source&& from) 
  : source_{common::Any::Make<Source>(std::forward<Source>(from))}
  {}

template<common::DataView Source>
void Bundle::Header::Read(common::DataReader<Source>& from) {
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

}  // namespace file

}  // namespace unity
