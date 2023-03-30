#pragma once

#include <common/types.hpp>
#include <client/cs2/cutl.hpp>

namespace cs2 {


struct FCVAR {
  using type = u32;

  #define _CS2_FCVAR_ENTRY(x, y) static constexpr type x = y;
  #include "fcvar_defs.lst"
  #undef _CS2_FCVAR_ENTRY

  auto tocstr(type id) -> const char *;
}; // FCVAR

struct convar_data {
  const char * name;
  void ** ppdefaultvalue;
  void * unk[6];
  void * pvalue;
  void * pdefaultvalue;
};
static_assert(
    sizeof(convar_data)           == 0x50
 /*&& offsetof(convar_data, pvalue) == 0x40*/
    , "Invalid cs2::convar_data");

struct convar_proxy {
  u64 flags;
  convar_data * data;
};
static_assert(sizeof(convar_proxy) == 0x10, "Invalid cs2::convar");

struct convar_callback_info {
  char pad0[0x438];
  cs2::list<const char *> args;
};

class iconvar {
public:
  inline auto find_convar(u64 * out_id, char * name, u64 justsetto0) -> void ** {
    return reinterpret_cast<void ** (***)(void *, u64 *, char *, u64)>(this)[0][11](this, out_id, name, justsetto0);
  }

  inline auto iter_get_first(u64 * out_id) -> void ** {
    return reinterpret_cast<void ** (***)(void *, u64 *)>(this)[0][12](this, out_id);
  }

  inline auto iter_get_next(u64 * out_id, void * current) -> void ** {
    return reinterpret_cast<void ** (***)(void *, u64 *, void *)>(this)[0][13](this, out_id, current);
  }

  inline auto find_concommand(u64 * out_id, const char * name) -> void ** {
    return reinterpret_cast<void ** (***)(void *, u64 *, const char *)>(this)[0][15](this, out_id, name);
  }

  inline auto get_cvar_from_id(u64 id) -> convar_data * {
    return reinterpret_cast<convar_data *(***)(void *, u64)>(this)[0][36](this, id);
  }
};

} // cs2
