#include "game.hpp"
#include "client/game.hpp"
#include "client/utils.hpp"

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/mem.hpp>
#include <common/utils.hpp>

#include <tuple>

namespace mem = common::mem;

static auto init_so() -> bool {
  #define _LOAD_SO(_d, _n)                                      \
    if (game::so::_d = GetModuleHandleA(_n); !game::so::_d) {   \
      cs2log("Failed to load so " _n);                          \
      return false;                                             \
    }                                                           \
    cs2log("SO loaded {} @ {}", _n, game::so::_d.get_base<>());

  #include "mlists/so.lst"
  #undef _LOAD_SO
  return true;
}

static auto init_patterns() -> bool {
  #define _LOAD_PATTERN(_mod, _out, _pat, ...)                                        \
    if (!game::so::_mod.pattern_scan(_out, _pat, '\xCC' __VA_OPT__(,) __VA_ARGS__)) { \
      cs2log("Failed to load value by pattern: {}", #_out);                           \
      return false;                                                                   \
    }                                                                                 \
    cs2log("Found value {} by pattern @ {}", #_out, (void *)_out);

  #include "mlists/patterns.lst"
  #undef _LOAD_PATTERN
  return true;
}

static auto init_interface() -> bool {
  #define _LOAD_INTERFACE(_m, _t, _n, _id) \
  if (game::intf::_n = reinterpret_cast<_t *>(game::so::_m.create_interface(_id)); !game::intf::_n) { \
    cs2log("Failed to create interface for {}", _id);                                                 \
    return false;                                                                                     \
  }                                                                                                   \
  cs2log(_id " @ {}", (void *)game::intf::_n);

  #include "mlists/intf.lst"
  #undef _LOAD_INTERFACE
  return true;
}

static auto init_concoms() -> bool {
  #define _LOAD_CONCOMMAND_FN(x)                                               \
    cs2log("Loading concom {}...", #x);                                        \
    if (game::concom::x = utils::find_concom_callback(#x); !game::concom::x) { \
      cs2log("Failed to load concom {}", #x);                                  \
      return false;                                                            \
    }

  #include "mlists/concoms.lst"
  #undef _LOAD_CONCOMMAND_FN

  cs2log("There are {} console commands registered.", game::intf::convar->concom_count);
  return true;
}

static auto init_convars() -> bool {
  #define _LOAD_CONVAR(x)                                               \
    cs2log("Loading convar {}...", #x);                                 \
    if (game::convar::x = utils::find_convar(#x); !game::convar::x) {   \
      cs2log("Failed to load convar {}", #x);                           \
      return false;                                                     \
    }

  #include "mlists/convars.lst"
  #undef _LOAD_CONVAR

  cs2log("There are {} console variables registered.", game::intf::convar->convar_count);
  return true;
}

auto game::init() -> bool {
  cs2log("Initializing game context...");
  bool r = init_so() && init_patterns() && init_interface() && init_concoms() && init_convars();
  if (r) {
    game::client_get_entity_by_index = reinterpret_cast<decltype(game::client_get_entity_by_index)>(game::so::client.get_base<u8 *>() + 0x612290);
    cs2log("game::client_get_entity_by_index @ {}", (void *)game::client_get_entity_by_index);

    cs2log("D3D.Device:                            {}", (void *)game::d3d_instance->device);
    cs2log("D3D.DeviceContext:                     {}", (void *)game::d3d_instance->device_context); 
    cs2log("D3D.SwapChain:                         {}", (void *)game::d3d_instance->info->swapchain);
    cs2log("HWindow:                               {}", (void *)game::d3d_instance->info->window);
    cs2log("D3D.DeviceContext->OMSetRenderTargets: {}", reinterpret_cast<void ***>(game::d3d_instance->device_context)[0][33]);
  }
  return r;
}

auto game::uninit() -> bool {
  return true;
}

#undef w_load_interface
#undef w_pattern_scan
#undef make_module_info
