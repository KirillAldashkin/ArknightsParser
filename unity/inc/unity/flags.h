#pragma once

#include <cstdint>

#include <common/macro/flag_set.h>

namespace unity {

FLAG_SET(ArchiveFlags, std::uint32_t,
  CompressionMask = 0x3F,
  BlocksDirInfoCombined = 0x40,
  BlocksInfoFromEnd = 0x80,
  WebPluginCompatible = 0x100,
  BlockInfoWithPadding = 0x200
);

FLAG_SET(BlockFlags, std::uint16_t,
  CompressionMask = 0x3F,
  Stream = 0x40
);

enum class CompressionType : std::uint8_t {
  None = 0,
  LZMA = 1,
  LZ4 = 2,
  LZ4HC = 3,
  LZ4Inv = 4,
};

}  // namespace unity