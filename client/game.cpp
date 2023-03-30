#include "game.hpp"
#include "client/game.hpp"

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

#define w_load_interface(iface, son, nm) \
  if (game::intf::iface = reinterpret_cast<decltype(game::intf::iface)>(game::so::son.create_interface(nm)); !game::intf::iface) { \
    cs2log("Failed to create interface for " nm);                                                                                  \
    return false;                                                                                                                  \
  }                                                                                                                                \
  cs2log(nm " @ {}", (void *)game::intf::iface);


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
  w_load_interface(convar, tier0, "VEngineCvar007");
  return true;
}

auto game::init() -> bool {
  cs2log("Initializing game context...");

  return init_shared_objects() && init_patterns() && init_interface();
}

auto game::uninit() -> bool {
  return true;
}

#undef w_load_interface
#undef w_pattern_scan
#undef make_module_info
