#pragma once

#include <client/cs2/interface.hpp>
#include <common/types.hpp>
#include <common/mem.hpp>
#include <client/cs2/convar.hpp>
#include <string_view>
#include <metapp/metapp.hpp>

namespace utils {

auto cmd(std::string_view s) -> void;

auto find_convar(mpp::cmphstr name) -> cs2::convar_data *;
auto find_convar_str(std::string_view name) -> cs2::convar_data *;

auto find_concom(mpp::cmphstr name) -> cs2::concom_data *;
auto find_concom_str(std::string_view name) -> cs2::concom_data *;

auto get_concom_callback(cs2::concom_data * concom) -> void(*)(void);
auto find_concom_callback(mpp::cmphstr concom) -> void(*)(void);
auto find_concom_callback_str(std::string_view concom) -> void(*)(void);

struct interface_iterator {
  interface_iterator(cs2::intfreg * root);
  auto begin() -> interface_iterator;
  auto end() -> interface_iterator;
  auto operator++() -> void;
  auto operator!=(const interface_iterator & rhs) -> bool;
  auto operator*() -> cs2::intfreg *;
private:
  cs2::intfreg * current;
};
static_assert(sizeof(interface_iterator) == sizeof(void *), "Invalid utils::interface_iterator");

auto get_module_interface_linkedlist(void * hmodule) -> cs2::intfreg *;
auto get_module_interfaces(void * hmodule) -> interface_iterator;

struct solib {
  solib() = default;
  solib(void * base);

  operator bool() const;

  template <typename T = void *>
  auto get_base() -> T {
    return reinterpret_cast<T>(base);
  }

  auto get_size() -> usize;

  auto create_interface(mpp::cmphstr str) -> void *;
  auto create_interface_partial(mpp::cmphstr_partial str) -> void *;

  template <typename T, int sz, typename... walkers_t>
  auto pattern_scan(T & out, const char (&pattern)[sz], const u8 mask, walkers_t... walkers) -> bool {
    out = common::mem::pattern_scan<T>(base, size, pattern, mask, walkers...);
    return out;
  }

private:
  auto acquire_intf_root() -> bool;

private:
  void * base = nullptr;
  usize  size = -1;

  cs2::intfreg * intf_root = nullptr;
}; // solib

auto abs2relu32(void * from, usize dispoffset, void * to) -> idiff;
auto rel2abs(void * inst, usize dispoffset) -> void *;
auto is_jmp(void * fn) -> bool;
auto jmp2abs(void * inst) -> void *;
auto parse_trampoline_entry_shell(void * hooked_function) -> void *;

// [31-03-2023] Use this everytime gameoverlay or any other (including self) hooks a function and you want to call original
// if needed repeatedly use `is_jump` first then use `parse_trampoline_entry_shell` to cache the `original` then call that instead
template <typename R, typename... vargs_t>
auto safe_trampoline_to_original_call(R(*fn)(vargs_t...), vargs_t... vargs) -> R {
  if (!is_jmp(fn))
    return fn(vargs...);
  return reinterpret_cast<decltype(fn)>(parse_trampoline_entry_shell(fn))(vargs...); // [31-03-2023] null checks? nah
}

} // utils
