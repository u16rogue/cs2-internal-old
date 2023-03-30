#pragma once

#include <client/cs2/structs.hpp>
#include <client/cs2/convar.hpp>
#include <client/utils.hpp>
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
inline utils::solib rendersystem;

} // game::so

namespace game::intf {

inline cs2::iconvar * convar = nullptr;

} // game::intf
