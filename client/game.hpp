#pragma once

#include "cs2/structs.hpp"
#include "cs2/convar.hpp"
#include "utils.hpp"
#include <common/utils.hpp>

namespace game {

inline common::utils::lazy_arrow<cs2::d3d_instance, 2> d3d_instance = nullptr;

auto init() -> bool;
auto uninit() -> bool;

} // game


namespace game::so /* shared object aka dll aka modules */ {

inline utils::solib client;
inline utils::solib engine;
inline utils::solib tier0;

} // game::so

namespace game::intf {

inline cs2::iconvar * convar = nullptr;

} // game::intf
