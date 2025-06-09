#include "archive/unpack.h"

#include <cstdlib>

#include "lzma/LzmaDec.h"

namespace archive {

static void* LzmaAlloc(lzma_impl::ISzAllocPtr, std::size_t size) {
  return malloc(size);
}
static void LzmaFree(lzma_impl::ISzAllocPtr, void* address) {
  if (address == nullptr) return;
  free(address);
}
static lzma_impl::ISzAlloc LzmaMemoryInterface { &LzmaAlloc, &LzmaFree };

std::int64_t UnpackLZMA(std::span<const char> src, std::span<char> dst) {
  std::size_t dst_size = dst.size();
  std::size_t src_size = src.size();

  lzma_impl::ELzmaStatus status;
  // TODO check ErrorUnpackNotAll
  return LzmaDecode(reinterpret_cast<lzma_impl::Byte*>(dst.data()), &dst_size, 
                    reinterpret_cast<const lzma_impl::Byte*>(src.data()) + LZMA_PROPS_SIZE, &src_size, 
                    reinterpret_cast<const lzma_impl::Byte*>(src.data()), LZMA_PROPS_SIZE, 
                    lzma_impl::ELzmaFinishMode::LZMA_FINISH_END, 
                    &status, &LzmaMemoryInterface);
}

}  // namespace archive
