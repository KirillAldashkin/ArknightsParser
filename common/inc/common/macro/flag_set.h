#pragma once

// Defines a flag set - enum which values can be merged,
// intersected and tested for having another value.
#define FLAG_SET(name, from, ...) \
  enum class name : from { \
    __VA_ARGS__ \
  }; \
  constexpr name operator|(name l, name r) { \
    return static_cast<name>(static_cast<from>(l) | static_cast<from>(r)); \
  } \
  constexpr name operator&(name l, name r) { \
    return static_cast<name>(static_cast<from>(l) & static_cast<from>(r)); \
  } \
  constexpr name operator^=(name& l, name r) { \
    return l = static_cast<name>(static_cast<from>(l) ^ static_cast<from>(r)); \
  } \
  constexpr bool operator<<(name l, name r) { \
    return (l & r) == r; \
  }
