#include <common/utils.hpp>
#include <Windows.h>
#include <Psapi.h>

auto common::utils::module_info(const char * nm) -> std::pair<u8 *, usize> {
  HMODULE mod = GetModuleHandleA(nm);
  MODULEINFO mi = {};
  GetModuleInformation(GetCurrentProcess(), mod, &mi, sizeof(mi));
  return std::make_pair(reinterpret_cast<u8 *>(mi.lpBaseOfDll), mi.SizeOfImage);
}
