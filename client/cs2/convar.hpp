#pragma once

#include <common/types.hpp>
#include <client/cs2/cutl.hpp>
#include <cstddef>

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
 && offsetof(convar_data, pvalue) == 0x40
    , "Invalid cs2::convar_data");

struct __attribute__((packed)) concom_data {
  const char * name;
  const char * description;
  void * pad0[3];
  u32 pad1;
  u32 cb_id;
  void * pad2;
};
static_assert(
    sizeof(concom_data)          == 0x38
 && offsetof(concom_data, cb_id) == 0x2C
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

using convar_id_t = u64;
using concom_id_t = u64;

constexpr convar_id_t INVALID_CONVAR_ID = 0xFFFFFFFF;
constexpr concom_id_t INVALID_CONCOM_ID = 0xFFFF;

constexpr u64 CONVAR_CALLBACK_ENTRY_FLAG_RECEIVES_ARGS = 0x1;
constexpr u64 CONVAR_CALLBACK_ENTRY_FLAG_NO_ARGS       = 0x2;
constexpr u64 CONVAR_CALLBACK_ENTRY_FLAG_ONE_ARG       = 0x4;

struct convar_callback_entry_t {
    void * data;
    u64    flag;
    u64    pad0[2];
};
static_assert(sizeof(convar_callback_entry_t) == 0x20, "Invalid cs2::convar_callback_entry_t");

// 31-03-2023 use concom "cvarlist" for ref
class iconvar {
public:
  inline auto find_convar(convar_id_t * out_id, char * name, u64 justsetto0) -> void ** {
    return reinterpret_cast<void ** (***)(void *, convar_id_t *, char *, u64)>(this)[0][11](this, out_id, name, justsetto0);
  }

  inline auto iter_convar_first(convar_id_t * out_id) -> void ** {
    return reinterpret_cast<void ** (***)(void *, convar_id_t *)>(this)[0][12](this, out_id);
  }

  inline auto iter_convar_next(convar_id_t * out_id, void * current) -> void ** {
    return reinterpret_cast<void ** (***)(void *, convar_id_t *, void *)>(this)[0][13](this, out_id, current);
  }

  inline auto find_concommand(concom_id_t * out_id, const char * name) -> void ** {
    return reinterpret_cast<void ** (***)(void *, concom_id_t *, const char *)>(this)[0][15](this, out_id, name);
  }

  inline auto iter_concom_first(concom_id_t * out_id) -> void ** {
    return reinterpret_cast<void ** (***)(void *, concom_id_t *)>(this)[0][16](this, out_id);
  }

  inline auto iter_concom_next(concom_id_t * out_id, void * current) -> void ** {
    return reinterpret_cast<void ** (***)(void *, concom_id_t *, void *)>(this)[0][17](this, out_id, current);
  }

  inline auto get_cvar_from_id(convar_id_t id) -> convar_data * {
    return reinterpret_cast<convar_data *(***)(void *, convar_id_t)>(this)[0][36](this, id);
  }

  inline auto get_concom_from_id(concom_id_t id) -> concom_data * {
    return reinterpret_cast<concom_data *(***)(void *, concom_id_t)>(this)[0][39](this, id);
  }

public:
  inline auto _get_concom_callback_entry(concom_data * concom) -> convar_callback_entry_t * {
    return &callbacks[concom->cb_id];
  }

  inline auto _get_concom_callback(concom_data * concom) -> void * {
    // udiff offset = concom->cb_id << 5;
    auto & entry = callbacks[concom->cb_id];
    if (!entry.data)
      return nullptr;

    if (entry.flag & CONVAR_CALLBACK_ENTRY_FLAG_RECEIVES_ARGS) 
      return **reinterpret_cast<void ***>(entry.data);
    else if (entry.flag & CONVAR_CALLBACK_ENTRY_FLAG_NO_ARGS || entry.flag & CONVAR_CALLBACK_ENTRY_FLAG_ONE_ARG)
      return entry.data;

    return entry.data;
  }

  u8 pad0[0x118];
  convar_callback_entry_t * callbacks;
};
static_assert(offsetof(iconvar, callbacks) == 0x118, "Invalid cs2::iconvar");

} // cs2

