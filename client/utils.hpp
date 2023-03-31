#pragma once

#include <client/cs2/interface.hpp>
#include <common/types.hpp>
#include <string_view>

namespace utils {

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

  auto create_interface(std::string_view name) -> void *;
  auto create_interface_partial(std::string_view name) -> void *;

  // TODO: Implement pattern scan, cached info, and other module stuff

private:
  auto acquire_intf_root() -> bool;

private:
  void * base;
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
