#pragma once

#include <cstddef>
#include <algorithm>
#include <string_view>

namespace common {

template<std::size_t Size>
struct TString {
  static_assert(Size != 0, "Can't have 0 length, must include at least \\0");

  char value[Size];

  constexpr TString(const char(&from)[Size]) {
    std::copy(from, from + Size, value);
  }

  constexpr std::string_view view() const {
    return { value, Size - 1 };
  }
};  // struct TString

namespace use {

template<TString String>
constexpr auto operator ""_t() {
  return String;
}

}  // namespace use

}  // namespace common
