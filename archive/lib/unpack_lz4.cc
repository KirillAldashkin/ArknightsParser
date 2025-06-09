#include "archive/unpack.h"

#include <cstdlib>

#include "lz4/lz4.h"

namespace archive {

std::int64_t UnpackLZ4(std::span<const char> src, std::span<char> dst) {
  if (src.size() > INT32_MAX) return ErrorInputTooBig;
  if (dst.size() > INT32_MAX) return ErrorOutputTooBig;
  auto decomp = lz4_impl::LZ4_decompress_safe(src.data(), dst.data(), 
                                              src.size(), dst.size());
  if (decomp < 0) return decomp;
  if (decomp != dst.size()) return ErrorUnpackNotAll;
  return 0;
}

}  // namespace archive