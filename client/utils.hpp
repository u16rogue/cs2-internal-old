#pragma once

#include "cs2/interface.hpp"
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
};

} // utils
