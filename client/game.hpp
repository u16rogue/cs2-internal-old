#pragma once

#include "cs2/structs.hpp"
#include <common/utils.hpp>

namespace game {

inline common::utils::lazy_arrow<cs2::d3d_instance, 2> d3d_instance = nullptr;

auto init() -> bool;
auto uninit() -> bool;

} // game
