#pragma once

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>
#include <unity/type/collection.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(StreamingInfo, "StreamingInfo",
  (offset, NOALIGN, UInt)
  (size, NOALIGN, UInt)
  (path, NOALIGN, String)
);

}  // namespace type

}  // namespace unity
