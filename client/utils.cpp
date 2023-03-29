#include "utils.hpp"
#include <Windows.h>
#include <common/types.hpp>

utils::solib::solib(void * base)
  : base(base) {}

auto utils::solib::acquire_intf_root() -> bool {
  if (intf_root)
    return true;

  intf_root = get_module_interface_linkedlist(base);
  return intf_root;
}

auto utils::solib::create_interface(std::string_view name) -> void * {
  if (!acquire_intf_root())
    return nullptr;

  for (auto intf : interface_iterator(intf_root)) {
    if (intf->name == name)
      return intf->create();
  }

  return nullptr;
}

utils::solib::operator bool() const {
  return base;
}

auto utils::solib::create_interface_partial(std::string_view name) -> void * {
  // TODO: implement
  if (!acquire_intf_root())
    return nullptr;

  return nullptr;
}

auto utils::get_module_interface_linkedlist(void * hmodule) -> cs2::intfreg * {
  // TODO: replace getprocaddress with something
  u8 * createintf_export = reinterpret_cast<decltype(createintf_export)>(GetProcAddress(HMODULE(hmodule), "CreateInterface"));
  if (!createintf_export) {
    return nullptr;
  }
  idiff displacement = *reinterpret_cast<idiff *>(createintf_export + 0x3);
  return *reinterpret_cast<cs2::intfreg **>((createintf_export + 0x7) + displacement);
}

auto utils::get_module_interfaces(void * hmodule) -> interface_iterator {
  return interface_iterator(get_module_interface_linkedlist(hmodule));
}

utils::interface_iterator::interface_iterator(cs2::intfreg * root)
  : current(root)
{}

auto utils::interface_iterator::begin() -> interface_iterator {
  return *this;
}

auto utils::interface_iterator::end() -> interface_iterator {
  return *this;
}

auto utils::interface_iterator::operator++() -> void {
  current = current->next;
}

auto utils::interface_iterator::operator!=(const interface_iterator & rhs) -> bool {
  return current;
}

auto utils::interface_iterator::operator*() -> cs2::intfreg * {
  return current;
}
