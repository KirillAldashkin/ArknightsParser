#pragma once

#include <cstdint>

#include <unity/type/typedef.h>
#include <unity/type/numeric.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(PolarPoint, "PolarPoint",
  (p, NOALIGN, Float)
  (r, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Vector2f, "Vector2f",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Vector3f, "Vector3f",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (z, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Vector4f, "Vector4f",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (z, NOALIGN, Float)
  (w, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Float3, "float3",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (z, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Float4, "float4",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (z, NOALIGN, Float)
  (w, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Quaternionf, "Quaternionf",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (z, NOALIGN, Float)
  (w, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Rectf, "Rectf",
  (x, NOALIGN, Float)
  (y, NOALIGN, Float)
  (width, NOALIGN, Float)
  (height, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(AABB, "AABB",
  (center, NOALIGN, Vector3f)
  (extent, NOALIGN, Vector3f)
);

UNITY_REGULAR_TYPE(Matrix3x4f, "Matrix3x4f",
  (e00, NOALIGN, Float) (e01, NOALIGN, Float) (e02, NOALIGN, Float) (e03, NOALIGN, Float)
  (e10, NOALIGN, Float) (e11, NOALIGN, Float) (e12, NOALIGN, Float) (e13, NOALIGN, Float)
  (e20, NOALIGN, Float) (e21, NOALIGN, Float) (e22, NOALIGN, Float) (e23, NOALIGN, Float)
);

UNITY_REGULAR_TYPE(Matrix4x4f, "Matrix4x4f",
  (e00, NOALIGN, Float) (e01, NOALIGN, Float) (e02, NOALIGN, Float) (e03, NOALIGN, Float)
  (e10, NOALIGN, Float) (e11, NOALIGN, Float) (e12, NOALIGN, Float) (e13, NOALIGN, Float)
  (e20, NOALIGN, Float) (e21, NOALIGN, Float) (e22, NOALIGN, Float) (e23, NOALIGN, Float)
  (e30, NOALIGN, Float) (e31, NOALIGN, Float) (e32, NOALIGN, Float) (e33, NOALIGN, Float)
);

}  // namespace type

}  // namespace unity
