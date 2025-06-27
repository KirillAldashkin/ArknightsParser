#pragma once

#include <unity/type/typedef.h>
#include <unity/type/array.h>
#include <unity/type/numeric.h>
#include <unity/type/resource.h>
#include <unity/type/collection.h>

namespace unity {

namespace type {

UNITY_REGULAR_TYPE(GLTextureSettings, "GLTextureSettings",
  (filter_mode, NOALIGN, Int)
  (aniso, NOALIGN, Int)
  (mip_bias, NOALIGN, Float)
  (wrap_u, NOALIGN, Int)
  (wrap_v, NOALIGN, Int)
  (wrap_w, NOALIGN, Int)
);

UNITY_REGULAR_TYPE(Texture2D, "Texture2D",
  (name, NOALIGN, String)
  (forced_fallback_format, NOALIGN, Int)
  (downscale_fallback, ALIGNED, Bool)
  (width, NOALIGN, Int)
  (height, NOALIGN, Int)
  (complete_image_size, NOALIGN, Int)
  (texture_format, NOALIGN, Int)
  (mip_count, NOALIGN, Int)
  (is_readable, ALIGNED, Bool)
  (image_count, NOALIGN, Int)
  (texture_dimension, NOALIGN, Int)
  (textute_settings, NOALIGN, GLTextureSettings)
  (lightmap_format, NOALIGN, Int)
  (colorspace, NOALIGN, Int)
  (image_data, ALIGNED, TypelessData)
  (stream_data, NOALIGN, StreamingInfo)
);

}  // namespace type

}  // namespace unity
