#pragma once

#include <cstdint>

#include "macro/flag_set.h"

namespace common {

// Access for read, write or execute some data.
FLAG_SET(RwxRights, std::uint8_t,
  Read = 1,
  Write = 2,
  Execute = 4
);

}  // namespace common