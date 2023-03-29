#pragma once

#include "cs2/interface.hpp"

namespace utils {

struct interface_iterator {
  interface_iterator(cs2::intfreg * first);
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

} // utils
