#pragma once

#include <common/types.hpp>
#include <metapp/metapp.hpp>

namespace common::mem {

namespace details {

auto pattern_scan(void * start, const usize size, const char * pattern, const usize pattern_length, const u8 mask) -> void *;

} // common::mem::details


template <int disp_offset /*displacement offset*/>
struct rel2abs {
  static auto walk(void * a) -> void * {
    u8 * adr = reinterpret_cast<u8 *>(a);
    return (adr + (disp_offset + sizeof(idiff))) + *reinterpret_cast<idiff *>(adr + disp_offset);
  }
};

template <typename T, int sz, typename... walkers_t>
auto pattern_scan(void * start, const usize size, const char (&pattern)[sz], const u8 mask, walkers_t... walkers) -> T {
  void * result = details::pattern_scan(start, size, pattern, sz - 1, mask);
  if (!result)
    return nullptr;
  ([&]() { result = walkers.walk(result); }(), ...);
  return reinterpret_cast<T>(result);
}

} // common::mem
