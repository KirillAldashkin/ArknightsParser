#pragma once

#include <concepts>

template<std::integral T>
bool IsPow2(T value) {
  return !(value & (value - 1));
}
