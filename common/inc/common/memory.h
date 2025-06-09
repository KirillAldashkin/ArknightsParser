#pragma once

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <span>
#include <bit>

namespace common {

template<typename T>
T* ByteOffset(T* ptr, std::size_t val) {
  auto ret = std::bit_cast<std::size_t, T*>(ptr) + val;
  return std::bit_cast<T*, std::size_t>(ret);
}

template<typename T>
void CopyTo(std::span<const T> from, std::span<T> to) {
  assert(from.size() == to.size());
  std::copy(from.begin(), from.end(), to.begin());
}

template<typename T>
std::span<const T> AsConst(std::span<T> data) {
  return { data.data(), data.size() };
}

}  // namespace common