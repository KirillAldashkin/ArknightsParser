#pragma once

#include <string>
#include <optional>
#include <span>

#include <unity/flags.h>

namespace unity {

std::optional<std::string> Unpack(CompressionType type, std::span<const char> src, std::span<char> dst);

}  // namespace unity