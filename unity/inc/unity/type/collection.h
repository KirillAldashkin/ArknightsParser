#pragma once

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>
#include <unity/type/array.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(String, "string",
  (data, ALIGNED, Array<Char>)
);

template<Mapper First, Mapper Second>
UNITY_REGULAR_TYPE(Pair, "pair",
  (first, NOALIGN, First)
  (second, NOALIGN, Second)
);

// TODO vector
// TODO map

}  // namespace type

}  // namespace unity
