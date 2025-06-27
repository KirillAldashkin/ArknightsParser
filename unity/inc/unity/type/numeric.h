#pragma once

#include <cstdint>

#include <common/tstring.h>

#include <unity/type/typedef.h>

namespace unity {

namespace type {

namespace _impl {

template<auto Name, typename Read, typename Cast = Read>
struct BasicValue {
  static bool Verify(const TypeTree::Node*& current, const TypeTree::Node* end);

  BasicValue(MapReader& raw, platform::Endian order);
  operator Cast() const;

  Cast value;
};  // struct BasicValue

} // namespace _impl

using namespace common::use;

using Char = _impl::BasicValue<"char"_t, char>;
using SInt8 = _impl::BasicValue<"SInt8"_t, std::int8_t>;
using UInt8 = _impl::BasicValue<"UInt8"_t, std::uint8_t>;
using SInt16 = _impl::BasicValue<"SInt16"_t, std::int16_t>;
using UInt16 = _impl::BasicValue<"UInt16"_t, std::uint16_t>;
using Float = _impl::BasicValue<"float"_t, float>;
using Int = _impl::BasicValue<"int"_t, std::int32_t>;
using UInt = _impl::BasicValue<"unsigned int"_t, std::int32_t>;
using SInt64 = _impl::BasicValue<"SInt64"_t, std::int64_t>;
using UInt64 = _impl::BasicValue<"UInt64"_t, std::uint64_t>;
using Bool = _impl::BasicValue<"bool"_t, std::uint8_t, bool>;



template<auto Name, typename Read, typename Cast>
bool _impl::BasicValue<Name, Read, Cast>::Verify(
    const TypeTree::Node*& current, 
    const TypeTree::Node* end) {
  
  return (current < end) && (current++->type == Name.view());
}

template <auto Name, typename Read, typename Cast>
_impl::BasicValue<Name, Read, Cast>::BasicValue(MapReader& raw, platform::Endian order) 
  : value{(Cast)raw.template Read<
            platform::ByteOrdered<Read, platform::Endian::Runtime>
          >().get(order)}
  {}

template <auto Name, typename Read, typename Cast>
_impl::BasicValue<Name, Read, Cast>::operator Cast() const {
  return value;
}

}  // namespace type

}  // namespace unity