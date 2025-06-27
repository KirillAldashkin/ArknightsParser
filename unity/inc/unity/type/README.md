# Where Unity types are located

## `numeric.h`:
* `SInt8`, `UInt8`
* `SInt16`, `UInt16`
* `int` (as `Int`), `unsigned int` (as `UInt`) - 32 bits
* `SInt64`, `UInt64`
* `char` (as `Char`)
* `bool` (as `Bool`)
* `float` (as `Float`)

## `array.h`:
* `FixedArray` - repeated fields, e.g. `bytes[0]`...`bytes[15]` in `Hash128`
* `Array` - of any type
* `TypelessData`

## `geometry.h`
* `Matrix3x4f`, `Matrix4x4f`
* `Vector2f`, `Vector3f`, `Vector4f`
* `float3` (as `Float3`), `float4` (as `Float4`)
* `PolarPoint`
* `Quaternionf`
* `Rectf`
* `AABB`

## `color.h`
* `ColorRGBA`

## `identify.h`
* `GUID`
* `Hash128`

## `collection.h`
* `string` (as `String`)
* `pair` (as `Pair`)

## `resource.h`
* `StreamingInfo`

## `texture.h`
* `GLTextureSettings`
* `Texture2D`
