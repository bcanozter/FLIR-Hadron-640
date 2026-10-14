#pragma once

#include <cstdint>
#include <vector>

struct RawFrame {
  int width = 0;
  int height = 0;
  std::vector<uint16_t> pixels;
};
