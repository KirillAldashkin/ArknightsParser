#pragma once

#include <cstddef>
#include <memory>
#include <array>
#include <utility>

#include <common/uninit.h>

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>

namespace unity {

namespace type {

template<Mapper T, std::size_t Count>
struct FixedArray {
  static bool Verify(const TypeTree::Node*& current, const TypeTree::Node* end);
  FixedArray(MapReader& raw, platform::Endian order);

  std::array<T, Count> data;
};  // struct FixedArray

template<Mapper T>
class Array {
 public:
  static bool Verify(const TypeTree::Node*& current, const TypeTree::Node* end);
  Array(MapReader& raw, platform::Endian order);

  Array(const Array&) = delete;
  Array& operator=(const Array&) = delete;
  Array(Array&& other) = default;
  Array& operator=(Array&& other) = default;
  ~Array();

  T* data();

  Int size;
 private:
  std::unique_ptr<common::Uninit<T>[]> data_;
};  // struct Array

struct TypelessData {
  static bool Verify(const TypeTree::Node*& current, const TypeTree::Node* end);
  TypelessData(MapReader& raw, platform::Endian order);

  Int size;
  const void* data;
};  // struct TypelessData



template<Mapper T, std::size_t Count>
bool FixedArray<T, Count>::Verify(const TypeTree::Node*& current, const TypeTree::Node* end) {
  if (current >= end) return false;
  auto level = current->level;
  for (std::size_t i = 0; i < Count; ++i) {
    if (current->level != level || current->aligned() || !T::Verify(current, end)) 
      return false;
  }
  return true;
}

namespace _impl {

template<Mapper T, std::size_t... Indices>
std::array<T, sizeof...(Indices)> MakeArray(MapReader& raw, platform::Endian order, 
                                            std::index_sequence<Indices...>) {
  return { ((void)Indices, T(raw, order))... };
}

}  // namespace _impl

template<Mapper T, std::size_t Count>
FixedArray<T, Count>::FixedArray(MapReader& raw, platform::Endian order) 
  : data{_impl::MakeArray<T>(raw, order, std::make_index_sequence<Count>())}
  {}

template<Mapper T>
bool Array<T>::Verify(const TypeTree::Node*& current, const TypeTree::Node* end) {
  if (current >= end) return false;
  if (current->type != (std::string_view)"Array") return false;
  
  auto level = current++->level + 1;
  if (current->level != level || current->aligned() || !Int::Verify(current, end)) return false;
  if (current->level != level || current->aligned() || !T::Verify(current, end)) return false;
  return true;
}

template<Mapper T>
Array<T>::Array(MapReader& raw, platform::Endian order) 
  : size{Int(raw, order)}
  , data_{std::make_unique<common::Uninit<T>[]>(size.value)}
{
  for (std::size_t i = 0; i < size; ++i)
    data_[i].Construct(raw, order);
}

template<Mapper T>
Array<T>::~Array() {
  for (std::size_t i = 0; i < size; ++i)
    data_[i].Destruct();
}

template<Mapper T>
T* Array<T>::data() {
  return reinterpret_cast<T*>(data_.get());
}

inline bool TypelessData::Verify(const TypeTree::Node*& current, const TypeTree::Node* end) {
  if (current >= end) return false;
  if (current->type != (std::string_view)"TypelessData") return false;
  
  auto level = current++->level + 1;
  if (current->level != level  || current->aligned() || !Int::Verify(current, end)) return false;
  if (current->level != level  || current->aligned() || !UInt8::Verify(current, end)) return false;
  return true;
}

inline TypelessData::TypelessData(MapReader& raw, platform::Endian order)
  : size{Int(raw, order)}
  , data{raw.template ReadArray<std::uint8_t>(size)}
  {}

}  // namespace type

}  // namespace unity
