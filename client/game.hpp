#pragma once

#include "cs2/structs.hpp"

namespace game {

inline cs2::d3d_instance ** d3d_instance = nullptr;

auto init() -> bool;
auto uninit() -> bool;

} // game
