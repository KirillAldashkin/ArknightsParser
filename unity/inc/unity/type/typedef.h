#pragma once

#include <span>
#include <concepts>
#include <utility>

#include <platform/endian.h>
#include <common/data_reader.h>
#include <common/macro/common.h>

#include <unity/type.h>

namespace unity {

namespace type {

using MapReader = common::DataReader<std::span<const char>>;

template<typename This>
concept Mapper = requires(const TypeTree::Node* current,
                          const TypeTree::Node* end,
                          MapReader& raw,
                          const platform::Endian order) {
  { This::Verify(current, end) } -> std::convertible_to<bool>;
  { This(raw, order) } -> std::same_as<This>;
};

template<bool Align, Mapper T>
T Read(MapReader& raw, platform::Endian order) {
  T data { raw, order };
  if constexpr(Align) raw.AlignTo(4);
  return std::move(data);
}

#define _URT_FIELD(f_name, ...) __VA_ARGS__ f_name;
#define _URT_FIELDS(data) CONCAT(_URT_FIELDS_1 data, _END)
#define _URT_FIELDS_1(f_name, _, ...) DEFER(_URT_FIELD)(f_name, __VA_ARGS__) _URT_FIELDS_2
#define _URT_FIELDS_2(f_name, _, ...) DEFER(_URT_FIELD)(f_name, __VA_ARGS__) _URT_FIELDS_1
#define _URT_FIELDS_1_END
#define _URT_FIELDS_2_END

#define _URT_CHECKER(f_align, ...) if ((current->level != level) || (current->aligned() != f_align) || !__VA_ARGS__::Verify(current, end)) return false;
#define _URT_CHECKERS(data) CONCAT(_URT_CHECKERS_1 data, _END)
#define _URT_CHECKERS_1(_, f_align, ...) _URT_CHECKER(f_align, __VA_ARGS__) _URT_CHECKERS_2
#define _URT_CHECKERS_2(_, f_align, ...) _URT_CHECKER(f_align, __VA_ARGS__) _URT_CHECKERS_1
#define _URT_CHECKERS_1_END
#define _URT_CHECKERS_2_END

#define _URT_READER_0(f_name, f_align, ...) : f_name{::unity::type::Read<f_align, __VA_ARGS__>(raw, order)}
#define _URT_READER(f_name, f_align, ...) , f_name{::unity::type::Read<f_align, __VA_ARGS__>(raw, order)}
#define _URT_READERS(data) CONCAT(_URT_READERS_0 data, _END)
#define _URT_READERS_0(f_name, f_align, ...) DEFER(_URT_READER_0)(f_name, f_align, __VA_ARGS__) _URT_READERS_1
#define _URT_READERS_1(f_name, f_align, ...) DEFER(_URT_READER)(f_name, f_align, __VA_ARGS__) _URT_READERS_2
#define _URT_READERS_2(f_name, f_align, ...) DEFER(_URT_READER)(f_name, f_align, __VA_ARGS__) _URT_READERS_1
#define _URT_READERS_1_END
#define _URT_READERS_2_END

#define UNITY_REGULAR_TYPE(type_name, name, data) struct type_name { \
  _URT_FIELDS(data) \
  \
  static bool Verify(const ::unity::TypeTree::Node*& current, \
                     const ::unity::TypeTree::Node* end) { \
    \
    if (current >= end) return false; \
    if (current->type != (std::string_view)(name)) return false; \
    \
    auto level = current++->level + 1; \
    _URT_CHECKERS(data) \
    return true; \
  } \
  \
  type_name(::unity::type::MapReader& raw, ::platform::Endian order) \
    _URT_READERS(data) {} \
  type_name(type_name&&) = default; \
}

#define ALIGNED true
#define NOALIGN false

}  // namespace type

}  // namespace unity
