#pragma once

#include <cstdint>

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(ColorRGBA, "ColorRGBA",
  (r, NOALIGN, Float)
  (g, NOALIGN, Float)
  (b, NOALIGN, Float)
  (a, NOALIGN, Float)
);

}  // namespace type

}  // namespace unity
