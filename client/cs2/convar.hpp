#pragma once

#include <common/types.hpp>

namespace cs2 {


struct FCVAR {
  using type = u32;

  #define _CS2_FCVAR_ENTRY(x, y) static constexpr type x = y;
  #include "convar.lst"
  #undef _CS2_FCVAR_ENTRY

  auto tocstr(type id) -> const char *;
}; // FCVAR

} // cs2
