#include "game.hpp"

#include <common/logging.hpp>
#include <common/types.hpp>
#include <common/pattern_scanner.hpp>

#include <tuple>

#include <Windows.h>
#include <Psapi.h>

namespace mem = common::mem;

static auto module_info(const char * nm) -> std::pair<u8 *, usize> {
  HMODULE rendersystem = GetModuleHandleA(nm);
  MODULEINFO mi = {};
  GetModuleInformation(GetCurrentProcess(), rendersystem, &mi, sizeof(mi));
  return std::make_pair(reinterpret_cast<u8 *>(mi.lpBaseOfDll), mi.SizeOfImage);
}

template <typename T, int sz, typename... vargs_t>
auto _deduce_T_pattern_scan(T & out, void * start, usize size, const char (&pattern)[sz], vargs_t... walkers) -> T {
  out = mem::pattern_scan<T>(start, size, pattern, '\xCC', walkers...);
  return out;
}

#define make_module_info(id, nm)                                      \
  auto [id, id_sz] = module_info(nm);                                 \
  if (!id || !id_sz) {                                                \
    cs2log(nm " not found.");                                         \
    return false;                                                     \
  }                                                                   \
  cs2log(nm " @ {} ({} bytes)", reinterpret_cast<void *>(id), id_sz)


#define w_pattern_scan(out, id, pattern, ...)                                       \
  if (!_deduce_T_pattern_scan(out, id, id_sz, pattern __VA_OPT__(,) __VA_ARGS__)) { \
    cs2log(#out "not found.");                                                      \
    return false;                                                                   \
  }                                                                                 \
  cs2log(#out " found @ {}", reinterpret_cast<void *>(out))


auto game::init() -> bool {
  cs2log("Initializing game context...");

  make_module_info(rendersystem, "rendersystemdx11.dll");

  w_pattern_scan(
      game::d3d_instance,
      rendersystem,
      "\x48\x8B\x0D\xCC\xCC\xCC\xCC\x44\x8B\xCB\x45\x8B\xC7",
      mem::rel2abs<3>()
  );

  return true;
}

auto game::uninit() -> bool {
  return true;
}

#undef w_pattern_scan
#undef make_module_info
