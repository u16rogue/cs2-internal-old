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
  u16 id;
  u16 id2;
  u32 pad2;
};
static_assert(
    sizeof(concom_data)          == 0x38
 && offsetof(concom_data, cb_id) == 0x2C
 && offsetof(concom_data, id)    == 0x30
 && offsetof(concom_data, id2)   == 0x32
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

struct convar_reg_entry_t {
  convar_data * data;
  u64           flag;
};

class iconvar {
public:
  // [01/04.2023] ~
  // NOTE: to do the the same thing with `find_convar` just do the same  thing but for a concom like `thirdperson`
  // NOTE: iteration and id lookup can be found at `tier0` when looking around `cvarlist`
  
  // NOTE: find these by looking at register convar since it first finds the convar by name to check if its already registered.
  inline auto find_convar(convar_id_t * out_id, const char * name, u64 justsetto0) -> convar_id_t * {
    return reinterpret_cast<convar_id_t * (***)(void *, convar_id_t *, const char *, u64)>(this)[0][11](this, out_id, name, justsetto0);
  }

  inline auto iter_convar_first(convar_id_t * out_id) -> convar_id_t * {
    return reinterpret_cast<convar_id_t * (***)(void *, convar_id_t *)>(this)[0][12](this, out_id);
  }

  inline auto iter_convar_next(convar_id_t * out_id, convar_id_t current) -> convar_id_t * {
    return reinterpret_cast<convar_id_t * (***)(void *, convar_id_t *, convar_id_t)>(this)[0][13](this, out_id, current);
  }

  inline auto find_concommand(concom_id_t * out_id, const char * name) -> concom_id_t * {
    return reinterpret_cast<concom_id_t * (***)(void *, concom_id_t *, const char *)>(this)[0][15](this, out_id, name);
  }

  inline auto iter_concom_first(concom_id_t * out_id) -> concom_id_t * {
    return reinterpret_cast<concom_id_t * (***)(void *, concom_id_t *)>(this)[0][16](this, out_id);
  }

  inline auto iter_concom_next(concom_id_t * out_id, concom_id_t current) -> concom_id_t * {
    return reinterpret_cast<concom_id_t * (***)(void *, concom_id_t *, concom_id_t)>(this)[0][17](this, out_id, current);
  }

  inline auto get_convar_from_id(convar_id_t id) -> convar_data * {
    return reinterpret_cast<convar_data *(***)(void *, convar_id_t)>(this)[0][36](this, id);
  }

  inline auto get_concom_from_id(concom_id_t id) -> concom_data * {
    return reinterpret_cast<concom_data *(***)(void *, concom_id_t)>(this)[0][39](this, id);
  }

public:
  u8  __pad0[0x40];
  convar_reg_entry_t * convar_entries; // [2/04/2023] Can find this by looking at iter_convar_next
  u8  __pad1[0x28];
  u32 convar_count; // 0x70
  u32 __pad2; // 0x74
  u8  __pad4[0x60]; // 0x78
  concom_data * concom_entries; // 0xD8
  u8  __pad6[0x28]; // 0xE0
  u32 concom_count; // 0x108
  u32 __pad5; // 0x10C
  u8  __pad3[0x8]; // 0x110
  convar_callback_entry_t * callbacks;
};
static_assert(
    offsetof(iconvar, convar_entries) == 0x40
 && offsetof(iconvar, convar_count)   == 0x70
 && offsetof(iconvar, concom_entries) == 0xD8
 && offsetof(iconvar, concom_count)   == 0x108
 && offsetof(iconvar, callbacks)      == 0x118,
    "Invalid cs2::iconvar");

} // cs2

