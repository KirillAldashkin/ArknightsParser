#pragma once

#include <cstddef>
#include <cassert>
#include <concepts>

#include <common/data_view.h>
#include <common/math.h>

namespace common {

template<DataView From>
struct DataReader {
  From from;
  std::size_t position = 0;
  
  // Returns pointer to the current location.
  void* current();

  // Skips data to make 'position' a multiple
  // of 'to'; 'to' must be a power of 2.
  void AlignTo(std::size_t to);

  // Returns current reference, then advances by 'sizeof(T)'.
  template<typename T>
  T& Read();

  // Returns current pointer, then continuously advances
  // by 'sizeof(T)' until 0 is encountered.
  template<typename T>
  T* ReadNullTerm(std::size_t& size);

  // Returns current pointer, then continuously advances
  // by 'sizeof(T)' until 0 is encountered.
  template<typename T>
  T* ReadNullTerm();

  // Returns current pointer, then advances 'size' times by 'sizeof(T)' 
  template<typename T>
  T* ReadArray(std::size_t size);
};  // struct DataReader

template<DataView From>
void* DataReader<From>::current() {
  return reinterpret_cast<char*>(from.data()) + position;
}

template<DataView From>
void DataReader<From>::AlignTo(std::size_t to) {
  assert(IsPow2(to));
  std::size_t mask = to - 1;  // 0b0..10~0 => 0b0..01~1

  if (position & mask) position += to;
  position &= ~mask;
}

template<DataView From>
template<typename T>
T& DataReader<From>::Read() {
  auto ptr = reinterpret_cast<T*>(current());
  position += sizeof(T);
  return *ptr;
}

template<DataView From>
template<typename T>
T* DataReader<From>::ReadNullTerm(std::size_t& size) {
  auto ptr = reinterpret_cast<T*>(current());
  size = 0;
  while (ptr[size] != 0) ++size;
  ++size;  // also consume terminator
  position += sizeof(T) * size;
  return ptr;
}

template<DataView From>
template<typename T>
T* DataReader<From>::ReadNullTerm() {
  std::size_t ignored;
  return ReadNullTerm<T>(ignored);
}

template<DataView From>
template<typename T>
T* DataReader<From>::ReadArray(std::size_t size) {
  auto ptr = reinterpret_cast<T*>(current());
  position += sizeof(T) * size;
  return ptr;
}

}  // namespace common