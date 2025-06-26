#pragma once

#include <cstdint>

#include <common/data_reader.h>

namespace unity {

struct Hash128 {
  std::uint8_t data[16];
};  // struct Hash128

struct TypeName {
  const char* clazz;
  const char* namezpace;
  const char* assembly;

  template<common::DataView Source>
  static TypeName Read(common::DataReader<Source>& from);
};  // struct TypeName

template<typename T>
struct Sized {
  std::uint32_t size;
  T* data;
};

template<common::DataView Source>
TypeName TypeName::Read(common::DataReader<Source>& from) {
  return {
    from.template ReadNullTerm<char>(),
    from.template ReadNullTerm<char>(),
    from.template ReadNullTerm<char>()
  };
}

}  // namespace unity