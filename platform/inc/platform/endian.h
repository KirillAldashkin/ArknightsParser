#pragma once

#include <cstdint>
#include <algorithm>
#include <cstring>

namespace platform {

enum class Endian {
  // Little endian
  L = 0,
  // Big endian
  B = 1,
  // Native for current machine. Equals to either L or B
#ifdef BYTE_ORDER_LITTLE
  N = L,
#elifdef BYTE_ORDER_BIG
  N = B
#endif
};  // enum class Endian

template<typename T, Endian E = Endian::N>
struct ByteOrdered {
  // Whether the byte representation of current type is native to current CPU.
  static constexpr bool Native = (sizeof(T) == 1) || (E == Endian::N);

  // Byte storage of value
  alignas(T) char raw[sizeof(T)];

  // Conversion from native byte order.
  ByteOrdered(const T& data);

  // Conversion to native byte order
  operator T() const;
};  // struct ByteOrdered

using u8le = ByteOrdered<std::uint8_t, Endian::L>;
using u16le = ByteOrdered<std::uint16_t, Endian::L>;
using u32le = ByteOrdered<std::uint32_t, Endian::L>;
using u64le = ByteOrdered<std::uint64_t, Endian::L>;

using u8be = ByteOrdered<std::uint8_t, Endian::B>;
using u16be = ByteOrdered<std::uint16_t, Endian::B>;
using u32be = ByteOrdered<std::uint32_t, Endian::B>;
using u64be = ByteOrdered<std::uint64_t, Endian::B>;

using i8le = ByteOrdered<std::int8_t, Endian::L>;
using i16le = ByteOrdered<std::int16_t, Endian::L>;
using i32le = ByteOrdered<std::int32_t, Endian::L>;
using i64le = ByteOrdered<std::int64_t, Endian::L>;

using i8be = ByteOrdered<std::int8_t, Endian::B>;
using i16be = ByteOrdered<std::int16_t, Endian::B>;
using i32be = ByteOrdered<std::int32_t, Endian::B>;
using i64be = ByteOrdered<std::int64_t, Endian::B>;

template <typename T, Endian E>
ByteOrdered<T, E>::ByteOrdered(const T& data) {
  *reinterpret_cast<T*>(raw) = data;
  if constexpr(!Native) std::reverse(raw, raw + sizeof(T));
}

template<typename T, Endian E>
ByteOrdered<T, E>::operator T() const {
  alignas(T) char ret[sizeof(T)];
  *reinterpret_cast<T*>(ret) = *reinterpret_cast<const T*>(raw);
  if constexpr(!Native) std::reverse(ret, ret + sizeof(T));
  return *reinterpret_cast<T*>(ret);
}

} // namespace platform