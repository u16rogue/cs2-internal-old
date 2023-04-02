#include "game.hpp"
#include "client/game.hpp"
#include "client/utils.hpp"

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/mem.hpp>
#include <common/utils.hpp>

#include <tuple>

namespace mem = common::mem;

template <typename T, int sz, typename... vargs_t>
auto _deduce_T_pattern_scan(T & out, void * start, usize size, const char (&pattern)[sz], vargs_t... walkers) -> T {
  out = mem::pattern_scan<T>(start, size, pattern, '\xCC', walkers...);
  return out;
}

#define make_module_info(id, nm)                                      \
  auto [id, id##_sz] = common::utils::module_info(nm);                \
  if (!id || !id##_sz) {                                              \
    cs2log(nm " not found.");                                         \
    return false;                                                     \
  }                                                                   \
  cs2log(nm " @ {} ({} bytes)", reinterpret_cast<void *>(id), id##_sz)

#define w_pattern_scan(out, id, pattern, ...)                                         \
  if (!_deduce_T_pattern_scan(out, id, id##_sz, pattern __VA_OPT__(,) __VA_ARGS__)) { \
    cs2log(#out "not found.");                                                        \
    return false;                                                                     \
  }                                                                                   \
  cs2log(#out " found @ {}", reinterpret_cast<void *>(out))

static auto init_shared_objects() -> bool {
  game::so::tier0 = GetModuleHandleA("tier0.dll");
  if (!game::so::tier0) {
    cs2log("tier0.dll not found.");
    return false;
  }

  return true;
}

static auto init_patterns() -> bool {
  make_module_info(rendersystem, "rendersystemdx11.dll");
  w_pattern_scan(
      game::d3d_instance.ptr,
      rendersystem,
      "\x48\x8B\x0D\xCC\xCC\xCC\xCC\x44\x8B\xCB\x45\x8B\xC7",
      mem::rel2abs<3>()
  );

  cs2log("D3D.Device:                            {}", (void *)game::d3d_instance->device);
  cs2log("D3D.DeviceContext:                     {}", (void *)game::d3d_instance->device_context); 
  cs2log("D3D.SwapChain:                         {}", (void *)game::d3d_instance->info->swapchain);
  cs2log("HWindow:                               {}", (void *)game::d3d_instance->info->window);
  cs2log("D3D.DeviceContext->OMSetRenderTargets: {}", reinterpret_cast<void ***>(game::d3d_instance->device_context)[0][33]);

  return true;
}

static auto init_interface() -> bool {
  #define _LOAD_INTERFACE(_m, _t, _n, _id) \
  if (game::intf::_n = reinterpret_cast<_t *>(game::so::_m.create_interface(_id)); !game::intf::_n) { \
    cs2log("Failed to create interface for " _id);                                                    \
    return false;                                                                                     \
  }                                                                                                   \
  cs2log(_id " @ {}", (void *)game::intf::_n);

  #include "mlists/intf.lst"
  #undef _LOAD_INTERFACE
  return true;
}

static auto init_concoms() -> bool {
  #define _LOAD_CONCOMMAND_FN(x)                                               \
    cs2log("Loading concom " #x "...");                                        \
    if (game::concom::x = utils::find_concom_callback(#x); !game::concom::x) { \
      cs2log("Failed to load concom " #x);                                     \
      return false;                                                            \
    }

  #include "mlists/concoms.lst"
  #undef _LOAD_CONCOMMAND_FN

  cs2log("There are {} console commands and variables registered.", game::intf::convar->entries_count);
  return true;
}

auto game::init() -> bool {
  cs2log("Initializing game context...");

  return init_shared_objects() && init_patterns() && init_interface() && init_concoms();
}

auto game::uninit() -> bool {
  return true;
}

#undef w_load_interface
#undef w_pattern_scan
#undef make_module_info
