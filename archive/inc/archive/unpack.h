#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace archive {

constexpr std::int64_t ErrorInputTooBig  = 1000000000001;
constexpr std::int64_t ErrorOutputTooBig = 1000000000002;
constexpr std::int64_t ErrorUnpackNotAll = 1000000000003;

std::int64_t UnpackLZMA(std::span<const char> src, std::span<char> dst);
std::int64_t UnpackLZ4(std::span<const char> src, std::span<char> dst);
std::int64_t UnpackLZ4Inv(std::span<const char> src, std::span<char> dst);

}  // namespace archive