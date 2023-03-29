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

struct convar_data {
   const char * name;
   void * unk[8];
};
static_assert(sizeof(convar_data) == 0x48, "Invalid cs2::convar_data");

struct convar {
   void * maybevtable;
   convar_data * data;
};
static_assert(sizeof(convar) == 0x10, "Invalid cs2::convar");

} // cs2
