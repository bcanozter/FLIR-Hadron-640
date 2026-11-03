#pragma once

#include <cstdint>
#include <vector>

struct Y16Frame {
  int width = 0;
  int height = 0;
  std::vector<uint16_t> pixels;
};

struct YU12Frame {
  int width = 0;
  int height = 0;
  std::vector<uint8_t> bytes;
};
