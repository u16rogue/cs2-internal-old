#include "utils.hpp"
#include "client/cs2/convar.hpp"
#include <Windows.h>
#include <common/types.hpp>
#include <common/logging.hpp>
#include <client/game.hpp>

auto utils::find_concom(std::string_view concom) -> cs2::concom_data * {
  cs2::concom_id_t id = 0;
  game::intf::convar->find_concommand(&id, concom.data());
  if (id == cs2::INVALID_CONCOM_ID)
    return nullptr;
  return game::intf::convar->get_concom_from_id(id);
}

auto utils::find_concom_callback(std::string_view concom) -> void(*)(void) {
  cs2::concom_data * d = find_concom(concom);
  if (!d)
    return nullptr;
  return reinterpret_cast<void(*)(void)>(game::intf::convar->_get_concom_callback(d));
}

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

auto utils::abs2relu32(void * from, usize dispoffset, void * to) -> idiff {
  u8 * f = reinterpret_cast<u8 *>(from);
  u8 * t = reinterpret_cast<u8 *>(to);
  auto ubmagic = t - (f + dispoffset + 4);
  return ubmagic;
}

auto utils::rel2abs(void * inst, usize dispoffset) -> void * {
  u8 * p = reinterpret_cast<u8 *>(inst);
  return (p + dispoffset + sizeof(idiff)) + *reinterpret_cast<idiff *>(&p[dispoffset]);
}

auto utils::is_jmp(void * fn) -> bool {
  return reinterpret_cast<u8 *>(fn)[0] == 0xE9 || *reinterpret_cast<u16 *>(fn) == *reinterpret_cast<const u16 *>("\xFF\x25");
}

static auto jmp2abs_e9(void * inst) -> void * {
  return utils::rel2abs(inst, 1);
}

static auto jmp2abs_ff25(void * inst) -> void * {
  u8 * p = reinterpret_cast<u8 *>(inst);
  return *reinterpret_cast<void **>(utils::rel2abs(inst, 2));
}

auto utils::jmp2abs(void * inst) -> void * {
  if (reinterpret_cast<u8 *>(inst)[0] == 0xE9)
    return jmp2abs_e9(inst);
  else if (*reinterpret_cast<u16 *>(inst) == *reinterpret_cast<const u16 *>("\xFF\x25"))
    return jmp2abs_ff25(inst);
  return nullptr;
}

auto utils::parse_trampoline_entry_shell(void * hooked_function) -> void * {
  u8 * p = reinterpret_cast<u8 *>(hooked_function);
  if (!is_jmp(p)) {
    cs2log("{} / Target is not hooked.", hooked_function);
    return nullptr;
  }

  u8 * tramp_to_hk = reinterpret_cast<u8 *>(rel2abs(p, 1));
  if (!is_jmp(tramp_to_hk)) {
    cs2log("{} / Assumed trampoline shell has no hook jump.", hooked_function);
    return nullptr;
  }

  u8 * jmp_to_original = tramp_to_hk - 5; 
  if (!is_jmp(jmp_to_original)) {
    cs2log("{} / Assumed trampoline shell has no original entry jump.", hooked_function);
    return nullptr;
  }

  u8 * post_original_entry = reinterpret_cast<u8 *>(rel2abs(jmp_to_original, 1));

  usize overwritten_sz = post_original_entry - p;
  // cs2log("Hook overwrote {} bytes", overwritten_sz);
  void * original_tramp = jmp_to_original - overwritten_sz;
  // cs2log("Original entry: {}", original_tramp);
  return original_tramp;
}
