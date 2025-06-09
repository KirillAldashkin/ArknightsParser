#include "unity/unpack.h"

#include <cstring>
#include <format>

#include <archive/unpack.h>
#include <common/memory.h>

namespace unity {

std::optional<std::string> Unpack(CompressionType type, 
                                  std::span<const char> src, 
                                  std::span<char> dst) {

  constexpr auto err_fmt = "{} decompression of bundle info failed, returning {}";
  switch (type) {
   case CompressionType::None: {
    if (src.size() != dst.size()) {
      constexpr auto fmt = "Buffer size mismatch: Packed({}) != unpacked({})";
      return std::format(fmt, src.size(), dst.size());
    }
    common::CopyTo(src, dst);
    break;
   }
   case CompressionType::LZMA: {
    auto code = archive::UnpackLZMA(src, dst);
    if (code != 0) return std::format(err_fmt, "LZMA", code);
    break;
   }
   case CompressionType::LZ4:
   case CompressionType::LZ4HC: {
    auto code = archive::UnpackLZ4(src, dst);
    if (code != 0) return std::format(err_fmt, "LZ4", code);
    break;
   }
   case CompressionType::LZ4Inv: {
    auto code = archive::UnpackLZ4Inv(src, dst);
    if (code != 0) return std::format(err_fmt, "LZ4-inverse", code);
    break;
   }
  default:
    return std::format("Unknown compression type {}", (int)type);
  }
  return std::nullopt;
}

} // namespace unity