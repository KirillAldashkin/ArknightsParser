#pragma once

#include <cstddef>
#include <concepts>

namespace common {

template<typename This>
concept DataView = requires(This& r) {
  { r.data() } -> std::convertible_to<void*>;
  { r.size() } -> std::same_as<std::size_t>;
};  // concept DataView

}  // namespace common