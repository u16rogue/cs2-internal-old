#pragma once

#include <client/cs2/structs.hpp>

#include <client/cs2/convar.hpp>
#include <client/cs2/inputservice.hpp>

#include <client/utils.hpp>
#include <common/utils.hpp>

namespace game {

inline common::utils::lazy_arrow<cs2::d3d_instance, 2> d3d_instance = nullptr;

auto init() -> bool;
auto uninit() -> bool;

} // game


namespace game::so /* shared object aka dll aka modules */ {

#define _LOAD_SO(_d, _n) inline utils::solib _d;
#include "mlists/so.lst"
#undef _LOAD_SO

} // game::so

namespace game::intf {

#define _LOAD_INTERFACE(m, t, n, id) inline t * n = nullptr;
#include "mlists/intf.lst"
#undef _LOAD_INTERFACE

} // game::intf

namespace game::concom {

using concomfn_t = void(*)(void);
#define _LOAD_CONCOMMAND_FN(x) inline concomfn_t x = nullptr;
#include "mlists/concoms.lst"
#undef _LOAD_CONCOMMAND_FN

} // game::concom
