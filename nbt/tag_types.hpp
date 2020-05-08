#pragma once
#include <cstdint>

namespace nbt {
enum class tag_type : uint8_t {
  tag_end = 0,
  tag_byte,
  tag_short,
  tag_int,
  tag_long,
  tag_float,
  tag_double,
  tag_string = 8,
  tag_compound = 10
};
};
