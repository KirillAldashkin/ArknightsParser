#pragma once

#include <cstdint>

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>
#include <unity/type/array.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(GUID, "GUID",
  (data, NOALIGN, FixedArray<UInt, 4>)
);

UNITY_REGULAR_TYPE(Hash128, "Hash128",
  (bytes, NOALIGN, FixedArray<UInt8, 16>)
);

}  // namespace type

}  // namespace unity
