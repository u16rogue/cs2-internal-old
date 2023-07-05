#pragma once

#include <client/cs2/structs.hpp>

#include <client/cs2/convar.hpp>
#include <client/cs2/inputservice.hpp>
#include <client/cs2/entity.hpp>

#include <client/utils.hpp>
#include <common/utils.hpp>

namespace game {

inline common::utils::lazy_arrow<cs2::d3d_instance, 2> d3d_instance = nullptr;
inline cs2::entity *(*client_get_entity_by_index)(int) = nullptr; // 

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

namespace game::convar {

#define _LOAD_CONVAR(x) inline cs2::convar_data * x = nullptr;
#include "mlists/convars.lst"
#undef _LOAD_CONVAR

} // game::convar
