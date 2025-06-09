#include "archive/unpack.h"

#include <cstdlib>

#include "lz4inv/lz4.h"

namespace archive {

std::int64_t UnpackLZ4Inv(std::span<const char> src, std::span<char> dst) {
  if (src.size() > INT32_MAX) return ErrorInputTooBig;
  if (dst.size() > INT32_MAX) return ErrorOutputTooBig;
  auto decomp = lz4inv_impl::LZ4_decompress_safe(src.data(), dst.data(), 
                                                 src.size(), dst.size());
  if (decomp < 0) return decomp;
  if (decomp != dst.size()) return ErrorUnpackNotAll;
  return 0;
}

}  // namespace archive