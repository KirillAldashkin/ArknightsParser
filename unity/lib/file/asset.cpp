#include "unity/file/asset.h"

namespace unity {

namespace file {

std::span<const char> Asset::GetObject(std::uint32_t index) const {
  auto& obj = objects[index];
  return {(const char*)data + obj.offset, obj.size};
}

}  // namespace file

}  // namespace unity
